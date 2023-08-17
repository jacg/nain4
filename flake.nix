{

  description = "Playing with G4 examples";

  inputs = {
    nixpkgs         .url = "github:NixOS/nixpkgs/nixos-23.05";
    flake-utils     .url = "github:numtide/flake-utils";
    flake-compat = { url = "github:edolstra/flake-compat"; flake = false; };
  };

  outputs = { self, nixpkgs, flake-utils, flake-compat }:
    flake-utils.lib.eachSystem ["x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin"] (system:
      let
        pkgs = import nixpkgs { inherit system; };

        my-geant4 = (pkgs.geant4.override {
          enableMultiThreading = false;
          enableInventor       = false;
          enableQt             = true;
          enableXM             = false;
          enableOpenGLX11      = true;
          enablePython         = false;
          enableRaytracerX11   = false;
        });

        my-packages = with pkgs; [
          my-geant4
          geant4.data.G4PhotonEvaporation
          geant4.data.G4EMLOW
          geant4.data.G4RadioactiveDecay
          geant4.data.G4ENSDFSTATE
          geant4.data.G4SAIDDATA
          geant4.data.G4PARTICLEXS
          geant4.data.G4NDL
          cmake
          cmake-language-server
          catch2_3
          just
          gnused # For hacking CMAKE_EXPORT stuff into CMakeLists.txt
          mdbook
        ] ++ lib.optionals stdenv.isDarwin [

        ] ++ lib.optionals stdenv.isLinux [
          clang_16
          clang-tools
        ];

        my-mkShell = pkgs.mkShell.override {
          stdenv = if pkgs.stdenv.isDarwin then pkgs.                stdenv
                                           else pkgs.llvmPackages_16.stdenv;
        };

      in {

        devShell = my-mkShell {
          name = "G4-examples-devenv";

          packages = my-packages;

          G4_DIR = "${pkgs.geant4}";
          G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
          QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

        };

        packages.geant4  = my-geant4;
        packages.default = my-geant4;

      });
}
