{
  description = "flake.nix line 2: Change this to describe your project";

  inputs = {
    nain4  .url     = "github:jacg/nain4/templates";
    nosys  .follows = "nain4/nosys";
    nixpkgs.follows = "nain4/nixpkgs";
    systems.follows = "nain4/systems";
  };

  inputs.systems.flake = false;

  outputs = inputs @ {
    nosys,
    nixpkgs, # <---- This `nixpkgs` still has the `system` e.g. legacyPackages.${system}.zlib
    ...
  }: let outputs = import ./flake/outputs.nix;
     in nosys inputs outputs;
}
