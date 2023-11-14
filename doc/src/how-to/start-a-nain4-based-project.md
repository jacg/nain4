# How to start a nain4-based project

The following instructions assume that you have `nix` installed. If
not, please see [Install nix](./install-nix.md).

## Setting up a new `nain4` project

In order to create a `nain4`-based project use `bootstrap-client-project` like this:

```bash
nix run github:jacg/nain4#bootstrap-client-project path/to/your/project your-chosen-name "one line project description"
```
where
+ `path/to/your/project` should not exist before you run the command.
+ `your-chosen-name` should be replaced with a name of your choice: this should
  be a valid identifier in both `bash` and the `Nix` language, so, roughly
  speaking, it should not contain any spaces, slashes or other dodgy characters.
  It will be used to name various things within your project.
+ `"one line project description"` should be replaced with some text of your
  choice. Don't forget the quotes.

## Basic usage

+ `just run -n 100`: run the application in batch mode with `/run/beamOn 100`
+ `just run -g`: run the application with the interactive GUI
  WARNING: unless you are running on
  + `NixOS`
  + `macOS` this will probably take a long time the first time you try it, as it
  will download and compile `nixGL` which is used to detect graphics hardware
  automatically and guarantee the presence of the required graphics drivers. On
  subsequent runs, this overhead will disappear.
+ TODO: describe the early/late CLI options
+ TODO: describe the macro path CLI options

## Project contents

Your bootstrapped project will contain the following files

```
path/to/your/project
├── execute-with-nixgl-if-needed.sh
├── flake
│  └── outputs.nix
├── flake.lock
├── flake.nix
├── justfile
├── macs
│  ├── early-cli.mac
│  ├── early-hard-wired.mac
│  ├── late-cli.mac
│  ├── late-hard-wired.mac
│  ├── run.mac
│  ├── vis.mac
│  └── vis2.mac
├── run-each-test-in-separate-process.sh.in
├── src
│  ├── LXe.cc
│  ├── LXe.hh
│  ├── meson.build
│  └── n4app.cc
└── test
   ├── catch2-main-test.cc
   ├── meson.build
   ├── test-catch2-demo.cc
   └── test-LXe.cc
```

+ TODO: discuss the contents of `src` and `test`

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
