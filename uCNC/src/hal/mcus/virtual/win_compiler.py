Import("env")
from os.path import join, isdir

#
# The `native` platform compiles with the host `gcc`, which is not present on
# Windows. Put the bundled MinGW toolchain's bin dir first on the build PATH so
# `gcc`, `g++`, `ar`, etc. resolve without requiring a system-wide install.
#
_toolchain_dir = env.PioPlatform().get_package_dir("toolchain-gccmingw32")
if _toolchain_dir:
    _toolchain_bin = join(_toolchain_dir, "bin")
    if isdir(_toolchain_bin):
        env.PrependENVPath("PATH", _toolchain_bin)

#
# Dump build environment (for debug)
# print(env.Dump())
#

env.Append(
  LINKFLAGS=[
      "-static",
      "-g3",
      # "-lSDL2main",
      # "-lSDL2",
      "-lws2_32"
  ]
)
