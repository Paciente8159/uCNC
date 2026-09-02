# socket.h — Backend Implementation Guide

This document specifies the contract a developer must satisfy to implement a
new TCP transport backend for the uCNC socket core, for any processor. It
defines the public (socket.h) API contract and, crucially, the constraints the
backend layer (socket_device.h) must honor.

The whole system is **allocation-free** and **cooperative, single-owner**. Read
this document fully before implementing.

---

## 1. Architecture overview

```
                uCNC owner loop
                      |
                       socket_server_dotasks()      <- the ONLY entry point
                      |
              +-------+---------------------+
              |                             |
        backend->poll()               dispatch at most one
        (emit normalized events)      application callback
              |                             |        or one RX receive
              v                             v
   events->accepted / readable / closed    socket_* callbacks (app)
```

- The public core owns static listener/connection tables and one shared RX
  scratch buffer.
- The **backend** owns all TCP protocol correctness: sequencing, retransmission,
  congestion control, checksums, connection state, native buffering.
- Application callbacks run **only** from `socket_server_dotasks()`, never from
  an ISR, another core, an RTOS task, or `backend->poll()`.

---

## 2. Threading and owner-context model

The socket core is deterministic and lock-free because every mutating call runs
in one owner context (the uCNC owner loop). This is the single most important
constraint.

- The core is **single-owner**: only one context calls
  `socket_server_poll()` / `socket_server_dotasks()`.
- The event sink (`events->accepted/readable/closed`) may be called **only**
  from inside `backend->poll()`, from the owner context.
- A backend driven asynchronously (ISR / second core / RTOS network task) must
  *not* call the event sink from that async context. Instead:
  - Push compact native events into a **bounded static queue** (or retain
    readiness state), then
  - Emit normalized events from `poll()`.
- `poll()` is never called recursively by the core. A backend must not call
  `socket_server_poll()` / `socket_server_dotasks()` from inside `poll()` or an
  event sink.
- Event functions never invoke application callbacks synchronously; they only
  update fixed core state. Callbacks are dispatched later.

Rule of thumb: everything that touches the socket core besides the single owner
loop must be funneled through a static queue consumed by `poll()`.

---

## 3. The backend interface (socket_device.h)

A backend is a struct of 6 function pointers. **All are mandatory** for
registration (socket.c rejects the device if any is NULL).

### 3.1 `init(const socket_device_events_t *events)`

- Called exactly once when the device is registered.
- `events` and every pointer inside it stay valid for the registered lifetime;
  store it in a static variable. Do not modify the table.
- Initialize all static state. **Must not emit transport events before this
  function returns.**
- Return `SOCKET_DEVICE_OK` or a negative `socket_device_result_t`. Any
  negative result fails registration.

### 3.2 `listen(const socket_device_endpoint_t *endpoint, uint8_t backlog)`

- Create, bind and start a **non-blocking** IPv4 TCP listener.
- Endpoint fields are in host byte order; address `0` (IP_ANY) means all local
  interfaces.
- `backlog` is a desired pending-connection limit. Clamp it to a native
  maximum if needed, but never treat 0 as an unbounded allocation request.
- **Every accepted client must be configured strictly non-blocking before it is
  passed to `events->accepted()`.**
- Return a stable native handle, or `SOCKET_DEVICE_INVALID_HANDLE` on any
  failure. No event is emitted for a failed listener.

### 3.3 `recv(socket_device_handle_t client, void *destination, size_t capacity)`

- One non-blocking receive attempt into `capacity` writable bytes owned by the
  core. Do not retain `destination` after returning. Data is binary, no NUL
  terminator.
- `capacity > 0`:
  - `> 0` → bytes copied, never greater than capacity.
  - `SOCKET_DEVICE_WOULD_BLOCK` → no payload currently available.
  - `SOCKET_DEVICE_CLOSED` or another negative result → closure/failure.
- `capacity == 0`: return `0` without consuming transport data.
- **On discovering closure/fatal error**: release/invalidate the client, call
  `events->closed(token, reason)` **before returning**, then return
  `SOCKET_DEVICE_CLOSED` (or the same fatal result). You need a native-handle →
  token association.

### 3.4 `send(socket_device_handle_t client, const void *source, size_t length)`

- One non-blocking send attempt of `length` immutable bytes. Do not retain
  `source`. A positive result means bytes were **copied/queued by the native
  TCP transport**, not acknowledged by the peer.
