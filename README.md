# LIB rM Lines
This project is largerly been made possible by all the findings made by the community. Projects like [rMscene](https://github.com/ricklupton/rmscene).

[![wakatime](https://wakatime.com/badge/github/RedTTGMoss/librm_lines.svg)](https://wakatime.com/badge/github/RedTTGMoss/librm_lines)

[![Development builds](https://github.com/RedTTGMoss/librm_lines/actions/workflows/build.yml/badge.svg?branch=experimental)](https://github.com/RedTTGMoss/librm_lines/actions/workflows/build.yml)

### This library is:
- A `.rm` lines reader
- A real time renderer
- ~~A `.rm` lines writer~~ *not yet*

## Building
The project contains a cmake file with everything preconfigured. 

**To build the shared library file**
```cmake build --target rm_lines```

You can also build the test executable
```cmake build --target test```

## Testing
*If you build the test executable just run it directly.*

If you build the shared library file following the above steps you could now run one of the python tests in the `tests` folder. 

Make sure that they are ran from inside the tests folder! You can install any missing packages, if you run into any other issue open it as an issue here. 

## Shoutouts
This project wouldn't have been possible without [rMscene](https://github.com/ricklupton/rmscene) and the open sourced project by reMarkable themselves [quill](https://github.com/CrimsonAS/quill) also a big shoutout to the Moss supporters too! 

Some of the test files included are by my fellow testers! Shout out to them too! 

## Usage & Releases
You can find versionned releases of the shared library on this repo. 

The shared library exposes a few basic functions to use. *(docs coming soon)*

You can also use wrappers:
- Python ([pyLIBrM_Lines](https://github.com/RedTTGMoss/pylibrm_lines)) 