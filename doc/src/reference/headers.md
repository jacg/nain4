# Library headers

`nain4` provides headers of different granularity, for convenience in different situations.

## `<n4-all.hh>`

Gives access to *all* `nain4` components. This header is meant to be used for exploration:

+ when you are just getting started with `nain4` and don't want to be distracted by the need to find specific headers for specific situations

+ when you want LSP to help you discover available features

+ When your whole `nain4` application still lives in a single source file

As your program grows and is split into multiple files, and you gain more experience with `nain4`, it would be more appropriate to use the more fine-grained headers, described below.

+ `<n4-main.hh>`

   Groups together utilities that are required to write the `main` function of a `nain4` application. These are also available via the separate headers: 
   + `<n4-run-manager.hh>`: `nain4` interface for configuring the Geant4 run manager
   + `<n4-mandatory.hh>`: `nain4` utilities for concise implementation of the user-defined classes that must be registered with the run manager

+ `<n4-geometry.hh>`

  Groups together functionality that is typically used in defining detector geometries. These are also available via the separate headers:
  + `<n4-material>`: finding existing materials; constructing new materials
  + `<n4-shape>`: construction of `G4VSolid`s
  + `<n4-boolean-shape>`: construction of boolean solids
  + `<n4-volume>` creation of logical volumes
  + `<n4-place>` creation and placement of physical volumes

+ `<n4-utils.hh>`

  Various ready-made conveniences to alleviate the tedium and verbosity of oft-encountered tasks.  These are also available via the separate headers:
  + `<n4-constants.hh>`: physical constants not provided by `CLHEP`
  + `<n4-inspect.hh>`: finding existing geometry components, materials, etc.
  + `<n4-random.hh>`: random number generation
  + `<n4-sequences.hh>`: convenient creation of sequences of numerical data
  + `<n4-stream.hh>`: redirecting or silencing C++ output streams

+ `<n4-testing.hh>`

  Utilities to help with writing automated tests for `nain4` applications. These are also available via the separate headers:
  + `<n4-defaults.hh>`: ready-made dummy application components
  + `<n4-geometry-iterators.hh>`: iterating over all sub-elements of a geometry
