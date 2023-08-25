{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, nain4
, ...
}: let
  inherit (nixpkgs.legacyPackages) pkgs;
  my-geant4 = (pkgs.geant4.override {
    enableMultiThreading = false;
    enableInventor       = false;
    enableQt             = true;
    enableXM             = false;
    enableOpenGLX11      = true;
    enablePython         = false;
    enableRaytracerX11   = false;
  });

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
             else pkgs.llvmPackages.clang;

  my-packages = with pkgs; [
    my-geant4
    geant4.data.G4PhotonEvaporation
    geant4.data.G4EMLOW
    geant4.data.G4RadioactiveDecay
    geant4.data.G4ENSDFSTATE
    geant4.data.G4SAIDDATA
    geant4.data.G4PARTICLEXS
    geant4.data.G4NDL
    cmake
    cmake-language-server
    catch2_3
    just
    gnused # For hacking CMAKE_EXPORT stuff into CMakeLists.txt
    mdbook
  ] ++ lib.optionals stdenv.isDarwin [

  ] ++ lib.optionals stdenv.isLinux [
  ];

  in rec {

    wtf = self.system;

    # TODO a package to run my nain4 project

    devShells.clang = pkgs.mkShell.override { stdenv = pkgs.clang_16.stdenv; } {
      name = "my-nain4-app-clang-devenv";

      packages = my-packages ++ [ nain4.packages.nain4 my-geant4 clang_16 pkgs.clang-tools ];

      G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    };

    # devShells.gcc = pkgs.mkShell {
    #   name = "my-nain4-app-gcc-devenv";

    #   packages = my-packages;

    #   G4_DIR = "${pkgs.geant4}";
    #   G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
    #   QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    # };

    devShell = devShells.clang;

    packages.geant4  = my-geant4;

  }
