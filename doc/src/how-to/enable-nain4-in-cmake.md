# How to make nain4 available in a Geant4/cmake project

There are 3 options to include nain4 in your project:

1. Adding it as an external project (recommended)
2. Adding it as a subdirectory in your project
3. Creating an independent installation

## Adding Nain4 as a external project

Add this snippet to your CMakeLists.txt:

```
{{#include ../../../client_side_tests/client_fetch_content/CMakeLists.txt:fetch}}
```

The value given to `GIT_TAG` may be a replaced with a branch name (`origin/<branch-name>`) or a commit hash. When opting for the 'branch name' option, it's important to be aware of the potential risk of losing reproducibility. With each execution of CMake, the library may be updated to a different state, introducing changes that can impact the reproducibility of your project. This can lead to compilation or runtime errors, making it challenging to recreate specific build or runtime environments. Thus, we advise to use a commit hash or a tag instead.


## Adding Nain4 as a subdirectory to your project

Clone the Nain4 repository into your own project or create a symbolic link to the clone. Then simply add

```
{{#include ../../../client_side_tests/client_subdirectory/CMakeLists.txt:add_subdir}}
```

to your CMakeLists.txt.


## Install Nain4 independently

```
git clone https://github.com/jacg/nain4.git
cd nain4
cmake [-DCMAKE_INSTALL_PREFIX=/path/to/install/] -S nain4 -B build
cmake --build build --target install
```

Unless the option `CMAKE_INSTALL_PREFIX` is speficied, the files will be installed in `/path/to/nain4/install`.
Then add to your CMakeLists.txt:

```
{{#include ../../../client_side_tests/client_independent_installation/CMakeLists.txt:find_package}}
```

## Linking to Nain4

Regardless of the chosen option, you will also need to link the library to each target like:

```
{{#include ../../../client_side_tests/client_fetch_content/CMakeLists.txt:link}}
```
