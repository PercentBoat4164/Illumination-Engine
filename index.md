---
title: Home
layout: default
---

# Welcome to Illumination Engine!

Illumination Engine is a cross-platform game development engine that is written in C++ that works on Windows and Linux.

We hope to add macOS compatibility to the game engine in the coming months.

## Getting Started

For the best experience using this graphics engine, please make sure that your graphics drivers are up-to-date.

To begin using this engine, you will need a few packages.

Here is a list of a few packages that you will need to download:

### - Linux

Required: g++-11, libglu1-mesa-dev, mesa-common-dev, libxrandr-dev, libxinerama-dev, libxcursor-dev, libxi-dev

`sudo apt install libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`
    
`sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test; sudo apt update; sudo apt install -y g++-11`
  
If you plan to compile the Vulkan Render Engine, you will need to install the Vulkan software development kit. To accomplish this task, install the Vulkan software development kit using the following commmand:
    
`wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -;sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.2.198-focal.list https://packages.lunarg.com/vulkan/1.2.198/lunarg-vulkan-1.2.198-focal.list;sudo apt update;sudo apt install vulkan-sdk`

In an upcoming release, users won't need to install the Vulkan SDK.

If you are not using CLion or another IDE with CMake bundled, you will need to install CMake:

`sudo apt install cmake`
    
(Copy and paste the commands into your terminal)

Those are the packages that Linux users will need to download to compile Illumination Engine.

### - Windows

Required: [Vulkan](https://vulkan.lunarg.com/sdk/home)

In an upcoming release users won't have to install the Vulkan SDK.

If you are not using CLion or another IDE with CMake bundled, you will need to install [CMake](https://cmake.org/download/).

When installing CMake, make sure that the `install to path` option is selected.

Additionally, if you don't plan to use CLion or another IDE with a GCC v11.2 compatible compiler built in, you'll need to
install [MiniGW v9.0](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download).

Windows users will need the above packages installed on their machine to compile Illumination Engine.

## Building Illumination Engine

### IDE Specific Instructions

#### - CLion

To build Illumination Engine with CLion, load the project folder into the IDE. CLion should automatically
import the CMake projects. Make sure that the configuration is set to build the Illumination Engine project,
your c++ compiler location is set, and your GCC 11 compatible compiler must be set. Finally, click the run button, and CLion should compile and run
the project.

### Terminal Instructions

#### - Linux

To build Illumination Engine without an IDE on Linux, make sure that you have the required packages installed.
Then, open your terminal window inside
the project folder and type the command `mkdir build`. This will create the folder that will hold all the files
used to build the program. Next, type the command `cd build`. This moves you into the folder
you just created. Next, type the command `cmake ..`. This will generate a makefile that can be used
to build the project. After it is generated, type the command `make`. This will take a moment and
should successfully build your project if your computer has all the libraries listed above.
Finally, to run what was just built, enter the command `./IlluminationEngine`.
This will launch the Illumination Engine program that was just built.

#### - Windows

To build Illumination Engine without an IDE on Windows, make sure that you have the required packages installed.
Then, open a Windows Powershell window inside the project folder and enter the command `mkdir build`.
This will create the folder that will hold all the files used to build the program.
Next, type the command `cd build`. This moves you into the folder
you just created. Next, type the command `cmake ..`. This will generate a makefile that can be used
to build the project. After it is generated, type the command `make`. This will take a moment and
should successfully build your project if your computer has all the libraries listed above.
Finally, to run what was just built, enter the command `./IlluminationEngine`.
This will launch the Illumination Engine program that was just built.

## Support

If you are having any trouble, check out our [documentation](https://percentboat4164.github.io/Illumination-Engine/docs/index.html) or [report an issue on Github](https://github.com/PercentBoat4164/Illumination-Engine/issues).
