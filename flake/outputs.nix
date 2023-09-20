{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, ...
}: let

  pkgs = (nixpkgs.legacyPackages.extend add-debug-symbols-to-geant4);
  # Would prefer something along the lines of
  #    pkgs = import nixpkgs { overlays = [ (import ./overlays/argparse.nix) ]; };
  # but not sure how to get it to work with `nosys`

  add-debug-symbols-to-geant4 = final: previous: {
    geant4 = previous.geant4.overrideAttrs (old: {
      dontStrip = true;
      cmakeBuildType = "RelWithDebInfo";
    });
  };

  g4 = { thread ? false , inventor ? false , qt ? false, xm ? false, ogl ? false, python ? false, raytrace ? false }:
    pkgs.geant4.override {
      enableMultiThreading = thread;
      enableInventor       = inventor;
      enableQt             = qt;
      enableXM             = xm;
      enableOpenGLX11      = ogl;
      enablePython         = python;
      enableRaytracerX11   = raytrace;
    };

  hmm = pkgs.callPackage ../hmm.nix {};
  hmm-o  = hmm  .override          { an-argument  = "OVERRIDDEN-arg"; };
  hmm-a  = hmm  .overrideAttrs (_: { an-attribute = "OVERRIDDEN-att"; });
  hmm-oa = hmm-o.overrideAttrs (_: { an-attribute = "OVERRIDDEN-att"; });
  hmm-ao = hmm-a.override          { an-argument  = "OVERRIDDEN-arg"; };
in {
    packages.default = self.packages.my-geant4;
    packages.geant4-with-debug-symbols = pkgs.geant4;
    packages.my-geant4 = g4 { qt = true; ogl = true; };
    packages.hmm    = hmm;
    packages.hmm-o  = hmm-o;
    packages.hmm-a  = hmm-a;
    packages.hmm-oa = hmm-oa;
    packages.hmm-ao = hmm-ao;
}
