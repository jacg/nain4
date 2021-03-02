# Caveats

1. This is a Nix flake, therefore it won't work without a *flakes-enabled* (i.e. unstable) Nix.

2. I don't know how to get Geant4 to work in Nix on MacOS, so this only works on Linux.

# Something to try, that should work

```shell
nix build github:jacg/g4-examples-flake --out-link /tmp/g4b1
tree /tmp/g4b1 # (assuming you have `tree`)
nix develop
cd $B1_MACRO_DIR
/tmp/g4b1/bin/exampleB1 # Tries to use OpenGL so YMMV
```

# Things that don't work

1. The executable absolutely needs to know the location of the G4 datasets (and probably some other crucial info). This is done by sourcing `${geant4}/bin/geant4.sh`, which seems to happen automatically in `nix develop` (I'm not sure how), but I don't know how to make that happen for `nix shell` or `nix run`.

2. G4 programs are supposed to be executed from the directory containing the `*.mac` files which control the run. In this flake, that's `B1_MACRO_DIR`. I don't know how to get `nix shell` and `nix run` to `cd B1_MACRO_DIR`.

Consequently `nix shell` and `nix run` don't do anything sensible. The latter does demonstrate the need to know the aforementioned G4 data directories, by crashing with the error
```
G4ENSDFSTATEDATA environment variable must be set
*** Fatal Exception *** core dump ***
```
.
