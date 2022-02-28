# ScriptingModule

The goal of the scripting module is to create a way for game creators to interact with the engine's components, other assets, and the game world through scripts. Illumination Engine's API is not yet available in scripts.

## Requirements for Completion
- [x] Compilation of custom external scripts
- [x] Execution of compiled code
- [ ] Expose Illumination Engine's API to the scripts
- [ ] Allow for users to set custom values in scripts

## Goals
- [ ] Support for a variety of scripting languages
  - [x] Lua
  - [ ] C#
  - [ ] Python
  - [ ] C/C++
  - [ ] Java (maybe)

## Optional Packages
Some languages require additional packages to be installed in order to work. These often consist of the language's development packages as well as the language itself.

### Linux:
Install all the following packages with `sudo apt install <package_name>`

- For Lua Support:
  - liblua5.3-dev lua5.3