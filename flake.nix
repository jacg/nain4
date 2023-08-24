{
  description = "Playing with G4 examples";

  inputs = {
    nixpkgs         .url = "github:NixOS/nixpkgs/nixos-23.05";
    flake-compat = { url = "github:edolstra/flake-compat"; flake = false; };
    nosys           .url = "github:divnix/nosys";
  };

  inputs.systems.url = "path:./flake/systems.nix";
  inputs.systems.flake = false;

  outputs = inputs @ {
    nosys,
    nixpkgs, # <---- This `nixpkgs` still has the `system` e.g. legacyPackages.${system}.zlib
    ...
  }: let outputs = import ./flake/outputs.nix;
     in nosys inputs outputs;
}
