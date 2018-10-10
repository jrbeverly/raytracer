# RayTracer

## Summary

A Raytracer that receives a scene defined in lua, and produces an image output.

## Getting Started

Compilation follows the standard process defined by the UWaterloo CS488 sample projects.

We use **premake4** as our cross-platform build system. First you will need to build all
the static libraries that the projects depend on. To build the libraries, open up a
terminal, and **cd** to the top level of the CS488 project directory and then run the
following:

```bash
premake4 gmake
make
```

This will build the following static libraries, and place them in the top level **lib** folder of your cs488 project directory.

* libcs488-framework.a
* libglfw3.a
* libimgui.a

**NOTE:** As the following is only the relevant Raytracer files, the build system/dependencies and premake4.lua definition files are all missing. The files in this project are provided as-is.

### Dependencies

* OpenGL 3.2+
* GLFW
  * http://www.glfw.org/
* Lua
  * http://www.lua.org/
* Premake4
  * https://github.com/premake/premake-4.x/wiki
  * http://premake.github.io/download.html
* GLM
  * http://glm.g-truc.net/0.9.7/index.html
* AntTweakBar
  * http://anttweakbar.sourceforge.net/doc/

## Notes

Objectives were completed as defined by the assignment.

The sample.lua scene that is defined is based on the simple.lua, and uses a compositions of items from the sample
lua files.

Features:

* Uses a multi-threaded design to increase performance (number of threads can be specified at compile-time)
* Mirror reflections was the supported offical feature that was added to the project

Standing Issues:

* Currently there is an issue with the Mesh models.  As it stands they only partially render (certain values do)
* This likely has to do with an issue with the algorithm that was being used
* Originally used an angle based algorithm (described [here](http://paulbourke.net/geometry/polygonmesh/)) but it did not render the objects

## Acknowledgements

The project icon is retrieved from [the Noun Project](docs/icon/icon.json). The original source material has been altered for the purposes of the project. The icon is used under the terms of the [Public Domain](https://creativecommons.org/publicdomain/zero/1.0/).

The project icon is by [Arthur Schmitt from the Noun Project](https://thenounproject.com/term/laser-cutter/18232/).