- `length > 0`:
  - `> 0` → bytes accepted, never greater than length. A partial positive
    result is valid; do not convert it to an error.
  - `SOCKET_DEVICE_WOULD_BLOCK` → zero bytes accepted temporarily.
  - `SOCKET_DEVICE_CLOSED` or negative → closure/failure.
- `length == 0`: return `0` without changing connection state.
- On discovering closure/failure: same rule as recv() — release client, emit
  `events->closed()`, return the negative result.

### 3.5 `close(socket_device_handle_t handle)`

- Locally close and release a listener or client native handle.
- Synchronous with respect to ownership: after return the handle is stale and
  all later operations on it must fail. Graceful TCP FIN may be attempted
  internally only if bounded; do not wait indefinitely for peer ACK.
- **Must not emit `events->closed()`.** The core already owns and schedules the
  local disconnection notification.
- Return `SOCKET_DEVICE_OK`; returning `SOCKET_DEVICE_INVALID` for an already
  stale handle is allowed. On any other failure still make a best effort to
  release native resources.

### 3.6 `poll(uint16_t budget)`

- One non-blocking, bounded service pass. `budget` is the max number of
  **normalized transport events** to emit during this call.
- May do smaller constant-time housekeeping beyond the budget, but must **not**
  indefinitely drain sockets, block for network activity, sleep, or wait on an
  RTOS primitive.
- Normalize and emit async/ISR/other-core events here. `accepted()`,
  `readable()`, `closed()` may be called in any order, but **no token may be
  used after `closed()`**.
- **TX readiness is deliberately not an event.** Blocking core sends poll until
  completion; nonblocking callers own their retry policy.

---

## 4. The event sink (socket_device_events_t)

Called only by the backend, only from `poll()`, only in owner context.

### 4.1 `accepted(listener_handle, client_handle) -> token`

- Register a newly accepted native TCP client. `listener` is the exact handle
  returned by `listen()`; `client` is a new usable native client handle.
- On success returns a non-zero **generation-tagged token**. Store it and use it
  for every later `readable()` / `closed()` for this client.
- Returns `SOCKET_DEVICE_INVALID_TOKEN` when no slot is free, the listener is
  stopping, or arguments are invalid. In that case you still own `client` and
  must close/release it immediately. **Do not emit `closed()` for a rejected
  client.**
- This does not transfer ownership: ownership stays with the backend until
  local `close()` or remote/fatal closure.

### 4.2 `readable(token)`

- Hint that `recv()` *may* return payload for `token`. Not a promise — `recv()`
  may still return `WOULD_BLOCK`. Duplicates/coalescing are permitted. The core
  retains the hint until `recv()` reports `WOULD_BLOCK`.
- **Retain RX bytes in the native transport until `recv()` consumes them.** Do
  not acknowledge/discard TCP payload merely because this function returned.

### 4.3 `closed(token, reason)`

- Report orderly remote close or fatal error. **Before calling this, invalidate
  the native client and release all its backend-owned resources.** The token
  becomes stale immediately and must never be emitted again.
- `reason` = `SOCKET_DEVICE_CLOSED` for orderly remote close, or a negative
  `socket_device_result_t` for a fatal failure. **Never use `SOCKET_DEVICE_OK`
  for a remote close.**
- Local `close()` must never generate this event.

---

## 5. Token and handle semantics

- `socket_device_handle_t` is a pointer-width integer. It can hold a small
  hardware socket number, a POSIX fd, a Windows SOCKET, or a native
  control-block pointer. Only the backend interprets it. Core stores it opaquely
  in `socket_device_handle_t` variables; `SOCKET_DEVICE_INVALID_HANDLE` marks
  "no handle".
- `socket_device_token_t` is opaque to the backend. It encodes a static
  connection-pool index plus a generation value. This prevents a late event for
  a closed native handle from hitting a new connection that reuses the same
  native handle. Store the token verbatim and emit it back unchanged.
- Required mapping for the backend: native handle → token. You need it to emit
  `closed()` when `recv()`/`send()` discover closure.

---

## 6. Public API contract (socket.h)

### Registration

`bool socket_register_device(socket_device_t *device)`

- All 6 function pointers mandatory. Device validated; `init()` called exactly
  once. Fails if: device NULL or incomplete, init fails, another device already
  registered, or any listener is active/stopping.

### Listeners

