# OpenGL demos
Simple OpenGL demos programs using win32

## Table of contents
* [Introduction](#introduction)
* [Technology](#technology)
* [Setup](#setup)

## Introduction
This are basic and intermediate level OpenGL C++ program.  

### Technology
Project is created with:
* Visual Studio 2019 community version
* msdocs understanding for win32 API's
* basic c++ programming
* opengl32.lib and glu32.lib 

#### Setup
```
$ cl.exe /c /EHsc source_program.cpp
$ link.exe source_program.obj User32.lib gdi32.lib kernel32.lib /SUBSYSTEM:WINDOWS 

for resource compilation
$ rc.exe Resource.rc
```
##### Contribute

Contributions are always welcome!  
Please read the [contribution guidelines](contributing.md) first.

## License
Use of this project is governed by the MIT License found at [LICENSE](./LICENSE).
