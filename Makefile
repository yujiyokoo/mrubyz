# Z88DK compiler configuration
ZCC = docker run --platform linux/amd64 --rm -i -t -v $(PWD):/src z88dk/z88dk zcc
CFLAGS = +sms -vn -m -DAMALLOC
MRBC = mrbc
MISAKI_BDF = misaki_gothic.bdf
FONT8_BIN  = font8.bin

# Demo targets
DEMOS = demo01 demo02 demo03 demo04 demo05

# Ruby bytecode files
BYTECODE_FILES = demo/add.ruby.c demo/demo02.ruby.c demo/demo03.ruby.c demo/demo04.ruby.c demo/demo05.ruby.c

.PHONY: all test clean

all: $(DEMOS)

# tests
test:
	$(MAKE) -C tests

clean:
	rm -f $(DEMOS) $(DEMOS:=.sms) $(DEMOS:=.map) $(DEMOS:=.sym)
	rm -f $(BYTECODE_FILES)
	rm -f encoding_map.rb font_data.h $(FONT8_BIN)
	$(MAKE) -C tests clean

# demo programs
demo01: demo01.c mrubyz.c demo/add.ruby.c
	$(ZCC) $(CFLAGS) demo01.c mrubyz.c demo/add.ruby.c -o demo01 -create-app

demo/add.ruby.c: demo/add.rb
	$(MRBC) -B bytecode --verbose -o $@ $<

demo02: demo02.c mrubyz.c demo/demo02.ruby.c
	$(ZCC) $(CFLAGS) demo02.c mrubyz.c demo/demo02.ruby.c -o demo02 -create-app

demo/demo02.ruby.c: demo/demo02.rb
	$(MRBC) -B demo02_bytecode --verbose -o $@ $<

demo03: demo03.c mrubyz.c demo/demo03.ruby.c
	$(ZCC) $(CFLAGS) demo03.c mrubyz.c demo/demo03.ruby.c -o demo03 -create-app

demo/demo03.ruby.c: demo/demo03.rb
	$(MRBC) -B demo03_bytecode --verbose -o $@ $<

# Extract standard_font binary from z88dk Docker image (gitignored)
$(FONT8_BIN):
	docker run --rm --platform linux/amd64 z88dk/z88dk \
	  cat /opt/z88dk/libsrc/target/sms/classic/FONT8.BIN > $@

# Font artifacts — generated from presentation text + Misaki BDF + standard_font
encoding_map.rb font_data.h: demo/demo04_data.txt $(MISAKI_BDF) $(FONT8_BIN)
	ruby scripts/font_builder.rb $< $(MISAKI_BDF) $(FONT8_BIN) encoding_map.rb font_data.h

demo04: demo04.c mrubyz.c demo/demo04.ruby.c font_data.h
	$(ZCC) $(CFLAGS) demo04.c mrubyz.c demo/demo04.ruby.c -o demo04 -create-app

demo/demo04.rb: demo/demo04.src.rb demo/demo04_data.txt encoding_map.rb
	ruby scripts/converter.rb demo/demo04_data.txt > /tmp/demo04_temp.rb ; cat demo/demo04.src.rb >> /tmp/demo04_temp.rb ; mv /tmp/demo04_temp.rb demo/demo04.rb

demo/demo04.ruby.c: demo/demo04.rb
	$(MRBC) -B demo04_bytecode --verbose -o $@ $<

demo05: demo05.c mrubyz.c demo/demo05.ruby.c
	$(ZCC) $(CFLAGS) demo05.c mrubyz.c demo/demo05.ruby.c -o demo05 -create-app

demo/demo05.ruby.c: demo/demo05.rb
	$(MRBC) -B demo05_bytecode --verbose -o $@ $<
