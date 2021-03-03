# Arnold CPC emulator

## Building

### macOS

To build on macOS, install the dependencies using [Homebrew](https://brew.sh).

```shell
brew install cmake wxwidgets
```

Download the latest SDL2 Runtime Binaries from https://www.libsdl.org/download-2.0.php
and copy the `SDL2.framework` folder it contains to `/Library/Frameworks` using `sudo`.
Arnold is know to compile with SDL2 2.0.14.

Then we can build the app:

```shell
cd src
./make_mac.sh
```

If there are SDL2-related issues, check whether you have an older version of SDL installed
in `/Library/Framework/SDL2.framework`. Download a new version and replace it as above.
