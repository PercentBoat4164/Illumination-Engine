---
title: Home
layout: default
---

# Welcome to Illumination Engine!

Illumination Engine is a cross-platform game development engine that is written in C++ that works on Windows and Linux.

We hope to add macOS compatibility to the game engine in the coming months.

## Getting Started

To get started, you will need a few packages. Also, make sure that you are using your favorite IDE.

Here is a list of a few packages that you will need to download:

### Linux:

  Required: cmake, c++-9, libglu1-mesa-dev, mesa-common-dev, libxrandr-dev, libxinerama-dev, libxcursor-dev, libxi-dev

    sudo apt install cmake c++-9 libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
  
  Recommended: libglew-dev, libglfw3-dev, libglm-dev

    sudo apt install libglew-dev libglfw3-dev libglm-dev
  
  If you plan to compile the Vulkan Render Engine, you will need to install the Vulkan software development kit. To accomplish this task, install the Vulkan software development kit using the following list of commands:
    
    wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -;sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.2.176-focal.list https://packages.lunarg.com/vulkan/1.2.176/lunarg-vulkan-1.2.176-focal.list;sudo apt update;sudo apt install vulkan-sdk
    
   (Copy and paste the commands into your terminal)

Those are the packages that Linux users will need to download to use the render engine.

### Windows:

Required: [cmake](https://cmake.org/download/)

(More to come...)

## Build



## Support

If you are having any trouble, check out our [documentation](https://percentboat4164.github.io/CrystalEngine/docs/index.html) or [report an issue on Github](https://github.com/PercentBoat4164/CrystalEngine/issues).
