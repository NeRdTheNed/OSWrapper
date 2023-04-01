# OSWrapper

> Single-header file libraries to wrap OS-specific functionality.

| Library           | Description                      | Platform implementations |
| ----------------- | -------------------------------- | ------------------------ |
| oswrapper_image.h | Image decoder using OS libraries | macOS                    |

## Usage

These libraries are single-header file libraries.
Include them in exactly one C file as such:

```C
#define OSWRAPPER_LIBRARYNAME_IMPLEMENTATION
#include "oswrapper_libraryname.h"
```

Replace `libraryname` with the name of the library.

Unlike standard single-header file libraries, you'll generally need to
link against a system library to use these libraries. Here's the linker requirements:

| Library           | macOS             |
| ----------------- | ----------------- |
| oswrapper_image.h | -framework AppKit |

## Future work

- Windows GDI+ image decoding
- Emscripten image decoding

Issues and PRs are welcome.

## License

All OSWrapper libraries are licensed under the BSD Zero Clause License.
