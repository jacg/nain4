{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, ...
}: let
  inherit (nixpkgs.legacyPackages) pkgs;

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

  geant4-data = with pkgs.geant4.data; [
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
             else pkgs.llvmPackages.clang;

  dev-deps = with pkgs; [
    just
    cmake-language-server
    mdbook
    gnused # For hacking CMAKE_EXPORT stuff into CMakeLists.txt
  ];

  build-deps = with pkgs; [ clang-tools cmake my-geant4 qt5.wrapQtAppsHook argparse ];
  test-deps  = with pkgs; [ catch2_3 ];
  run-deps   = with pkgs; [ just geant4-data ];
  dev-shell-packages = dev-deps ++ build-deps ++ test-deps ++ run-deps
                       ++ pkgs.lib.optionals pkgs.stdenv.isDarwin []
                       ++ pkgs.lib.optionals pkgs.stdenv.isLinux  []
  ;

  in {

    packages.default = self.packages.nain4;

    # TODO: switch to clang environment
    packages.nain4 = pkgs.stdenv.mkDerivation {
      pname = "nain4";
      version = "0.1.10";
      src = "${self}/nain4/src";
      nativeBuildInputs = build-deps; # extra-cmake-modules ?

      # meta = with pkgs.lib; {
      #   description = "An API that makes it easier to write Geant4 application code.";
      #   homepage = "https://jacg.github.io/nain4/";
      #   # license = licenses.TODO;
      #   platforms = platforms.unix;
      # };
    };

    packages.nain4-tests = pkgs.stdenv.mkDerivation {
      pname = "nain4-tests";
      version = "0.1.10";
      src = "${self}/nain4/test";
      nativeBuildInputs = [ self.packages.nain4 ] ++ build-deps ++ test-deps;
    };

    devShells.clang = pkgs.mkShell.override { stdenv = pkgs.clang_16.stdenv; } {
      name = "nain4-clang-devenv";

      packages = dev-shell-packages ++ [ clang_16 ];

      G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    };

    devShells.gcc = pkgs.mkShell {
      name = "nain4-gcc-devenv";

      packages = dev-shell-packages;

      G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    };

    devShell = self.devShells.clang;

    packages.geant4 = my-geant4;
    packages.clang_16 = clang_16;


    # Executed by `nix run <URL of this flake> -- <args?>`
    # TODO apps.default = { type = "app"; program = "..."; };

    # Executed by `nix run <URL of this flake>#my-app`
    # TODO apps.my-app = { type = "app"; program = "<store-path>"; };

    apps.bootstrap-client-project = {
      type    = "app";
      # nix run github:jacg/nain4#bootstrap-client-project project-name author etc
      program = "${pkgs.writeShellScript "bootstrap.sh" ''
        DIRECTORY=$1
        mkdir -p $DIRECTORY
        ${pkgs.coreutils}/bin/cp -Tr ${self}/templates/basic $DIRECTORY
        chmod -R u+w $DIRECTORY
        nix develop $DIRECTORY -c true # create flake.lock
        cd $DIRECTORY
        git -c init.defaultBranch=master init -q
        # TODO: protect against user not having set git user.{name,email}
        git add .
        git commit -qm "Bootstrap project"
        echo
        echo You have created a simple nain4 application which you can use as the basis of your own nain4 project.
        echo
        echo See https://jacg.github.io/nain4/how-to/start-a-nain4-based-project.html#basic-usage for what to do next.
      ''}";
    };

    # Leading underscore prevents nosys from regenerating this for every system
    _templates = (import ../templates);

    _contains-systems = { systems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ]; };

    deps = {
      inherit dev-deps build-deps test-deps run-deps;
      g4-data-package = pkgs.geant4.data;
    };

  }
