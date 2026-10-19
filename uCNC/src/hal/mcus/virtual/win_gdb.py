Import("env")
import glob
import os
import re
import shutil
import struct
import subprocess
from os.path import abspath, basename, dirname, isdir, isfile, join, normpath

#
# Debug client (GDB) resolution for the Windows virtual MCU.
#
# PlatformIO takes the debugger of the VSCode debug session from `$GDB`
# (`platformio/builder/tools/piointegration.py` exports it as `gdb_path` in
# `idedata.json`, `platformio/debug/config/base.py` uses it as
# `client_executable_path`).  The `native` platform leaves `$GDB` empty, so
# PlatformIO falls back to a bare `gdb` looked up in PATH.  On Windows that is
# the gdb bundled with `toolchain-gccmingw32`, and that build is unusable:
# `gdb.exe` is only a launcher for `gdborig.exe`, and `gdborig.exe` imports
# `libpython2.7.dll`, which the package does not ship.  The Windows loader
# kills it with STATUS_DLL_NOT_FOUND (exit code 53) before it can even print
# its banner, so the VSCode session starts "successfully" but the target is
# never started, never stops and can not be single stepped.
#
# This script exports a debugger that actually runs:
#   * `custom_win_gdb` (platformio.ini) or the `WIN_GDB` environment variable
#     may point at a specific gdb,
#   * otherwise PATH and a list of well known MinGW/MSYS2 installations are
#     probed with `gdb --version`,
#   * a gdb that can not start because a private DLL is missing (Dev-Cpp's
#     `gdb32.exe` needs `zlib1.dll` for example) is repaired by copying the
#     executable next to the DLLs it imports into `.pio/gdb-shim`; a missing
#     `zlib1.dll` is rebuilt there from the static `libz.a` of the native
#     toolchain, which is the only private dependency the toolchain can supply.
#
# 32 bit debuggers are preferred: the toolchain compiles an i686 program and
# an *old* 64 bit gdb mishandles the WOW64 breakpoint used to stop it
# ("unknown target exception 0x4000001f"), which makes single stepping
# impossible.

_I386 = 0x014C
_X64 = 0x8664
_NO_WINDOW = 0x08000000  # subprocess.CREATE_NO_WINDOW

#: Minimum version accepted from a 64 bit gdb (see the WOW64 note above).
#: Conservative: gdb 7.8.1 was verified to fail on the 32 bit emulator while
#: gdb 15.2 was verified to work. A 32 bit gdb is always preferred.
_X64_MIN_MAJOR = 8

#: Directories (relative to a drive root or to a "Program Files" directory)
#: that hold MinGW/MSYS2 binaries, both for finding a debugger and for finding
#: the private DLLs a debugger needs. Applications that bundle a MinGW
#: toolchain (Inkscape, Dev-C++, ...) are listed as well.
_INSTALL_PATTERNS = (
    "Dev-Cpp/*/bin",
    "msys*/mingw32/bin",
    "msys*/mingw64/bin",
    "mingw32/bin",
    "mingw64/bin",
    "TDM-GCC-32/bin",
    "TDM-GCC-64/bin",
    "w64devkit/bin",
    "Git/mingw32/bin",
    "Git/mingw64/bin",
    "Git/usr/bin",
    "inkscape/bin",
    "ProgramData/chocolatey/bin",
)


def _pe_machine(path):
    """Return the machine (architecture) field of a PE file, or None."""
    try:
        with open(path, "rb") as fp:
            data = fp.read()
    except OSError:
        return None
    if len(data) < 0x40 or data[:2] != b"MZ":
        return None
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if pe + 6 > len(data) or data[pe : pe + 4] != b"PE\0\0":
        return None
    return struct.unpack_from("<H", data, pe + 4)[0]


