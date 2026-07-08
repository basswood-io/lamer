# LAMER

LAMER is a cleaned-up source tree for the LAME MP3 encoder. It builds the
`lame` command line encoder and `libmp3lame` library from the bundled autotools
build files.

## Build

The configured tree can be built directly:

```sh
make
```

To configure from scratch, use:

```sh
./configure
make
```

The simplified Unix makefile is also available:

```sh
make -f Makefile.unix
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
- `Dll/`, `ACM/`, `dshow/`, `vc_solution/` - Windows-related build targets

## License

See `LICENSE` for the project license.
