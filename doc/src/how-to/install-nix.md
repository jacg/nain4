# How to install `nix`

`nain4` is distributed and its dependencies are managed using
`nix`. The reasons for this are explained in [why nix?](./why-nix.md).

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

We *strongly* recommend using the `direnv` tool to seamlessly turn on
and off development environments depending on the working
directory. `direnv` will look for a `.envrc` file and execute it when
you `cd` into a directory.

To install `direnv` with `nix`[^2], run

```bash
nix profile install nixpkgs#direnv
```

After this, you will need to hook `direnv` to your shell. Follow [these instructions](https://direnv.net/docs/hook.html) to complete the installation.

Finally, every time you enter a *new* directory where `direnv` can be executed, you will need to actively allow the load of the environment. However, this is only needed the first time. To allow execution, run the following line from within the directory.

```bash
direnv allow
```

Now, every time you enter the directory you will see a message like:
```bash
direnv: loading .envrc
direnv export: +FOO +BAR +BAZ ...
```

and when you exit
```bash
direnv: unloading
```


[^2] For a non-nix installation see [the direnv webpage](https://direnv.net/)
