[![built with nix](https://builtwithnix.org/badge.svg)](https://builtwithnix.org)

![GHA tests](https://github.com/github/docs/actions/workflows/test.yml/badge.svg)

# What is this?

`nain4` is an API and accompanying set of libraries whose aim is to make it
easier to write, test and deploy [Geant4](https://geant4.web.cern.ch/)
applications.

# Documentation

The documentation for `nain4` is gradually being written
[here](https://jacg.github.io/nain4/).

# Nix

Nain4 uses [`Nix`](https://nixos.org/) to manage dependencies, installation and
provision of the development environment. The value proposition is: If *you*
[install nix](https://determinate.systems/posts/determinate-nix-installer) on
your machine *we* can provide a zero-effort means of installing Geant4 plus
dependencies and development tools, and making sure that everything has
compatible versions and works together in harmony[^1].

[^1]: Somewhere in the repository, we have provided the means to install nain4
    without the use of Nix, but we do not have the resources or the motivation
    to maintain this. If something in `nain4` is broken when using it via Nix,
    then we will aim to fix it; if something is broken when using `nain4`
    without Nix, then we won't be able to help.

For HPC systems on which installing Nix might be problematic:

+ [nix-portable](https://github.com/DavHau/nix-portable) works well in many situations
+ we provide the means to generate Singularity/[Apptainer](https://apptainer.org/) containers.

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

The whole[^2] B1 example is translated into nain4
[here](https://github.com/jacg/nain4/blob/master/n4-examples/B1/src/b1.cc), and,
even though it is written in a style that places almost every argument on a
separate line, it fits in a single file in under 150 lines, compared to the 1138
lines spread over 13 files in the original Geant4 rendition.


[^2]: This is not *strictly* true. The original B1 example caters for multiprocessing. As we actively discourage (TODO link to section in docs) using multiprocessing in Geant4, this translation into nain4 is not *exactly* equivalent. However, the differences are absolutely minimal.

# Getting started

1. [install nix](https://determinate.systems/posts/determinate-nix-installer)

   ```bash
   curl --proto '=https' --tlsv1.2 -sSf -L https://install.determinate.systems/nix | sh -s -- install
   ```

2. Bootstrap a new `nain4` project:
   ```sh
   nix run github:jacg/nain4#bootstrap-client-project path/to/your-new-project your-project-name "Your project description"
   ```
   This will create a new directory in `path/to/your-new-project`. Adapt this path to your needs *before* executing the command.

   Also adapt `your-project-name` (used in various identifiers and path components inside your project) and `"Your project description"` before executing the command.

3. `cd` into `path/to/your-new-project`

4. Type `nix develop` [^3]

   This step will take a while, the very first time you do it: it will download
   and compile Geant4. Thereafter, the build result is cached, and subsequent
   invocations should take under a second.

5. Type `just run -g -n 10`.

   This step compiles the example application in your new project and runs it in
   interactive mode. A visualization window should pop up.

[^3]: see the section on [direnv](#automatic-environment-switching-with-direnv) for a more ergonomic alternative

If all this worked, hooray! You have a git repository containing a development
environment in which you can use to evolve, test and deploy your `nain4`-based
Geant4 application.

If not, read on to see possible problems and fixes.

# Automatic environment switching with `direnv`

As it stands you have to write `nix develop` in order to activate the
environment necessary to run and develop this code. What is more, `nix develop`
places you in a minimal bash shell in which any personal configurations you may
be used to, will be missing.

Both of these problems can be fixed with [direnv](https://direnv.net/) which:

  * automatically enables the environment when you enter the directory (asking
    your permission, the first time)
  * updates the environment in whatever shell you happen to be using, thus
    allowing you to enjoy your previous settings.

To use `direnv`:

1. Make sure that it is [installed](https://direnv.net/docs/installation.html)
   on your system. If you have got this far, then you have already installed the
   Nix package manager on your machine, so you could use it to install `direnv`
   like this:
   ```sh
   nix profile install nixpkgs#direnv
   ```

2. Don't forget to [hook](https://direnv.net/docs/hook.html) `direnv` into your
   shell. Depending on which shell you are using, this will involve adding *one*
   of the following lines to the end of your shell configuration file:

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

# Operating systems

+ Linux: This code is developed and tested on Linux. If the above procedure
  didn't work, it's a bug. Please report it.

+ Windows: It should work equally well in WSL2 on Windows.

  Caveat: if you are going to install Nix in multi-user mode, make sure that `systemd` is enabled. In short, this requires you to ensure that the file `/etc/wsl.conf` exists in your in-WSL linux and that it contains the lines
  ```bash
  [boot]
  systemd=true
  ```
+ MacOS: Everything seems to work on macOS, with both Intel processors and Apple
  Silicon, but it has not been tested as extensively as on Linux.

# Graphics drivers

The interactive mode uses a Qt-based GUI. This requires the correct graphics
drivers to be installed in a location known to Nix. This should work out of the box on

+ NixOS
+ MacOS

On other systems, that is to say non-NixOS Linuxes and WSL2 on Windows, the
environment will try to use [`nixGL`](https://github.com/nix-community/nixGL) to
provide the appropriate drivers. WARNING: the first time `nixGL` is needed, it
will take a *long* time to download and compile.


<!-- # TODO Running examples -->
