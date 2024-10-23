(msg, test if condition)
G90G21G17G0X0Y0Z0

O101 IF [#1 EQ 1]
(msg, test repeat X should go to 10)
G91
O102 REPEAT [10]
G0X1
O102 ENDREPEAT

O101 ELSEIF [#1 EQ 2]
(msg, test repeat with continue X should go to 0)
G91
O102 REPEAT [10]
O102 CONTINUE
G0X1
O102 ENDREPEAT

O101 ELSEIF [#1 EQ 3]
(msg, test repeat with break X should go to 1)
G91
O102 REPEAT [10]
G0X1
O102 BREAK
(msg, unreachable)
O102 ENDREPEAT

O101 ELSEIF [#1 EQ 4]
(msg, test while X should go to 10)
G91
O102 WHILE [#1 GT 0]
G0X#1 #1=[#1 - 1]
O102 ENDWHILE

O101 ELSEIF [#1 EQ 5]
(msg, test while with continue X should go to 15)
G91
O102 WHILE [#1 GT 0]
G0X#1 #1=[#1 - 1]
O102 ENDWHILE

O101 ELSEIF [#1 EQ 6]
(msg, test while with continue X should go to 6)
G91
O102 DO
G0X#1 #1=[#1 + 1]
O102 WHILE[#1 LT 6]

O101 ELSEIF [#1 EQ 7]
(msg, test while with continue X should go to 15)
G91
O102 DO
G0X#1 #1=[#1 + 1]
O102 WHILE[#1 LE 8]

O101 ELSEIF [#1 EQ 10]
(msg, should go to 10)
G91
G0X10
O100 RETURN
G0X10

O101 ELSE
(msg, do O call)
O110 CALL [#1]
O101 ENDIF
