# OSWrapper

> Single-header file libraries to wrap OS-specific functionality.

[![Build](https://github.com/NeRdTheNed/OSWrapper/actions/workflows/build.yml/badge.svg)](https://github.com/NeRdTheNed/OSWrapper/actions/workflows/build.yml)

| Library           | Description                      | Platform implementations                        |
| ----------------- | -------------------------------- | ----------------------------------------------- |
| oswrapper_image.h | Image decoder using OS libraries | macOS, Windows (Vista and higher), Emscripten   |
| oswrapper_audio.h | Audio decoder using OS libraries | macOS (10.4 and higher), Windows (7 and higher) |

## Usage

These libraries are single-header file libraries.
Include them in exactly one C or C++ file as such:

```C
#define OSWRAPPER_LIBRARYNAME_IMPLEMENTATION
#include "oswrapper_libraryname.h"
```

Replace `libraryname` with the name of the library.

Unlike standard single-header file libraries, you'll generally need to
link against a system library to use these libraries,
or perform some platform-specific initialisation action before using them.
Here's the requirements:

| Library           | macOS                             | Windows                                                                              | Emscripten            |
| ----------------- | --------------------------------- | ------------------------------------------------------------------------------------ | --------------------- |
| oswrapper_image.h | Link with -framework AppKit       | Initialise the COM library, link with windowscodecs.lib                              | Compile with Asyncify |
| oswrapper_audio.h | Link with -framework AudioToolbox | Initialise the COM library, link with mfplat.lib, mfreadwrite.lib, and shlwapi.lib   | N/A                   |

Full examples of linking and using OSWrapper libraries can be found in the test folder.

## Future work

- Emscripten audio decoding

Issues and PRs are welcome.

## License

All OSWrapper libraries and test / demo applications are licensed under the BSD Zero Clause License.
