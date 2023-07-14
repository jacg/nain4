{

  description = "Playing with G4 examples";

  inputs = {
    nixpkgs         .url = "github:NixOS/nixpkgs/nixos-23.05";
    flake-utils     .url = "github:numtide/flake-utils";
    flake-compat = { url = "github:edolstra/flake-compat"; flake = false; };
  };

  outputs = { self, nixpkgs, flake-utils, flake-compat }:
    let
      geant4-dbg-overlay = final: prev: {
        geant4-dbg = prev.geant4.overrideAttrs (old: {
          cmakeBuildType = "Debug";
        });
      };
      geant4-config = {
        enableMultiThreading = false;
        enableInventor       = false;
        enableQt             = true;
        enableXM             = false;
        enableOpenGLX11      = true;
        enablePython         = false;
        enableRaytracerX11   = false;
      };
    in
    flake-utils.lib.eachSystem ["x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin"] (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ geant4-dbg-overlay ];
        };
        my-geant4-dbg = (pkgs.geant4-dbg.override geant4-config);
        my-geant4     = (pkgs.geant4    .override geant4-config);

      in {

        devShell = pkgs.mkShell.override { stdenv = pkgs.clang_16.stdenv; } {
          name = "G4-examples-devenv";

          packages = with pkgs; [
            my-geant4-dbg
            geant4.data.G4PhotonEvaporation
            geant4.data.G4EMLOW
            geant4.data.G4RadioactiveDecay
            geant4.data.G4ENSDFSTATE
            geant4.data.G4SAIDDATA
            geant4.data.G4PARTICLEXS
            geant4.data.G4NDL
            clang_16
            clang-tools
            cmake
            cmake-language-server
            catch2_3
            just
            gnused # For hacking CMAKE_EXPORT stuff into CMakeLists.txt
            mdbook
            gdb
          ];

          G4_DIR = "${pkgs.geant4}";
          G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
          QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

        };

        packages.geant4  = my-geant4;
        packages.default = my-geant4;

      });
}