def _pe_imports(path):
    """Return the names of the DLLs a PE file imports."""
    try:
        with open(path, "rb") as fp:
            data = fp.read()
    except OSError:
        return []
    if len(data) < 0x40 or data[:2] != b"MZ":
        return []
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if pe + 24 > len(data) or data[pe : pe + 4] != b"PE\0\0":
        return []

    coff = pe + 4
    nsections = struct.unpack_from("<H", data, coff + 2)[0]
    opt_size = struct.unpack_from("<H", data, coff + 16)[0]
    opt = coff + 20
    magic = struct.unpack_from("<H", data, opt)[0]
    if magic == 0x10B:  # PE32
        datadir = opt + 96
    elif magic == 0x20B:  # PE32+
        datadir = opt + 112
    else:
        return []

    sections = []
    for index in range(nsections):
        header = opt + opt_size + index * 40
        vsize, vaddr, rsize, raw = struct.unpack_from("<IIII", data, header + 8)
        sections.append((vaddr, max(vsize, rsize), raw))

    def to_offset(rva):
        for vaddr, size, raw in sections:
            if vaddr <= rva < vaddr + size:
                return raw + (rva - vaddr)
        return None

    names = []
    descriptor = to_offset(struct.unpack_from("<I", data, datadir + 8)[0])
    while descriptor is not None:
        name_rva = struct.unpack_from("<I", data, descriptor + 12)[0]
        if not name_rva:
            break
        name_offset = to_offset(name_rva)
        if name_offset is None:
            break
        end = data.find(b"\0", name_offset)
        names.append(data[name_offset:end].decode("ascii", "replace"))
        descriptor += 20
    return names


def _is_system_dll(path):
    """True for DLLs shipped by Windows (never copied into the shim)."""
    windir = normpath(os.environ.get("WINDIR", r"C:\Windows")).lower()
    return normpath(path).lower().startswith(windir + os.sep)


def _roots():
    """Drive roots and their "Program Files" directories."""
    roots = []
    for letter in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        drive = "%s:\\" % letter
        if not isdir(drive):
            continue
        roots.append(drive)
        for program_files in ("Program Files", "Program Files (x86)"):
            directory = join(drive, program_files)
            if isdir(directory):
                roots.append(directory)
    return roots


def _search_dirs(extra=None):
    """Directories searched for a debugger and for its private DLLs."""
    dirs = []
    for directory in list(extra or []) + os.environ.get("PATH", "").split(os.pathsep):
        if directory and isdir(directory):
            dirs.append(normpath(directory))
    for root in _roots():
        for pattern in _INSTALL_PATTERNS:
            dirs.extend(glob.glob(join(root, *pattern.split("/"))))
    seen, result = set(), []
    for directory in dirs:
        key = normpath(directory).lower()
        if key not in seen:
            seen.add(key)
            result.append(directory)
    return result


