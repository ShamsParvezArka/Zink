# The Zink Project

> Watch this [video demo](https://youtu.be/10KHCzyjaIk?si=jCqsO029LgUpOTMk)

Zink is a native, user-mode application. It's main focus is to zoom in/out 
screen using mouse wheel. It currently only supports Windows(x64) operating 
system. In the future it may expand to also support native Linux system.

Zink is currently in *ALPHA*. Some features may come and go time to time. The
Zink project aims to reduce the friction between user and their visual 
workspace, providing a seamless and intuitive way to magnify on-screen content 
without breaking workflow or performance.

You can download pre-built binaries for Zink 
[here](https://github.com/ShamsParvezArka/Zink/releases/)

# Development Setup Instructions

> Note: Currently, only x64 Windows development is supported.
### 1. Install the required tools
- MSVC
- Window SDK

### 2. Build Environment Setup
Building the codebase can be done in a terminal which is equipped with the
ability to call MSVC.

This is generally done by calling `vcvarsall.bat x64`, which is included in the
Microsoft C/C++ Build Tools. This script is automatically called by the `x64
Native Tools Command Prompt for VS <year>` variant of the vanilla `cmd.exe`. If
you've installed the build tools, this command prompt may be easily located by
searching for `Native` from the Windows Start Menu search.

### 3. Building
Within the `cmd`, cd to the root directory of the codebase and just run the
`build.bat` script
```
build
```
The entire project should build in _less than 3 seconds_ and you're good to go.
You can find the executable in the `build` directory.

# Usage
| Control                                             | Description            |
|-----------------------------------------------------|------------------------|
| <kbd>Ctrl</kbd> + <kbd>Win</kbd> + <kbd>Space</kbd> | Trigger Zink           |
| <kbd>Esc</kbd>                                      | Exit Zink              |
| <kbd>Scroll</kbd> or <kbd>Pinch</kbd>               | Zoom in/out            |
| Drag with <kbd>left mouse</kbd> button              | Move the image around  |
| <kbd>R</kbd>                                        | Reset view             |

# Limitations
`Zink` may not appear at the top(foreground activation) the first time when
it's triggered due to Windows **focus-stealing prevention**. This is normal 
Windows behavior.
