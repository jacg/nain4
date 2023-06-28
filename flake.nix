{

  description = "Playing with G4 examples";

  inputs.nixpkgs    .url = "github:NixOS/nixpkgs/nixos-23.05";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in rec {

        packages.B1 = pkgs.stdenv.mkDerivation {
          name = "geant4-example-B1";
          src = self;

          nativeBuildInputs = with pkgs; [
            geant4
            geant4.data.G4PhotonEvaporation
            geant4.data.G4EMLOW
            geant4.data.G4RadioactiveDecay
            geant4.data.G4ENSDFSTATE
            geant4.data.G4SAIDDATA
            geant4.data.G4PARTICLEXS
            geant4.data.G4NDL
            clang_16
          ];

          buildInputs = [ ];

          buildPhase = ''
            mkdir -p example-B1/build
            cd example-B1/build
            G4DIR=${pkgs.geant4};
            ${pkgs.cmake}/bin/cmake .. # Why can we get away without setting -DGeant4_DIR
            make -j $NIX_BUILD_CORES
          '';

          installPhase = ''
            mkdir -p $out/bin    && install        -t $out/bin     exampleB1
            mkdir -p $out/macros && install -m 444 -t $out/macros *.mac
          '';

        };

        devShell = pkgs.llvmPackages_16.stdenv.mkDerivation {
          inherit (packages.B1) name nativeBuildInputs;

          buildInputs = packages.B1.nativeBuildInputs ++ [
            pkgs.clang-tools
            pkgs.clang_16
            pkgs.bear
            pkgs.cmake
          ];

          B1_MACRO_DIR = "${packages.B1}/macros";
          G4_DIR = "${pkgs.geant4}";

        };

        defaultPackage = packages.B1;

        defaultApp = {
          type = "app";
          program = "${packages.B1}/bin/exampleB1";
        };

      });
}
