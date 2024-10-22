O101 DO
#1=[#1 + 1]
(msg, #1 < 5)
O101 WHILE[#1 LT 5]
O102 WHILE[#1 GT 0]
#1=[#1 - 1]
O103 IF [#1 EQ 2]
O111 CALL
O110 RETURN
O103 ENDIF
(msg, #1 > 5)
O102 ENDWHILE
O110 RETURN
(msg, this cannot be reached)
