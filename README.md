# mrubyz
Experimental mruby VM for Z80.

Currently it skips the headers, and can load and return integers.
It's not much of mruby yet but can run mruby bytecode.

The current focus is Sega Master System but there is no reason why it should not run on other Z80-based platforms.

Currently it is capable of running a very limited set of Ruby instructions, such as:

```
return 1 + 0
```

If you compile this into mruby bytecode, then load it, you it will evaluate to the value `1` (as expected).

Here is a screenshot of it running on a Sega Master System emulator.

![screenshot of return 1 + 0](https://github.com/yujiyokoo/mrubyz/blob/main/docs/images/screenshot.png?raw=true)

## Build

To build the sample program (for Sega Master System) which loads an integer and returns an integer, run:

```
./build.sh
```

You should end up with `test_prog.sms` which you should be able to run on a Master System or an emulator.
