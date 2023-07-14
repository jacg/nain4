# How to create a minimal nain4 app

The most basic example of a nain4 app can be found in [examples/n4app.cc](../../../examples/n4app.cc). Here is the full file:

```c++
{{#include ../../../examples/n4app.cc:full_file}}
```

Now, let's split it into bite-sized portions. First we create a simple program that reads the number of events from CLI:
```c++
{{#include ../../../examples/n4app.cc:pick_cli_arguments}}
{{#include ../../../examples/n4app.cc:closing_bracket}}
```

In order to run a simulation we need to create a run manager:
```c++
{{#include ../../../examples/n4app.cc:create_run_manager}}
```

The run manager needs to be initialized with 4 attributes:
- A physics list
- A geometry
- A primary generator
- A set of actions

Note that the physics list must be instantiated and given to the run manager before the primary generator is instantiated. We provide the run manager with these attributes:
```c++
{{#include ../../../examples/n4app.cc:build_minimal_framework}}
```
here, `my_geometry`, and `my_generator` are the two functions we need to provide to run our simulation:
```c++
{{#include ../../../examples/n4app.cc:my_geometry}}

{{#include ../../../examples/n4app.cc:my_generator}}
```

With this, our setup is ready, and we can start the simulation with
```c++
{{#include ../../../examples/n4app.cc:run}}
```

Don't forget to add the relevant includes
```c++
{{#include ../../../examples/n4app.cc:includes}}
```

To run this test type `just test-examples` from the top `nain4` directory.
