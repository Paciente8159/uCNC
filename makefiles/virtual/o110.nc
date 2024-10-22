O101 DO
#1=[#1 + 1]
(msg, #1 < 5)
O101 WHILE[#1 LT 5]
O102 WHILE[#1 GT 0]
#1=[#1 - 1]
(msg, #1 > 5)
O102 ENDWHILE
O111 CALL
(msg, this cannot be reached)
