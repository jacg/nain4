{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, ...
}: let

  pkgs = (nixpkgs.legacyPackages.extend (import ./overlays/argparse.nix));
  # Would prefer something along the lines of
  #    pkgs = import nixpkgs { overlays = [ (import ./overlays/argparse.nix) ]; };
  # but not sure how to get it to work with `nosys`

  my-geant4 = pkgs.geant4.overrideAttrs (oldAttrs: {
    dontStrip = true;
    cmakeBuildType = "Debug";
  });
in {
    packages.my-geant4 = my-geant4;

    packages.default = self.packages.my-geant4;

  }