`socket_if_t *socket_start(uint32_t ip_listen, uint16_t port)`

- IPv4 TCP listener from the static pool. Host byte order; IP_ANY (0) = all
  interfaces. Non-blocking. NULL if: no backend, no free slot, invalid args, or
  native listen/bind fails. Returns a stable pointer.

`void socket_stop(socket_if_t *socket)`

- Stops accepting, synchronously closes the native listener, schedules local
  close of all clients. Disconnect callbacks are deferred through
  `socket_server_dotasks()` with reason `SOCKET_DEVICE_OK`. The listener stays
  in a stopping state until all pending disconnect callbacks are dispatched.
  Repeated calls harmless; NULL/already-free ignored.

### Callback registration

| Function                              | Callback type                     | NULL disables |
|---------------------------------------|-----------------------------------|---------------|
| `socket_add_ondata_handler`           | `socket_data_delegate`            | yes           |
| `socket_add_onidle_handler`           | `socket_idle_delegate`            | yes           |
| `socket_add_onconnected_handler`      | `socket_connect_delegate`         | yes           |
| `socket_add_ondisconnected_handler`   | `socket_disconnect_delegate`      | yes           |

- These only set the listener's callback field; they do nothing on a NULL or
  invalid listener.

### Protocol context (opaque to the core)

- `socket_set_protocol(socket, protocol)` stores one opaque pointer; core never
  dereferences it, passes it unchanged to every callback. Must remain valid
  until the listener is free.
- `socket_get_protocol(socket)` returns it, or NULL for a NULL listener.

### Sending

`int socket_send(socket_if_t *socket, uint8_t client_idx, const void *data,
                size_t data_len, bool noblock)`

- **No retain/copy** of caller data. `data_len` must fit in `int` (return values
  are byte counts).
- `noblock == true`: calls the nonblocking backend while immediate progress
  exists; never polls or waits. Returns bytes accepted (0 = backpressure), or a
  negative `socket_device_result_t` on failure. Caller owns any unsent suffix
  and decides when to retry.
- `noblock == false`: retries until all bytes accepted, polling the backend
  between blocked attempts (no app callbacks dispatched while waiting). Returns
  exactly `data_len` on success, a negative result on disconnect/failure. After
  `SOCKET_SEND_TIMEOUT_MS` without completion, **closes the client** and returns
  `SOCKET_DEVICE_TIMEOUT` (closing is required because a prefix may already be in
  the TCP stream).

### Closing

`int socket_close(socket_if_t *socket, uint8_t client_idx)`

- Schedules local explicit close. Native handle invalidated and closed
  synchronously; disconnect callback deferred through `socket_server_dotasks()`
  with reason `SOCKET_DEVICE_OK`. Client index unusable after success.
- Returns `SOCKET_DEVICE_OK`, `SOCKET_DEVICE_INVALID` for invalid/already-closed
  client, or a negative backend close result. Logical closure is scheduled even
  if native close reports an error.

### Cooperative pumping

`void socket_server_poll(void)`

- Pumps one bounded backend pass **without dispatching app callbacks**. Events
  are normalized into core state and stay pending until `socket_server_dotasks()`
  dispatches them. Calling from inside an app callback is allowed; recursive
  polling is suppressed.

`void socket_server_dotasks(void)`

- First calls `backend->poll(SOCKET_DEVICE_POLL_BUDGET)`, then dispatches **at
  most one** application callback or one RX receive for one round-robin client.
- Recursive calls return immediately. Alias `sockets_dotasks`.
- Call frequently from the uCNC owner loop.

Priorities inside one dispatch turn: (1) pending closure, (2) pending connect
notification, (3) one RX receive while READABLE, (4) idle callback.

### State queries

- `int socket_server_hasclients(const socket_if_t *)` — counts active/stopping
  clients of one listener (or all if NULL). Clients awaiting their disconnect
  callback stay counted until dispatched and slot released.
- `bool socket_client_is_connected(const socket_if_t *, uint8_t client_idx)` —
  true only if the client is live and sendable. Pending local/remote
  disconnection returns false.

### Initialization

- `DECL_MODULE(socket_server)` initializes all static socket core state.

---

## 7. Callback contracts (application side)

### `socket_connect_delegate(uint8_t client_idx, void *protocol)`

- Called after a client is accepted into a listener-local slot.
- `client_idx` is stable for the connection's application lifetime, in
  `[0, SOCKET_MAX_CLIENTS)`. Connection is usable during the callback;
  `socket_send()` / `socket_close()` may be called.

