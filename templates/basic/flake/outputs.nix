{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, nain4
, ...
}: let
  inherit (nixpkgs.legacyPackages) pkgs;
  inherit (import ./helpers.nix {inherit pkgs;}) shell-shared;
  inherit (nain4.deps) args-from-cli make-app;
  in {

    packages.default = self.packages.CHANGEME-PACKAGE;

    # Executed by `nix run <URL of this flake>#CHANGEME-PACKAGE -- <args?>`
    # TODO: switch to clang environment
    packages.CHANGEME-PACKAGE = pkgs.stdenv.mkDerivation {
      pname = "CHANGEME-PACKAGE";
      version = "0.0.0";
      src = "${self}/src";
      postInstall = "${pkgs.coreutils}/bin/cp -r ${self}/macs $out";
      nativeBuildInputs = [];
      buildInputs = [ nain4.packages.nain4 ];
    };

    # Executed by `nix run <URL of this flake> -- <args?>`
    apps.default = self.apps.CHANGEME-EXE;

    # Executed by `nix run <URL of this flake>#CHANGEME-EXE`
    apps.CHANGEME-EXE = make-app {
      executable = "CHANGEME-EXE";
      args = "--macro-path ${self}/macs ${args-from-cli}";
      package = self.packages.default;
    };

    # Used by `direnv` when entering this directory (also by `nix develop <URL to this flake>`)
    devShell = self.devShells.clang;

    # Activated by `nix develop <URL to this flake>#clang`
    devShells.clang = pkgs.mkShell.override { stdenv = nain4.packages.clang_current.stdenv; } (shell-shared // {
      name = "CHANGEME-PROJECT-NAME-clang-devenv";
      packages = nain4.deps.dev-shell-packages ++ [ nain4.packages.clang_current ];
    });

    # Activated by `nix develop <URL to this flake>#gcc`
    devShells.gcc = pkgs.mkShell (shell-shared // {
      name = "CHANGEME-PROJECT-NAME-gcc-devenv";
      packages = nain4.deps.dev-shell-packages;
    });

    # 1. `nix build` .#singularity
    # 2. `scp result <me>@lxplus7.cern.ch:hello.img`
    # 3. [on lxplus] `singularity run hello.img`
    packages.singularity = pkgs.singularity-tools.buildImage {
      name = "CHANGEME-PROJECT-NAME";
      contents = [ self.apps.CHANGEME-EXE.program ];
      runScript = "${self.apps.CHANGEME-EXE.program} $@";
      diskSize = 10240;
      memSize = 5120;
    };

  }
