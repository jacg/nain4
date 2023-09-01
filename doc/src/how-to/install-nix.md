# How to install `nix`

`nain4` is distributed and its dependencies are managed using
`nix`. The reasons for this are explained in [why nix?](../why-nix.md).

## On your personal computer

In order to install `nix` on your personal computer, simply use[^1]

```bash
curl --proto '=https' --tlsv1.2 -sSf -L https://install.determinate.systems/nix | sh -s -- install
```

The installer will guide you through the installation steps. This
installer requires super-user permissions (`sudo`).

[^1]: From [https://determinate.systems/posts/determinate-nix-installer](https://determinate.systems/posts/determinate-nix-installer)


## On a HPC cluster

The situation on HPC clusters is different, as we usually we don't
have super-user permissions. At the moment the installation of nix in
this kind of environments is not fully supported. In the meantime, you
may want to check [How to make nain4 available in a Geant4/cmake
project](./how-to/enable-nain4-in-cmake.md)


## `direnv`

We *strongly* recommend using [`direnv`](https://direnv.net/) to automatically
activate and deactivate development environments when entering or
leaving a project directory.

To use `direnv`:

1. Make sure that it is [installed](https://direnv.net/docs/installation.html) on your system.
   To install `direnv` with `nix`, run

   ```bash
   nix profile install nixpkgs#direnv
   ```

2. Don't forget to [hook](https://direnv.net/docs/hook.html) it into your shell.
   Depending on which shell you are using, this will involve adding
   one of the following lines to the end of your shell configuration
   file:

   ```bash
   eval "$(direnv hook bash)"  # in ~/.bashrc
   eval "$(direnv hook zsh)"   # in ~/.zshrc
   eval `direnv hook tcsh`     # in ~/.cshrc
   ```

The first time `direnv` wants to perform an automatic switch in a new context
(combination of directory + `.envrc` contents), it asks you for permission to do
so. You can give it permission by typing `direnv allow` in the shell. The
message that `direnv` gives you at this stage is pretty clear, but it's usually
written in red, thus you might get the mistaken impression that there is an
error.

Now, every time you enter the directory you will see a message like:
```bash
direnv: loading .envrc
direnv export: +FOO +BAR +BAZ ...
```

and when you exit
```bash
direnv: unloading
```
