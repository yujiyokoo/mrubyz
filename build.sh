#!/bin/sh

mrbc -B bytecode --verbose -o ./demo/add.ruby.c demo/add.rb
mrbc -B demo02_bytecode --verbose -o ./demo/demo02.ruby.c demo/demo02.rb

docker run --platform linux/amd64 --rm -i -t -v $(pwd):/src z88dk/z88dk zcc +sms -vn demo01.c mrubyz.c demo/add.ruby.c -o demo01 -create-app -DAMALLOC
docker run --platform linux/amd64 --rm -i -t -v $(pwd):/src z88dk/z88dk zcc +sms -vn demo02.c mrubyz.c demo/demo02.ruby.c -o demo02 -create-app -DAMALLOC
