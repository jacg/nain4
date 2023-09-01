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

  my-packages = with pkgs; [
    my-geant4 ] ++ geant4-data ++ [
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

    packages.default = self.packages.nain4;

    # TODO: switch to clang environment
    packages.nain4 = pkgs.stdenv.mkDerivation {
      pname = "nain4";
      version = "0.1.9";
      src = "${self}/nain4/src";
      nativeBuildInputs = with pkgs; [ cmake my-geant4 qt5.wrapQtAppsHook ]; # extra-cmake-modules ?

      # meta = with pkgs.lib; {
      #   description = "An API that makes it easier to write Geant4 application code.";
      #   homepage = "https://jacg.github.io/nain4/";
      #   # license = licenses.TODO;
      #   platforms = platforms.unix;
      # };
    };

    packages.nain4-tests = pkgs.stdenv.mkDerivation {
      pname = "nain4-tests";
      version = "0.1.9";
      src = "${self}/nain4/test";
      nativeBuildInputs = with pkgs; [
        self.packages.nain4
        cmake
        my-geant4
        catch2_3
        qt5.wrapQtAppsHook ]; # extra-cmake-modules ?
    };

    devShells.clang = pkgs.mkShell.override { stdenv = pkgs.clang_16.stdenv; } {
      name = "nain4-clang-devenv";

      packages = my-packages ++ [ clang_16 pkgs.clang-tools ];

      G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    };

    devShells.gcc = pkgs.mkShell {
      name = "nain4-gcc-devenv";

      packages = my-packages;

      G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

    };

    devShell = devShells.clang;

    packages.geant4 = my-geant4;


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
  }
