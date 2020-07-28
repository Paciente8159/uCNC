# Project: µCNC

MAKE 	 = make

.PHONY: avr

avr:
	cd ./uCNC/mcus/avr/ && $(MAKE) all
