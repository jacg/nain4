{

  description = "Playing with G4 examples";

  inputs = {
    nixpkgs    .url = "github:NixOS/nixpkgs/nixos-23.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in {

        devShell = pkgs.mkShell.override { stdenv = pkgs.clang_16.stdenv; } {
          name = "G4-examples-devenv";

          packages = with pkgs; [

            (geant4.override {
              enableMultiThreading = false;
              enableInventor       = false;
              enableQt             = true;
              enableXM             = false;
              enableOpenGLX11      = true;
              enablePython         = false;
              enableRaytracerX11   = false;
            })

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
            catch2
            just
            gnused # For hacking CMAKE_EXPORT stuff into CMakeLists.txt
            mdbook
          ];

          G4_DIR = "${pkgs.geant4}";
          G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
          QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

        };

      });
}
