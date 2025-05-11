#!/bin/sh

docker run --rm -i -t -v $(pwd):/src z88dk/z88dk make
