# How to start a nain4-based project

## Setting up the project

In order to create a `nain4`-based project, we recommend starting with
one of the templates offered by `nain4`. Assuming you have `nix`
installed (if not, please check, [Install
nix](./install-nix.md)), simply run the following commands

// Is there something we can do to remove the last one?
```bash
nix flake new -t github:jacg/nain4 your-project-folder
cd your-project-folder
git init && git add . && git commit -m "Bootstrap project from template"`
```

## Project contents

This will generate a directory with the following contents

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

The `src` folder contains the source code and the `CMakeLists.txt`
file, which is used to compile the project. The `macs` directory
contains macro files, used to provide some app configuration
parameters at runtime. The `justfile` file defines some recipes for
easier usage of the application.  The `flake` directory and
`flake.lock`, `flake.nix` files describe the dependency tree for your
project. In most cases, you will not need to read or modify them.


## Compiling the project

Using `just`[^1], you can simply compile the program using

```bash
just build
```

All the files related to the compilation of the project will be stored
in a `build` directory. You can ignore this folder, which will not be
tracked by `git`. In some rare scenarios, you might need to erase the
compilation products before attempting to recompile your project. In
that case run

```bash
just clean
```

[^1] This command is provided by the app template.


## Running the app

Using `just`, you can run the app with

```bash
just run
```

This will compile the program if needed, so you don't need to compile
it yourself. Without any arguments, the previous line will open the
interactive GUI, where you will be able to explore the geometry in a
visual manner. This GUI accepts the usual G4 commands, like
`/run/beamOn <N>`

In order to run the simulation in batch mode, you can provide a macro
file

```bash
just run <macro_file>
```

or provide it with a certain number of events with

```bash
just run <N>
```

In this case, the macro `macs/run.mac` will be used.


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
