# How to start a nain4-based project

The following instructions assume that you have `nix` installed. If
not, please see [Install nix](./install-nix.md).

## Setting up a new `nain4` project

In order to create a `nain4`-based project use `bootstrap-client-project` like this:

```bash
nix run github:jacg/nain4#bootstrap-client-project path/to/your/project
```

where `path/to/your/project` should not exist before you run the command.


## Basic usage
+ `just run 100`: run the application in batch mode with `/run/beamOn 100`
+ `just run macs/run.mac`: run the application using the macro file `macs/run.mac`
+ `just run`: run the application with the interactive GUI
  WARNING: this will probably crash unless you are running on
  + `NixOS`
  + `macOS`
  To run the GUI on other OSes you need to use a graphics hardware
  abstraction helper. There is a `just` recipe which automatically
  includes this for you:
  ```bash
  just view
  ```
  The first time you use it, it will take a while to compile.

## Project contents

Your bootstrapped project will contain the following files

// TO BE UPDATED WITH TESTS
```
your-project-folder
├── flake
│   └── outputs.nix
├── flake.lock
├── flake.nix
├── justfile
├── macs
│   ├── run.mac
│   └── vis.mac
└── src
    ├── CMakeLists.txt
    └── n4app.cc
```

+ `src/n4app.cc` contains the entire c++ source code of your
  project. As the project grows, you may wish to break this up into
  smaller source files. You can add as many header (`.hh`) and
  implementation (`.cc`) files as you wish.

+ `src/CMakeLists.txt` is responsible for the details of the build
  process. If you do not change the directory structure of your
  project, you should not need to change this file.

+ The `macs` directory contains Geant4 macro files, used to provide
  some app configuration parameters at runtime.
  + `macs/run.mac` controls aspects of the simulation
  + `macs/vis.mac` controls aspects of visulisation (only used in interactive mode)

+ The `justfile` file defines some recipes for easier usage of the
  application.

+ The `flake` directory and `flake.lock`, `flake.nix` files describe
  the dependency tree for your project. In most cases, you will not
  need to read or modify them.


## Make it *your* project

Because this is a template, there are a number of variables that take
a generic name. We encourage you to change them to represent something
meaningfull to you. You can find these variables by running
```bash
grep -Rn CHANGEME .
```

on the root folder of your project. You will see an output with
several lines like

```
[file]:[line-number]:#CHANGEME: [description]
```

Open those files with your favourite text editor and rename those
variables.
