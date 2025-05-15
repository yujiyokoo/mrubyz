# mrubyz
Experimental mruby VM for Z80.

Currently it skips the headers, and can load and return integers.
It's not much of mruby yet but can run mruby bytecode.

## Build

To build the sample program (for Sega Master System) which loads an integer and returns an integer, run:

```
./build.sh
```

You should end up with `test_prog.sms` which you should be able to run on a Master System or an emulator.
