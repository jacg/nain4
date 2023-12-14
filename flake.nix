{
  description = "An abstraction layer on top of Geant4";

  inputs = {
    nixpkgs         .url = "github:NixOS/nixpkgs/nixos-23.11";
    oldpkgs         .url = "github:NixOS/nixpkgs/nixos-23.05";
    flake-compat = { url = "github:edolstra/flake-compat"; flake = false; };
    nosys           .url = "github:divnix/nosys";
  };

  outputs = inputs @ {
    nosys,
    nixpkgs, # <---- This `nixpkgs` still has the `system` e.g. legacyPackages.${system}.zlib
    oldpkgs,
    ...
  }:
    let
      outputs = import ./flake/outputs.nix;
      system = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];
    in nosys (inputs // { inherit system; }) outputs;
}
