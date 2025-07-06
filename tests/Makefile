Z88DK_DIR = /media/psf/Home/Repos/parallels-ubuntu/Z80/z88dk

include $(Z88DK_DIR)/test/suites/make.config

# override make.config
FRAMEWORK_DIR = $(Z88DK_DIR)/test/framework

SOURCES += $(wildcard *.c)
SOURCES += ../mrubyz.c ../add.c
OBJECTS := $(SOURCES:.c=.o)
CFLAGS +=  -DNO_LOG_RUNNING -DNO_LOG_PASSED

all:	test.bin

test.bin: $(SOURCES)
	$(call compile,-lm)
	$(runtest)

clean:
	rm -f test.bin *.map $(OBJECTS)
