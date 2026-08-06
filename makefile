TARGET   = 07pwmSoftware_Efectos
MCU      = -mmcs51 --std-c11
PROTOCOL = stc8g
BAUD     = 115200

.PHONY: all flash clean

all: $(TARGET).ihx

$(TARGET).ihx: $(TARGET).c stc8h.h
	sdcc $(MCU) -o $@ $<

flash: $(TARGET).ihx
	stcgal -P $(PROTOCOL) -p /dev/ttyUSB0 -b $(BAUD) $<

clean:
	rm -f *.ihx *.hex *.lst *.asm *.sym *.map
