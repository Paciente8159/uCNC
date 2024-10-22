G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0010 g21 g1 x3 f20000 (msg,expression test)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0020 x [1 + 2] (msg,x should be 3)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0030 x [1 - 2] (msg,x should be -1)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0040 x [1 --3] (msg,x should be 4)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0050 x [2/5] (msg,x should be 0.40)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0060 x [3.0 * 5] (msg,x should be 15)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0070 x [0 OR 0] (msg,x should be 0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0080 x [0 OR 1] (msg,x should be 1)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0090 x [2 or 2] (msg,x should be 1)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0100 x [0 AND 0] (msg,x should be 0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0110 x [0 AND 1] (msg,x should be 0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0120 x [2 and 2] (msg,x should be 1)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0130 x [0 XOR 0] (msg,x should be 0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0140 x [0 XOR 1] (msg,x should be 1)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0150 x [2 xor 2] (msg,x should be 0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0160 x [15 MOD 4.0] (msg,x should be 3)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0170 x [1 + 2 * 3 - 4 / 5] (msg,x should be 6.2)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0180 x sin[30] (msg,x should be 0.5)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0190 x cos[0.0] (msg,x should be 1.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0200 x tan[60.0] (msg,x should be 1.7321)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0210 x sqrt[3] (msg,x should be 1.7321)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0220 x atan[1.7321]/[1.0] (msg,x should be 60.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0230 x asin[1.0] (msg,x should be 90.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0240 x acos[0.707107] (msg,x should be 45.0000)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0250 x abs[20.0] (msg,x should be 20)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0260 x abs[-1.23] (msg,x should be 1.23)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0270 x round[-0.499] (msg,x should be 0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0280 x round[-0.5001] (msg,x should be -1.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0290 x round[2.444] (msg,x should be 2)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0300 x round[9.975] (msg,x should be 10)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0310 x fix[-0.499] (msg,x should be -1.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0320 x fix[-0.5001] (msg,x should be -1.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0330 x fix[2.444] (msg,x should be 2)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0340 x fix[9.975] (msg,x should be 9)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0350 x fup[-0.499] (msg,x should be 0.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0360 x fup[-0.5001] (msg,x should be 0.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0370 x fup[2.444] (msg,x should be 3)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0380 x fup[9.975] (msg,x should be 10)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0390 x exp[2.3026] (msg,x should be 10)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0400 x ln[10.0] (msg,x should be 2.3026)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0410 x [2 ** 3.0] #1=2.0 (msg,x should be 8.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0420 ##1 = 0.375 (msg,#1 is 2, so parameter 2 is set to 0.375)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0430 x #2 (msg,x should be 0.375) #3=7.0
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0440 #3=5.0 x #3 (msg,parameters set in parallel, so x should be 7, not 5)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0450 x #3 #3=1.1 (msg,parameters set in parallel, so x should be 5, not 1.1)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0460 x [2 + asin[1/2.1+-0.345] / [atan[fix[4.4] * 2.1 * sqrt[16.8]] /[-18]]**2]
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0470 x sqrt[3**2 + 4**2] (msg,x should be 5.0)
G4P0
(msg, x:#5061, y:#5062, z:#5063)
n0480 m2