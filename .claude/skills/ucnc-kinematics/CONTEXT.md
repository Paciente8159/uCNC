# µCNC kinematics vocabulary

Use these terms for kinematics documentation and discussion. Current declarations,
wrapper behavior, and selected-backend behavior remain authoritative.

## Coordinate spaces

**World coordinate** — The machine-space X/Y/Z/A/B/C position used by motion planning
and G-code semantics.

**Actuator position** — The integer step count for each physical actuator. Its mapping
to world coordinates may be direct, coupled, nonlinear, or redundant.

**Forward kinematics** — Converts actuator steps into world coordinates.

**Inverse kinematics** — Converts a world-coordinate target into actuator steps.

**Kinematic transform** — An optional backend mapping around the core conversions.
The shared wrappers apply it before inverse conversion and reverse it after forward
conversion. Skew compensation is also handled by those wrappers.

**Reachable workspace** — Coordinates the mechanism can physically attain, which may
be narrower or differently shaped than configured Cartesian soft limits.

## Motion and boundaries

**Segmented motion** — Subdividing a Cartesian line into smaller linear moves so a
nonlinear actuator path remains within the required accuracy. Segment size trades path
error against planner work.

**Boundary policy** — The backend's treatment of targets outside its valid workspace.
It includes soft-limit, homing, and optional jog-clamping behavior; the target may be
mutable in selected paths.

## Homing and configuration

**Homing reference** — The kinematic-specific coordinate established after limit
seeking and any pull-off. It must agree across the interpolator, motion controller,
parser, and physical machine.

**Kinematic backend** — The selected header and implementation providing the
kinematics interface for one machine mechanism.

**Compile-time dispatch** — Selection through `KINEMATIC`, its identifier, and the
include branch in `kinematicdefs.h`. Inspect the current headers for the active names
and configuration rules.

**Kinematic settings macros** — Header macros that extend global settings defaults and
the `$` settings registry. Inspect their expansion sites before adding or changing a
field or setting ID.
