# How to make nain4 available in a Geant4/cmake project

There are 3 options to include nain4 in your project:

1. Adding it as an external project (recommended)
2. Adding it as a subdirectory in your project
3. Creating an independent installation

## Adding Nain4 as a external project

Add this snippet to your CMakeLists.txt:

```
include(FetchContent)

FetchContent_Declare(
  Nain4
  GIT_REPOSITORY https://github.com/jacg/nain4.git
  GIT_TAG        <tag>
  SOURCE_SUBDIR  nain4
)

FetchContent_MakeAvailable(Nain4)
```

where `<tag>` may be a branch name (`origin/<branch-name>`), a tag or a commit hash. When opting for the 'branch name' option, it's important to be aware of the potential risk of losing reproducibility. With each execution of CMake, the library may be updated to a different state, introducing changes that can impact the reproducibility of your project. This can lead to compilation or runtime errors, making it challenging to recreate specific build or runtime environments. Thus, we advise to use a commit hash or a tag instead.


## Adding Nain4 as a subdirectory to your project

Clone the Nain4 repository into your own project or create a symbolic link to the clone. Then simply add

```
add_subdirectory(nain4/nain4)
```

to your CMakeLists.txt.


## Install Nain4 independently

```
git clone https://github.com/jacg/nain4.git
mkdir nain4/build && cd $_
cmake [-DCMAKE_INSTALL_PREFIX=/path/to/install/] ../nain4/
make && make install
```

Unless the option `CMAKE_INSTALL_PREFIX` is speficied, the files will be installed in `/path/to/nain4/install`.
Then add to your CMakeLists.txt:

```
set(Nain4_DIR "/path/to/install/lib/cmake/Nain4/")
find_package(Nain4 REQUIRED)
```

## Linking to Nain4

Regardless of the chosen option, you will also need to link the library to your targets:

```
foreach(target <targets>)
  target_link_libraries(
    <target>
    PRIVATE
    ${NAIN4_LIBRARIES}
  )
endforeach()
```
