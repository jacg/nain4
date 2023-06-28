{

  description = "Playing with G4 examples";

  inputs.nixpkgs    .url = "github:NixOS/nixpkgs/nixos-23.05";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in {

        devShell = pkgs.mkShell.override { stdenv = pkgs.clang_16.stdenv; } {
          name = "G4-examples-devenv";

          packages = with pkgs; [
            #(geant4.override { enableQt = true; })
            geant4
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
            just
          ];

          G4_DIR = "${pkgs.geant4}";
          G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
        };

      });
}