### `socket_data_delegate(uint8_t client_idx, const uint8_t *data, size_t
data_len, void *protocol)`

- One borrowed binary TCP payload chunk pointing into the core's **shared** RX
  buffer. Valid only during the callback. Not NUL-terminated; may contain zero
  bytes.
- **The callback must fully consume/copy/parse the chunk before returning.** The
  buffer is shared across clients and reused on the next receive.
- **Chunk boundaries are arbitrary** (TCP): one message may span many callbacks,
  several messages may be one callback. Protocols needing cross-callback bytes
  keep their own bounded, protocol-specific state.

### `socket_disconnect_delegate(uint8_t client_idx, int reason, void *protocol)`

- Called once when an app-visible client disconnects. The slot is already
  invalid; `send()`/`close()` for `client_idx` fail.
- `reason`: `SOCKET_DEVICE_CLOSED` (orderly remote close), `SOCKET_DEVICE_OK`
  (local explicit close), or another negative `socket_device_result_t` (fatal
  transport failure).

### `socket_idle_delegate(uint8_t client_idx, uint32_t idle_ms, void *protocol)`

- Cooperative, called when the client has no higher-priority event during its
  round-robin turn.
- `idle_ms`: ms since last successful TX byte receive/accept when
  `ENABLE_SOCKET_TIMEOUTS` is on; else 0. This callback never auto-closes a
  client at `SOCKET_IDLE_TIMEOUT`.

---

## 8. Memory model constraints

- **No function in the backend or core allocates memory.** Use static state,
  native stack-owned storage, or caller-provided storage only.
- One shared `SOCKET_RX_BUFFER_SIZE` scratch buffer (default 1024), reused for
  every client, every receive. Do not retain pointers into it across callbacks.
- Sizes are compile-time (`MAX_SOCKETS`, `SOCKET_MAX_CLIENTS`,
  `SOCKET_MAX_CONNECTIONS`). The core maps listener-local client indices to
  global connection slots; `SOCKET_INVALID_CLIENT_SLOT` (UINT16_MAX) marks a
  free local slot.

---

## 9. Ownership checklist for a new backend

- Implement all 6 functions and all 3 event-sink callbacks.
- Enforce owner context: call event sink only from `poll()`.
- Provide a native-handle → token mapping for closure detection.
- Make listeners and clients strictly non-blocking.
- Keep `poll()` bounded and never blocking/sleeping.
- Retain RX bytes until `recv()` consumes them; never use `SOCKET_DEVICE_OK`
  for remote close.
- Never emit `closed()` from `close()` or for a client rejected by `accepted()`.
- Never retain `destination`/`source` pointers after `recv()`/`send()` return.
- No dynamic allocation anywhere.
- Forward-ready: drain async/ISR events through a static queue consumed in
  `poll()`.

---

## 10. Key configuration macros (socket.h)

| Macro                          | Default            | Purpose                                              |
|--------------------------------|--------------------|------------------------------------------------------|
| `MAX_SOCKETS`                  | 3                  | Max simultaneous TCP listeners (1 telnet + 1 WS + 1 HTTP). |
| `SOCKET_MAX_CLIENTS`           | 2                  | Max clients addressable per listener.                |
| `SOCKET_MAX_CONNECTIONS`       | `MAX_SOCKETS * SOCKET_MAX_CLIENTS` | Total clients shared by all listeners.   |
| `SOCKET_RX_BUFFER_SIZE`        | `SOCKET_MAX_DATA_SIZE` or 1024 | Shared RX scratch buffer size.            |
| `SOCKET_MAX_DATA_SIZE`         | = `SOCKET_RX_BUFFER_SIZE` | Compatibility alias.                      |
| `SOCKET_DEVICE_POLL_BUDGET`    | 4                  | Max backend events emitted per poll pass.            |
| `SOCKET_IDLE_TIMEOUT`          | 60                 | Idle time supplied when timeouts enabled.            |
| `SOCKET_SEND_TIMEOUT_MS`       | 1000               | Max blocking-send wait before forced close.          |
| `IP_ANY`                       | 0                  | IPv4 wildcard address (host byte order).             |

Note: `SOCKET_RX_BUFFER_SIZE` is global, not per-client. Keep callbacks that
need cross-callback state inside a bounded protocol struct.