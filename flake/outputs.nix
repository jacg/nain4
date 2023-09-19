{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, ...
}: let

  pkgs = (nixpkgs.legacyPackages.extend (import ./overlays/argparse.nix));
  # Would prefer something along the lines of
  #    pkgs = import nixpkgs { overlays = [ (import ./overlays/argparse.nix) ]; };
  # but not sure how to get it to work with `nosys`

  g4 = { thread ? false , inventor ? false , qt ? false, xm ? false, ogl ? false, python ? false, raytrace ? false }:
    (pkgs.geant4.override {
      enableMultiThreading = thread;
      enableInventor       = inventor;
      enableQt             = qt;
      enableXM             = xm;
      enableOpenGLX11      = ogl;
      enablePython         = python;
      enableRaytracerX11   = raytrace;
    });

  my-geant4 = g4 { qt = true; ogl = true ; };

  geant4-data = with my-geant4.data; [
    G4PhotonEvaporation
    G4EMLOW
    G4RadioactiveDecay
    G4ENSDFSTATE
    G4SAIDDATA
    G4PARTICLEXS
    G4NDL
  ];

  # Should be able to remove this, once https://github.com/NixOS/nixpkgs/issues/234710 is merged
  clang_16 = if pkgs.stdenv.isDarwin
             then pkgs.llvmPackages_16.clang.override rec {
               libc = pkgs.darwin.Libsystem;
               bintools = pkgs.bintools.override { inherit libc; };
               inherit (pkgs.llvmPackages) libcxx;
               extraPackages = [
                 pkgs.llvmPackages.libcxxabi
                 # Use the compiler-rt associated with clang, but use the libc++abi from the stdenv
                 # to avoid linking against two different versions (for the same reasons as above).
                 (pkgs.llvmPackages_16.compiler-rt.override {
                   inherit (pkgs.llvmPackages) libcxxabi;
                 })
               ];
             }
             else pkgs.llvmPackages_16.clang;

  dev-shell-packages = with self.deps;
    dev   ++ prop-dev   ++
    build ++ prop-build ++
    run   ++ prop-run   ++
    pkgs.lib.optionals pkgs.stdenv.isDarwin [] ++
    pkgs.lib.optionals pkgs.stdenv.isLinux  []
  ;

  client-dev-shell-packages =
    [ self.packages.nain4 ] ++ self.deps.prop-dev ++
    pkgs.lib.optionals pkgs.stdenv.isDarwin []    ++
    pkgs.lib.optionals pkgs.stdenv.isLinux  []
  ;

  shell-shared = {
    G4_DIR = "${my-geant4}";
      G4_EXAMPLES_DIR = "${my-geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

      shellHook = ''
          export NAIN4_LIB=$PWD/install/nain4/lib
          export LD_LIBRARY_PATH=$NAIN4_LIB:$LD_LIBRARY_PATH;
          export PKG_CONFIG_PATH=$NAIN4_LIB/pkgconfig:$PKG_CONFIG_PATH;
      '';
    };

  in {

    packages.my-geant4 = my-geant4;

    packages.default = self.packages.my-geant4;

  }
