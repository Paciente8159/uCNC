Import("env")

#
# Dump build environment (for debug)
# print(env.Dump())
#

env.Append(
  LINKFLAGS=[
      "-static-libgcc",
      "-g3",
	  "-lws2_32"
  ]
)
