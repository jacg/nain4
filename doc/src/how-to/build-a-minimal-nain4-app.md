# How to create a minimal nain4 app

## Building the app

One of the most basic examples of a nain4 app can be found in [templates/basic/src/n4app.cc](../../../templates/basic/src/n4app.cc). Here is the full file:

```c++
{{#include ../../../templates/basic/src/n4app.cc:full_file}}
```

Now, let's split it into bite-sized portions. First we create a simple program that reads the number of events from CLI:
```c++
{{#include ../../../templates/basic/src/n4app.cc:pick_cli_arguments}}
{{#include ../../../templates/basic/src/n4app.cc:closing_bracket}}
```

In order to run a simulation we need to create a run manager:
```c++
{{#include ../../../templates/basic/src/n4app.cc:create_run_manager}}
```

The run manager needs to be initialized with 3 attributes:
- A physics list
- A geometry
- A set of actions

To build the action set we need to provide a primary generator action. Other actions are optional.
Note that the physics list must be instantiated and given to the run manager before the primary generator is instantiated. We provide the run manager with these attributes:

```c++
{{#include ../../../templates/basic/src/n4app.cc:build_minimal_framework}}
```

here, `my_geometry`, and `my_generator` are the two functions we need to provide to run our simulation:

```c++
{{#include ../../../templates/basic/src/n4app.cc:my_geometry}}

{{#include ../../../templates/basic/src/n4app.cc:my_generator}}
```

and `create_actions` is defined as:

```c++
{{#include ../../../templates/basic/src/n4app.cc:create_actions}}
```


With this, our setup is ready, and we can start the simulation with
```c++
{{#include ../../../templates/basic/src/n4app.cc:run}}
```

Don't forget to add the relevant includes
```c++
{{#include ../../../templates/basic/src/n4app.cc:includes}}
```

## Compiling the app

The app comes with its own `CMakeLists.txt`:

```cmake
{{#include ../../../examples/00-basic/CMakeLists.txt:full_file}}
```

This contains the following bits. First, we have the project initialization:

```cmake
{{#include ../../../examples/00-basic/CMakeLists.txt:project_setup}}
```

It specifies the minimum cmake version, defines the project name and some lines necessary for code analysis in IDEs.
Next, we include `nain4` and `Geant4` in our project:

```cmake
{{#include ../../../examples/00-basic/CMakeLists.txt:include_nain4_geant4}}
```

Note that we have chosen to use an existing installation of `nain4` so this example runs out of the box when run within `nain4`. If you were to copy this example and run it elsewhere, make sure to check [How to make nain4 available in a Geant4/cmake project](./enable-nain4-in-cmake.md).

Finally, we create an executable file and link it to the relevant libraries.

```cmake
{{#include ../../../examples/00-basic/CMakeLists.txt:create_exe_and_link}}
```

To compile the app we `cd` into `00-basic` and run `just compile-app` or
```
cmake -S . -B build && cmake --build build
```


## Running the app

To run this app type `just run-app <number of events>` (after compilation) from the `00-basic` directory. You can also achieve the same typing `./build/n4-00-basic <number of events>`.

As this simulation is extremelly basic, it doesn't produce anything. You will only see a Geant4 header.
