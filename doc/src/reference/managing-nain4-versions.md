# Managing nain4 versions

The version of `nain4` used by your project is determined by two files in its top-level directory:

1. `flake.nix`
2. `flake.lock`

By default, `flake.nix` contains the line

```nix
nain4.url = "github:jacg/nain4";
```
which states that `nain4` should be taken from the default branch (`master`) of the `nain4` repository owned by user `jacg` on GitHub.

However

+ the official `master` branch receives frequent updates, and
+ you don't want the version of `nain4` that your project uses to change without
  your knowledge and consent.

This is the purpose of `flake.lock`: it pins the precise version (the commit id) of all of your project's `flake.nix`-specified dependencies.

## Upgranding to the lastest version of `nain4` available of `master`

If you want to upgrade to the most recent version of `nain4` available on its `master` branch, see [here](../how-to/upgrade-nain4.md).

## Using other versions of `nain4`

By default, `flake.nix` instructs your system to use the latest `master` and `flake.lock` pins it to a specific version of `master`. But you can specify other versions of any dependency, including ones which are not located in the official repository. Here a few examples of how you might modify the corresponding line in your project's `flake.nix`:

+ Use a preview of a feature being developed on the `some-experimental-feature` branch in the `nain4` repo.

    `nain4.url = "github:jacg/nain4?branch=some-experimental-feature"`


+ Use a specific tagged version of `nain4`.

    `nain4.url = "github:jacg/nain4?ref=v1.2.3"`

+ Use a specific commit available in the `nain4` repo.

    `nain4.url = "github:jacg/nain4?ref=9b4699ca25539d41bd1f5965a341be5e8ff862f1"`

+ Use a version of `nain4` made available in a repo other than the official `nain4` one.

    `nain4.url = "github:gonzaponte/nain4?branch=cool-idea"`

+ Use a version of `nain4` being developed on your own machine.

  `nain4.url = "/home/me/src/nain4?branch=cool-idea"`

For the full details, see the [Nix manual](https://nixos.org/manual/nix/unstable/command-ref/new-cli/nix3-flake#url-like-syntax).

Don't forget that, after switching to a different version of `nain4`, you may need to `just clean` and recompile your code, to avoid mysterious and annoying errors caused by stale build artefacts. (Hopefully this will be done for you automatically once `nain4` migrates its build infrastructure from `cmake` to `meson`.)