def _gdb_version(path):
    """Return the (major, minor) version of a gdb, or None if it can not run."""
    try:
        proc = subprocess.run(
            [path, "--version"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=60,
            creationflags=_NO_WINDOW,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    match = re.search(r"GNU gdb[^\n]*?(\d+)\.(\d+)", proc.stdout.decode("ascii", "replace"))
    return (int(match.group(1)), int(match.group(2))) if match else None


def _find_dll(name, machine, dirs):
    """Locate a DLL import; returns a path, the string "system" or None."""
    for directory in dirs:
        candidate = join(directory, name)
        if not isfile(candidate):
            continue
        if _is_system_dll(candidate):
            return "system"
        if _pe_machine(candidate) == machine:
            return candidate
    return None


#: The only private DLL the native toolchain can supply: its GCC is built against
#: the static zlib it ships as `libz.a`. A gdb built against zlib (Dev-Cpp's 32
#: bit `gdb32.exe` is) will not even load without `zlib1.dll`, and that DLL is
#: missing from most machines, so rebuild it instead of leaving gdb unusable.
def _rebuild_dll(name, machine, shim_dir):
    """Rebuild a private DLL from the native toolchain, or return None."""
    if name.lower() != "zlib1.dll":
        return None
    package = env.PioPlatform().get_package_dir("toolchain-gccmingw32")
    if not package:
        return None
    compiler = join(package, "bin", "i686-w64-mingw32-gcc.exe")
    library = join(package, "i686-w64-mingw32", "lib", "libz.a")
    if not (isfile(compiler) and isfile(library)):
        return None
    if not isdir(shim_dir):
        os.makedirs(shim_dir)
    output = join(shim_dir, name)
    if isfile(output) and _pe_machine(output) == machine:
        return output
    try:
        subprocess.run(
            [
                compiler,
                "-shared",
                "-o",
                output,
                "-Wl,--export-all-symbols",
                "-Wl,--whole-archive",
                library,
                "-Wl,--no-whole-archive",
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=120,
            creationflags=_NO_WINDOW,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if isfile(output) and _pe_machine(output) == machine:
        return output
    return None


def _copy(source, destination):
    if isfile(destination) and os.path.getsize(destination) == os.path.getsize(source):
        return
    shutil.copy2(source, destination)


def _build_shim(path, machine):
    """Copy a gdb next to the private DLLs it imports, or return None."""
    gdb_dir = dirname(path)
    dirs = _search_dirs([gdb_dir, dirname(gdb_dir), join(dirname(gdb_dir), "bin")])
    shim_dir = join(env.subst("$PROJECT_DIR"), ".pio", "gdb-shim")
    dlls = {}
    for name in _pe_imports(path):
        if isfile(join(gdb_dir, name)):
            continue
        found = _find_dll(name, machine, dirs)
        if found is None:
            found = _rebuild_dll(name, machine, shim_dir)
        if found is None:
            # a private DLL we can not provide: a shim would not help
            return None
        if found != "system":
            dlls[name] = found
    if not dlls:
        return None

    if not isdir(shim_dir):
        os.makedirs(shim_dir)
    # gdb.exe is a launcher for gdborig.exe on some MinGW builds
    for sibling in glob.glob(join(gdb_dir, "gdborig*.exe")):
        _copy(sibling, join(shim_dir, basename(sibling)))
    _copy(path, join(shim_dir, basename(path)))
    for name, source in dlls.items():
        _copy(source, join(shim_dir, name))
    return join(shim_dir, basename(path))


def _candidates():
    """Debugger candidates, in the order they should be tried."""
    paths = []

    def add(path):
        if path and isfile(path):
            path = abspath(path)
            if path not in paths:
                paths.append(path)

    add(env.GetProjectOption("custom_win_gdb", None) or os.environ.get("WIN_GDB"))
    toolchain_dir = env.PioPlatform().get_package_dir("toolchain-gccmingw32")
    if toolchain_dir:
        add(join(toolchain_dir, "bin", "gdb.exe"))
    for directory in os.environ.get("PATH", "").split(os.pathsep):
        add(join(directory, "gdb32.exe"))
        add(join(directory, "gdb.exe"))
    for directory in _search_dirs():
        for name in ("gdb32.exe", "gdb.exe"):
            add(join(directory, name))
    return paths


def _resolve():
    """Return (gdb path, gdb version) of a usable debugger, or (None, None)."""
    candidates = {path: _pe_machine(path) for path in _candidates()}
    # 32 bit debuggers first: they are the safe choice for an i686 program
    order = [_I386, _X64, None]
    for machine in order:
        for path, path_machine in candidates.items():
            if path_machine != machine:
                continue
            version = _gdb_version(path)
            if version is None:
                shim = _build_shim(path, path_machine) if path_machine else None
                if shim:
                    version = _gdb_version(shim)
                    path = shim
            if version is None:
                continue
            if machine == _X64 and version[0] < _X64_MIN_MAJOR:
                print(
                    "Warning: skipping 64 bit gdb %s (%d.%d) - too old to debug "
                    "the 32 bit emulator" % ((path,) + version)
                )
                continue
            return path, version
    return None, None


_gdb, _version = _resolve()
if _gdb:
    env.Replace(GDB=_gdb)
    print(
        "Debugger for %s: %s (gdb %d.%d)"
        % (env["PIOENV"], _gdb, _version[0], _version[1])
    )
else:
    print(
        "Warning: no usable gdb found for %s - VSCode debugging will not work. "
        "Install a MinGW/MSYS2 gdb (32 bit preferred) or set `custom_win_gdb` "
        "in platformio.ini." % env["PIOENV"]
    )
