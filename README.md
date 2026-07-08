# LAMER

LAMER is a cleaned-up source tree for the LAME MP3 encoder. It builds the
`lame` command line encoder and `libmp3lame` library with a direct maintained
Makefile.

## Build

Build the command line encoder:

```sh
make
```

Override the compiler or flags when needed:

```sh
make CC=clang CFLAGS="-O2 -Wall"
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
- `doc/` - bundled documentation and man page
- `Dll/`, `ACM/` - legacy Windows-related sources

## License

See `LICENSE` for the project license.
