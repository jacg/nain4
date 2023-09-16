{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, nain4
, ...
}: let
  inherit (nixpkgs.legacyPackages) pkgs;


  # TODO inject nain4 itself into most of these:

  dev-shell-packages = with nain4.deps;
    [ nain4.packages.nain4 ] ++
    dev-deps ++ build-deps ++ test-deps ++ run-deps
    ++ pkgs.lib.optionals pkgs.stdenv.isDarwin []
    ++ pkgs.lib.optionals pkgs.stdenv.isLinux  []
  ;

  in {

    packages.default = self.packages.CHANGEME-my-package;

    # TODO: switch to clang environment
    packages.CHANGEME-my-package = pkgs.stdenv.mkDerivation {
      # CHANGEME-pname: replace "CHANGEME-my-package" with a name better-suited to your project
      pname = "CHANGEME-my-package";
      version = "0.0.0";
      src = "${self}/src";
      nativeBuildInputs = dev-shell-packages; # TODO be more discriminating in nativeBuildInputs
    };

    # Executed by `nix run <URL of this flake> -- <args?>`
    apps.default = self.apps.CHANGEME-my-app;

    # Executed by `nix run <URL of this flake>#CHANGEME-my-app`
    apps.CHANGEME-my-app = let
      g4-data = nain4.deps.g4-data-package;
      CHANGEME-wrap-my-package = pkgs.writeShellScriptBin "CHANGEME-my-app" ''
        export PATH=${pkgs.lib.makeBinPath [ self.packages.CHANGEME-my-package ]}:$PATH
        # export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath [ nain4.packages.geant4 ] }:$LD_LIBRARY_PATH

        # TODO replace manual envvar setting with with use of packages' setupHooks
        export G4NEUTRONHPDATA="${g4-data.G4NDL}/share/Geant4-11.0.4/data/G4NDL4.6"
        export G4LEDATA="${g4-data.G4EMLOW}/share/Geant4-11.0.4/data/G4EMLOW8.0"
        export G4LEVELGAMMADATA="${g4-data.G4PhotonEvaporation}/share/Geant4-11.0.4/data/G4PhotonEvaporation5.7"
        export G4RADIOACTIVEDATA="${g4-data.G4RadioactiveDecay}/share/Geant4-11.0.4/data/G4RadioactiveDecay5.6"
        export G4PARTICLEXSDATA="${g4-data.G4PARTICLEXS}/share/Geant4-11.0.4/data/G4PARTICLEXS4.0"
        export G4PIIDATA="${g4-data.G4PII}/share/Geant4-11.0.4/data/G4PII1.3"
        export G4REALSURFACEDATA="${g4-data.G4RealSurface}/share/Geant4-11.0.4/data/G4RealSurface2.2"
        export G4SAIDXSDATA="${g4-data.G4SAIDDATA}/share/Geant4-11.0.4/data/G4SAIDDATA2.0"
        export G4ABLADATA="${g4-data.G4ABLA}/share/Geant4-11.0.4/data/G4ABLA3.1"
        export G4INCLDATA="${g4-data.G4INCL}/share/Geant4-11.0.4/data/G4INCL1.0"
        export G4ENSDFSTATEDATA="${g4-data.G4ENSDFSTATE}/share/Geant4-11.0.4/data/G4ENSDFSTATE2.3"

        exec CHANGEME-my-n4-prog --macro-path ${self}/macs $*
      '';
    in { type = "app"; program = "${CHANGEME-wrap-my-package}/bin/CHANGEME-my-app"; };


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
    };

    # Activated by `nix develop <URL to this flake>#gcc`
    # devShells.gcc = pkgs.mkShell {
    #   name = "my-nain4-app-gcc-devenv";

    #   packages = dev-shell-packages;

    #   G4_DIR = "${pkgs.geant4}";
    #   G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
    #   QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    # };

    # 1. `nix build` .#singularity
    # 2. `scp result <me>@lxplus7.cern.ch:hello.img`
    # 3. [on lxplus] `singularity run hello.img`
    packages.singularity = pkgs.singularity-tools.buildImage {
      name = "test";
      contents = [ self.apps.CHANGEME-my-app.program ];
      runScript = "${self.apps.CHANGEME-my-app.program} $@";
      diskSize = 10240;
      memSize = 5120;
    };

  }
