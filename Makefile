MODULES += os/services/shell

CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)

CONTIKI = ..
include $(CONTIKI)/Makefile.include
