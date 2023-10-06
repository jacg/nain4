# How to upgrade your nain4 dependency

To upgrade `nain4` to the lastest version available on `master`:

1. `nix flake lock --update-input nain4`

   This operation modifies `flake.lock`. Commit these modifications, either right now, or after step 3.

2. `just clean`

    This ensures that there are no old build products which still use the old version. <font size=-1>(Hopefully this will no longer be necessary once `nain4` migrates its build infrastructure from `cmake` to `meson`.)</font>

3. Build, run and test the behaviour of your project, and fix any problems caused by incompatibility of your code with the new version of `nain4`.

4. Commit these changes to your repository. Don't forget to commit the modified `flake.lock`; this is important in order to ensure that every checkout of a given commit in your repository uses exactly the same version of dependencies and that the versions of the dependencies are compatible with the version of your code.


## Using other versions of `nain4`

It is possible to use versions of `nain4` other than those available on the official `master` branch. See [here](../reference/managing-nain4-versions.md) for details.
