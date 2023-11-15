Import("env")

#
# Dump build environment (for debug)
# print(env.Dump())
#

env.Append(
  LINKFLAGS=[
      "-static-libgcc",
      "-g3",
      # "-lSDL2main",
      # "-lSDL2",
      "-lws2_32"
  ]
)
