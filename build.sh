#!/bin/sh

mrbc -B bytecode --verbose -o ./add.ruby.c tests/ruby/add.rb

docker run --platform linux/amd64 --rm -i -t -v $(pwd):/src z88dk/z88dk zcc +sms -vn test_prog.c mrubyz.c add.ruby.c -o test_prog -create-app
