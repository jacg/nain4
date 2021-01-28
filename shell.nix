let
  name = "nixos-unstable-2021-01-28";
  commit-id = "4c9a74aa459dc525fcfdfb3019b234f68de66c8a";
  url = "https://github.com/nixos/nixpkgs/archive/${commit-id}.tar.gz";
  pkgs = import (builtins.fetchTarball { url = url; }) {};

  derivation-fn = { geant4, stdenv }:
    stdenv.mkDerivation {
      name = "g4-example-B1";
      src = ./.;

      # build-time dependencies
      nativeBuildInputs = [
        geant4
        geant4.data.G4PhotonEvaporation
        geant4.data.G4EMLOW
        geant4.data.G4RadioactiveDecay
        geant4.data.G4ENSDFSTATE
        geant4.data.G4SAIDDATA
        geant4.data.G4PARTICLEXS
        geant4.data.G4NDL
      ];

      buildInputs = [ ];

    };

  derivation = pkgs.callPackage derivation-fn {};
in

pkgs.llvmPackages_11.stdenv.mkDerivation {
  inherit (derivation) name nativeBuildInputs;

  buildInputs = derivation.buildInputs ++ [
    pkgs.clang-tools
    pkgs.clang_11
    pkgs.bear
    pkgs.cmake
  ];

}
