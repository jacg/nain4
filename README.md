This repository is going to be the home of `nain4`. The process of extracting it from the repository of the project in which it was born, has just started, so this is very much a Work In Progress.

The documentation for `nain4` is gradually being written [here](https://jacg.github.io/nain4/).

# What is this?

This repository contains two orthogonal but related products:

1. `nain4`

   Utilities that make writing and testing Geant4 applications much easier.

2. A [Nix](https://zero-to-nix.com/) flake for easy provision of Geant4 user and application-developer environments.

   The value proposition is: If *you* [install Nix](https://zero-to-nix.com/start/install) on your machine *we* can provide a zero-effort means of installing Geant4 plus dependencies and development tools.

The flake is not necessary to *use* `nain4`, but an installation of Geant4 is necessary to *test* `nain4`.

Providing an easy means of using `nain4` without the flake is a top priority, but the extraction of `nain4` from its parent repository has only just begun, so this is not ready yet.

If you manage to persuade `cmake` to treat the contents of `<this-repo>/nain4/` as a package or library (sorry, not sure of the exact cmake nomencladure) in your Geant4 application, then it should work.

# `nain4`

Nain4 is a collection of utilities whose aim is to

1. make it significantly easier to write Geant4 applications, in code that is much more robust, readable and self-documenting;
2. make it possible/easy to write unit and integration tests for Geant4 application code;
3. make it more difficult to write invalid Geant4 code by promoting errors to compile time;

As a quick taster, here is a translation of the [detector geometry from Geant4 example `basic/B1` ](https://gitlab.cern.ch/geant4/geant4/-/blob/9f34590941fb8a3f7ad139731089ec3794947545/examples/basic/B1/src/DetectorConstruction.cc#L50-L165) into `nain4`:
```C++
// Materials
auto water  = n4::material("G4_WATER");
auto air    = n4::material("G4_AIR");
auto tissue = n4::material("G4_A-150_TISSUE");
auto bone   = n4::material("G4_BONE_COMPACT_ICRU");

// Dimensions
// ...
// world_sizeXY = ... unremarkable value settings elided for brevity
// ...

// Volumes
auto world     = n4::box ("World"   ).xy(world_sizeXY).z(world_sizeZ)  .volume(air);
auto envelope  = n4::box ("Envelope").xy(  env_sizeXY).z(  env_sizeZ)  .volume(water);
auto trapezoid = n4::trd ("Bone"    ).x1(trd_dxa).x2(trd_dxb)
                                     .y1(trd_dya).y2(trd_dyb).z(trd_dz).volume(bone);

// If no handle is needed for a logical volume, it can be placed immediately
n4::cons("Tissue").r1(cone_rmaxa).r2(cone_rmaxb).z(cone_hz).place(tissue).in(envelope).at_yz(2*cm, -7*cm).now();

// Set Trapezoid as scoring volume
this -> fScoringVolume = trapezoid;

// Placement
n4::       place(envelope) .in(world)                      .now();
n4::       place(trapezoid).in(envelope).at_yz(-1*cm, 7*cm).now();
return n4::place(world)                                    .now();
```
This is the complete (except for setting of the values like `world_sizeXYZ`) `nain4` implementation of `DetectorConstruction::Construct()`.

These 13 lines of code (without comments or blank lines) correspond to 62 lines in the original example.

In Geant4's interfaces, you have to remember (or look up) the order of the parameters of the shape constructors, and you must provide values for each parameter, even when you want to use an obvious default value (such as inner `radius` being zero, or `phi` covering `2π`); in `nain4` the parameters have clear names and can be provided in any order that you find convenient; obvious default values can be omitted.

Geant4 obliges you to express everything in (frequently annoying) half-lengths; `nain4` gives you the choice: `.x` vs `.half_x`.

# The Nix flake

## Getting started
1. [install nix](https://determinate.systems/posts/determinate-nix-installer)

   ```bash
   curl --proto '=https' --tlsv1.2 -sSf -L https://install.determinate.systems/nix | sh -s -- install
   ```

2. Clone this repository: `git clone https://github.com/jacg/nain4`

3. `cd` into it

4. Type `nix develop`

   This step will take a while, the very first time you do it: it will download and compile Geant4. Thereafter, the build result is cached, and subsequent invocations should take under a second.

5. Type `just g4-examples/run B1`.

   This step compiles and runs the `basic/B1` example that is distributed with Geant4, in interactive mode. A visualization window should pop up.

If all this worked, hooray! You have an environment in which you can execute the Geant4 examples, and develop and run your own Geant4 code.

If not, read on to see possible problems and fixes.

## Operating systems

+ Linux: This code is developed and tested on Linux. If the above procedure didn't work, it's a bug. Please report it.

+ Windows: It should work equally well in WSL2 on Windows.

  Caveat: if you are going to install Nix in multi-user mode, make sure that `systemd` is enabled. In short, this requires you to ensure that the file `/etc/wsl.conf` exists in your in-WSL linux and that it contains the lines
  ```bash
  [boot]
  systemd=true
  ```
+ MacOS: Everything seems to work on macOS, with both Intel processors and Apple Silicon, but it has not been tested extensively.

## Ergonomics

### Automatic environment switching with `direnv`

As is stands you have to write `nix develop` in order to activate the environment necessary to run and develop this code. What is more, `nix develop` places you in a minimal bash shell in which any personal configurations you may be used to, will be missing.

Both of these problems can be fixed with [direnv](https://direnv.net/) which:
  * automatically enables the environment when you enter the directory (asking your permission, the first time)
  * updates the environment in whatever shell you happen to be using, thus allowing you to enjoy your previous settings.

To use `direnv`:

1. Make sure that it is [installed](https://direnv.net/docs/installation.html) on your system.

2. Don't forget to [hook](https://direnv.net/docs/hook.html) it into your shell.
   Depending on which shell you are using, this will involve adding
   one of the following lines to the end of your shell configuration
   file:

   ```bash
   eval "$(direnv hook bash)"  # in ~/.bashrc
   eval "$(direnv hook zsh)"   # in ~/.zshrc
   eval `direnv hook tcsh`     # in ~/.cshrc
   ```

The first time `direnv` wants to perform an automatic switch in a new
context (combination of directory + `.envrc` contents), it asks you
for permission to do so. You can give it permission by typing `direnv
allow` in the shell. The message that `direnv` gives you at this stage
is pretty clear, but it's usually written in red, thus you might get
the mistaken impression that there is an error.

## Geant 4 configuration

Various configuration options of Geant4 itself can be changed by editing `flake.nix` here:

``` nix
(geant4.override {
  enableMultiThreading = false;
  enableInventor       = false;
  enableQt             = true;
  enableXM             = false;
  enableOpenGLX11      = true;
  enablePython         = false;
  enableRaytracerX11   = false;
})
```

If you change the Geant4 configuration (if you are using `direnv`, it will notice the change, and automatically switch to the new configuration (recompiling Geant4, if this is a configuration not seen before) at your next shell prompt).

Be sure to expunge any examples you had compiled with a differently-configured Geant4, otherwise you may get mysterious problems.

## Standard Geant4 examples

This repository also allows you to run and edit the standard Geant4 examples. For example,

```bash
just g4-examples/run B1
```

This should copy the sources of the most basic example that is distributed with Geant4, into the `g4-examples` directory, configure it, compile it and execute it.

If all goes well, an image of a detector should appear. Try typing `/run/beamOn 10` in the `Session` box, and some simulated events should appear on the image.

You should be able to modify the source code (for example increase the value of `env_sizeZ` in `B1/src/DetectorConstruction.cc` (change it from 30 to 130, to make the change obvious)) and run your modified version by repeating the earlier command `just G4-examples/run B1`.

### Running other examples

Many of the other examples can be run in the same way: `just g4-examples/run <example-name>`.

+ Some of them will fail because they require Geant4 to be compiled with multithreading enabled. By default, multithreading is disabled in `flake.nix`.

+ Others will fail because the internal organization of the example differs from that of the simple ones, which is assumed by `just g4-examples/run`.

  In many of these cases it should be fairly easy to figure out how to compile and execute the example by hand. The procedure tends to me something like

  1. Create a `build` subirectory the example's top-level directory
  2. `cd` into the newly-created `build` directory
  3. `cmake -S . -B build`
  4. `cmake --build build -j`
  5. Find the executable which was produced by the previous step, and execute it by preceding its name with `./` In the case of the B1 example, this would be `./exampleB1`.
