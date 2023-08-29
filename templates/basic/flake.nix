{
  #CHANGEME-flake-description: keep it short (1 line).
  description = "flake.nix line 2: Change this to describe your project";

  inputs = {
    nain4  .url     = "github:jacg/nain4";
    nosys  .follows = "nain4/nosys";
    nixpkgs.follows = "nain4/nixpkgs";
  };

  outputs = inputs @ {
    nosys,
    nixpkgs, # <---- This `nixpkgs` still has the `system` e.g. legacyPackages.${system}.zlib
    ...
  }: let outputs = import ./flake/outputs.nix;
         systems = inputs.nain4.contains-systems.systems;
    in nosys (inputs // { inherit systems; }) outputs;

}
