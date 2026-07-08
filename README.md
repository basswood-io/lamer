# LAMER

LAMER is a cleaned-up source tree for the LAME MP3 encoder. It builds the
`lame` command line encoder and `libmp3lame` library with a direct maintained
Makefile.

## Build

Build the command line encoder:

```sh
make
```

Build only the static library:

```sh
make lib
```

By default, `libmp3lame/libmp3lame.a` is self-contained and includes the MP3
decoder support objects from `mpglib/`.

Build an encoder-only library, matching LAME's old `--disable-decoder` option:

```sh
make lib DECODER=0
```

Use the same `DECODER=0` setting with `make`, `make lib`, and `make install`
when installing an encoder-only build.

Install the static library and public header:

```sh
make install PREFIX=/usr/local
```

This installs `lib/libmp3lame.a` and `include/lame/lame.h`. Use `DESTDIR` for
staged packaging installs.

The install target also writes `lib/pkgconfig/mp3lame.pc` for consumers that
discover the library through pkg-config.

Override the compiler or flags when needed:

```sh
make CC=clang CFLAGS="-O2 -Wall"
make lib PIC=1
```

## Test

Run the encoder smoke test:

```sh
make test
```

This encodes `testcase.wav` to `testcase.new.mp3` and compares it with the
checked-in reference `testcase.mp3`.

## Layout

- `frontend/` - command line encoder sources
- `libmp3lame/` - MP3 encoder library sources
- `mpglib/` - MP3 decoder support used by the library
- `include/` - public headers
- `doc/` - command line man page

## License

See `LICENSE` for the project license.
