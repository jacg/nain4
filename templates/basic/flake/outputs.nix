{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, nain4
, ...
}: let
  inherit (nixpkgs.legacyPackages) pkgs;


  # TODO inject nain4 itself into most of these:

  dev-shell-packages = with nain4.deps;
    dev-deps ++ build-deps ++ test-deps ++ run-deps
    ++ pkgs.lib.optionals pkgs.stdenv.isDarwin []
    ++ pkgs.lib.optionals pkgs.stdenv.isLinux  []
  ;

  in {

    # packages.default = pkgs.stdenv.mkDerivation { stdenv = pkgs.clang_16.stdenv; } {
    #   # CHANGEME-pname: replace "my-package" with a name better-suited to your project
    #   pname = "my-package";
    #   version = "0.0.1";
    #   src = "${self}/src";
    #   # TODO nativeBuildInputs =
    # };

    # Executed by `nix run <URL of this flake> -- <args?>`
    # TODO apps.default = { type = "app"; program = "..."; };

    # Executed by `nix run <URL of this flake>#my-app`
    # TODO apps.my-app = { type = "app"; program = "<store-path>"; };

    # Used by `direnv` when entering this directory (also by `nix develop <URL to this flake>`)
    devShell = self.devShells.clang;

    # TODO Create functions which generate the shells

    # Activated by `nix develop <URL to this flake>#clang`
    devShells.clang = pkgs.mkShell.override { stdenv = nain4.packages.clang_16.stdenv; } {
      name = "my-nain4-app-clang-devenv";

      packages = dev-shell-packages ++ [
        nain4.packages.nain4
        nain4.packages.clang_16
      ];

      G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    };

    # Activated by `nix develop <URL to this flake>#gcc`
    # devShells.gcc = pkgs.mkShell {
    #   name = "my-nain4-app-gcc-devenv";

    #   packages = dev-shell-packages;

    #   G4_DIR = "${pkgs.geant4}";
    #   G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
    #   QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    # };

  }
