O101 IF [#1 EQ 1]
(msg, if #1 equal to 1)
O101 ELSEIF [#1 EQ 2]
(msg, if #1 equal to 2)
O101 ELSE
(msg, else #1 is not 1 or 2)
O101 ENDIF
(msg, repeat loop)
O102 REPEAT [2]
#1=[#1 + 1]
(msg, #1)
O102 ENDREPEAT
O110 CALL [#1]
