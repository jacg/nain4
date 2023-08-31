{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
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

    packages.geant4  = my-geant4;


    # Executed by `nix run <URL of this flake> -- <args?>`
    # TODO apps.default = { type = "app"; program = "..."; };

    # Executed by `nix run <URL of this flake>#my-app`
    # TODO apps.my-app = { type = "app"; program = "<store-path>"; };

    apps.bootstrap-client-project = {
      type    = "app";
      program = "${pkgs.writeShellScript "bootstrap.sh" ''
        echo 'Running bootstrap'




        #nix run github:jacg/nain4#bootstrap-client-project project-name author etc

        # nix flake new -t github:jacg/nain4 folder
        # cd folder
        # git init
        # git add .
        # git commit -m "Bootstrap project"
      ''}";
    };

    # Leading underscore prevents nosys from regenerating this for every system
    _templates = (import ../templates);

    _contains-systems = { systems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ]; };
  }
