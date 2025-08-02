Z88DK_DIR = /media/psf/Home/Repos/parallels-ubuntu/Z80/z88dk

include $(Z88DK_DIR)/test/suites/make.config

# override make.config
FRAMEWORK_DIR = $(Z88DK_DIR)/test/framework

SOURCES += $(wildcard *.c)
SOURCES += ../mrubyz.c
OBJECTS := $(SOURCES:.c=.o) $(BYTECODE_C_FILES:.ruby.c=.rb.o)
CFLAGS +=  -DNO_LOG_RUNNING -DNO_LOG_PASSED

RUBY_FILES := $(wildcard *.rb)
BYTECODE_C_FILES := $(RUBY_FILES:.rb=.ruby.c)

all:	test.bin
.PHONY: all clean debug test

%.ruby.c: %.rb
	mrbc -B $(basename $< .rb) -o $@ $<

test.bin: $(SOURCES) $(BYTECODE_C_FILES)
	$(call compile,-lm)
	$(runtest)

clean:
	rm -f test.bin *.map $(OBJECTS) $(BYTECODE_C_FILES)

debug:
	@echo "SOURCES: $(SOURCES)"
	@echo "RUBY_FILES: $(RUBY_FILES)"
	@echo "BYTECODE_C_FILES: $(BYTECODE_C_FILES)"
	@echo "OBJECTS: $(OBJECTS)"
