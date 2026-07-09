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

Build an encoder-only library, matching LAME's old `--disable-decoder` option.
This is useful for consumers that already rely on ffmpeg or another decoder:

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

Useful build variables:

- `CC`, `AR`, `RANLIB` - toolchain programs
- `CFLAGS`, `CPPFLAGS`, `LDFLAGS` - extra compile and link flags
- `PIC=1` - add `-fPIC` for static libraries later linked into shared objects
- `DECODER=0` - omit `mpglib` decoder support
- `PREFIX`, `DESTDIR` - install location and packaging root

## WebAssembly

Build a WebAssembly SIMD static library with Emscripten:

```sh
make lib DECODER=0 CC=emcc AR=emar RANLIB=emranlib CFLAGS="-msimd128"
```

When `-msimd128` enables `__wasm_simd128__`, `libmp3lame` uses WebAssembly SIMD
intrinsics for the `init_xrpow_core` quantization helper. Native builds use SSE
intrinsics when the compiler target supports them, and otherwise fall back to C.

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

LAMER is licensed under `LGPL-2.1-or-later`. See `LICENSE` for the full license
text.

## Changes from the Original Source

This tree keeps the LAME encoder code while removing dead source paths and
tightening code that modern compilers warn about.

- Fixed VBR tag header bit packing so shifts are performed on unsigned values
  and explicitly narrowed back to bytes. The original macro shifted through
  signed integer types, which could hit implementation-defined or undefined
  behavior when constructing MPEG header bytes.
- Fixed floating-point comparisons in preset and machine helper macros by
  casting operands to `double` before calling `fabs`. That avoids accidental
  precision changes and keeps the call type consistent across C libraries.
- Corrected psychoacoustic function declarations so array parameters match the
  implementation, including the four-entry energy buffer used for left, right,
  mid, and side channels.
- Made raw IEEE float PCM conversion cast `INT_MAX` intentionally before
  scaling, instead of relying on an implicit integer-to-float conversion at the
  edge of the representable range.
- Removed stale constants and unused declarations from inactive IEEE754 and
  AIFF/MPEG paths so strict clang builds no longer depend on dead code.
- Added maintained intrinsic dispatch for native SSE and WebAssembly SIMD while
  keeping scalar C fallbacks for other targets.
- Backported the LAME 3.101 encoder fixes, including bounded flush output, ABR
  target-bit accounting, corrected SIMD quantization bounds, faster CRCs, and
  terminal-width-aware progress output. The obsolete Takehiro IEEE754
  shortcut has been removed.
