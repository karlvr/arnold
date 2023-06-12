# Arnold CPC emulator

Arnold is a CPC emulator written and maintained by [Kevin Thacker](https://www.cpcwiki.eu/index.php/Kevin_Thacker).

https://www.cpcwiki.eu/index.php/Arnold_(Emulator)

## Building

### macOS

To build on macOS, install the dependencies using [Homebrew](https://brew.sh).

```shell
brew install cmake wxwidgets
```

Note that `wxwidgets` 3.1.4 or later is required to build on Big Sur, due to some dylib
problems with (at least) version 3.0.5.

Download the latest SDL2 Runtime Binaries from https://www.libsdl.org/download-2.0.php
and copy the `SDL2.framework` folder using the Finder (this ensures that you preserve symlinks).
Arnold is known to compile with SDL2 2.0.14.

Then we can build the app:

```shell
cd src
./make_mac.sh
```

If there are SDL2-related issues, check whether you have an older version of SDL installed
in `/Library/Framework/SDL2.framework`. Download a new version and replace it as above.

Then to codesign the binary:

```shell
codesignidentity="Developer ID Application: Your Company"
codesign -v -s "$codesignidentity" --timestamp --deep --force --options=runtime,hard,kill ../exe/Release/arnold/arnold.app
```

Note that we must use force in order to replace codesigning on some of the frameworks.

Then to notarize the binary (assuming you're setup with Apple Developer certificates, otherwise
see https://developer.apple.com/documentation/xcode/notarizing_macos_software_before_distribution):

First, you need to setup your credentials once, replacing `TEAMID` and `APPLEID`:

```shell
xcrun notarytool store-credentials --team-id TEAMID --apple-id APPLEID Arnold
```

Then, each time you want to notarize the `arnold.zip` file:

```shell
xcrun notarytool submit --keychain-profile Arnold --progress --wait arnold.zip
xcrun stapler staple ../exe/Release/arnold/arnold.app
rm arnold.zip
```
