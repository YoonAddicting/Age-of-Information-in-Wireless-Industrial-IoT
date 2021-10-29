MAKE_MAC = MAKE_MAC_TSCH
MODULES += os/services/shell

CONTIKI_PROJECT = temp-sensor
all: $(CONTIKI_PROJECT)
	

CONTIKI = ..
include $(CONTIKI)/Makefile.include
