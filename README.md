# `nain4`

This repository is going to be the home of `nain4`. The process of extracting it from the repository of the project in which it was born, has just started, so this is very much a Work In Progress, and there is no documentation *yet*.

The rest of this README refers to the old primary use case of this repository: an Nix flake for easy provision of Geant4 user and application-developer environments.

# TL;DR

1. Clone this repository.
2. `cd` into it  (and type `direnv allow`, if prompted to do so).
3. Type `just run B1`.

If, after a lot of downloading and compiling, an image of a small simulated detector appears, hoooray! You have an environment in which you can execute the Geant4 examples, and develop and run your own Geant4 code.

If not, read on to see possible problems and fixes.

# Caveats

1. This is a Nix flake, therefore it won't work without a *flakes-enabled* (i.e. unstable) Nix.

2. I don't know how to get Geant4 to work in Nix on MacOS, so this only works on Linux. (This seems to have been fixed in a recent `nixpkgs` ? If so, it might work. I don't have the means to check it myself.)

# Activating the environment

If you have [direnv](https://direnv.net/) installed and configured, it will automatically activate the development environment when you enter this directory (though you will have to approve it the first time with `direnv allow`) and disable it when you leave the directory.

Without `direnv` you can manually enable the environment with `nix develop`, and disable it by quitting the shell started by `nix develop`.


# Something to try, that should work

Once the environment has been activated, try typing

``` shell
just run B1
```

This should copy the sources of the most basic example that is distributed with Geant4, into the local directory, configure it, compile it and execute it.

If all goes well, an image of a detector should appear. Try typing `/run/beamOn 10` in the `Session` box, and some simulated events should appear on the image.

You should be able to modify the source code (for example increase the value of `env_sizeZ` in `B1/src/DetectorConstruction.cc` (change it from 30 to 130, to make the change obvious)) and run your modified version by repeating the earlier command `just run B1`.

# Running other examples

Many of the other examples can be run in the same way: `just run <example-name>`. 

+ Some of them will fail because necessary libraries have not (yet) been provided in this flake. 

+ Others will fail because the internal organization of the example differs from that of the simple ones, which is assumed by `just run`.

  In many of these cases it should be fairly easy to figure out how to compile and execute the example by hand. The procedure tends to me something like
  
  1. Create a `build` subirectory the example's top-level directory
  2. `cd` into the newly-created `build` directory
  3. `cmake ..`
  4. `make -j N` (where `N` specifies the number of processors you want to use during compilation)
  5. Find the executable which was produced by the previous step, and execute it by preceding its name with `./` In the case of the B1 example, this would be `./exampleB1`. 

Fixes to these problems may be provided eventually. Don't hold your breath.

# Geant 4 configuration

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
