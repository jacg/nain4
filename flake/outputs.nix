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
             else pkgs.llvmPackages_16.clang;

  dev-shell-packages = with self.deps;
    dev   ++   dev-prop ++
    build ++ build-prop ++
    test  ++  test-prop ++
    run   ++   run-prop ++
    pkgs.lib.optionals pkgs.stdenv.isDarwin [] ++
    pkgs.lib.optionals pkgs.stdenv.isLinux  []
  ;

  in {

    packages.default = self.packages.nain4;

    # TODO: switch to clang environment
    packages.nain4 = pkgs.stdenv.mkDerivation {
      pname = "nain4";
      version = "0.2.0";
      src = "${self}/nain4/src";

                nativeBuildInputs = self.deps.build;      # local            build             environment
      propagatedNativeBuildInputs = self.deps.build-prop; # local and client build             environment # appears NOT to propagate!
                      buildInputs = self.deps.run;        # local            build and runtime environment
      propagatedBuildInputs       = self.deps.run-prop;   # local and client build and runtime environment

      hook_g4_dir = "${pkgs.geant4}";
      hook_g4_examples = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      hook_qt_stuff = "${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";
      setupHook = ./nain4-hook.sh;

      # https://nixos.org/guides/nix-pills/basic-dependencies-and-hooks#id1470
      # TODO propagatedBuildInputs ... will be used as buildInputs by dependencies

      # meta = with pkgs.lib; {
      #   description = "An API that makes it easier to write Geant4 application code.";
      #   homepage = "https://jacg.github.io/nain4/";
      #   # license = licenses.TODO;
      #   platforms = platforms.unix;
      # };
    };

    packages.nain4-tests = pkgs.stdenv.mkDerivation {
      pname = "nain4-tests";
      version = "0.2.0";
      src = "${self}/nain4/test";
      nativeBuildInputs = with self; [ packages.nain4 ] ++ deps.build ++ deps.test;
    };

    devShells.clang = pkgs.mkShell.override { stdenv = clang_16.stdenv; } {
      name = "nain4-clang-devenv";

      packages = dev-shell-packages ++ [ clang_16 ];

      G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

      shellHook = ''
          export NAIN4_LIB=$PWD/install/nain4/lib/
          export LD_LIBRARY_PATH=$NAIN4_LIB:$LD_LIBRARY_PATH;
          export PKG_CONFIG_PATH=$NAIN4_LIB/pkgconfig:$PKG_CONFIG_PATH;
      '';
    };

    devShells.gcc = pkgs.mkShell {
      name = "nain4-gcc-devenv";

      packages = dev-shell-packages;

      G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs.libsForQt5.qt5.qtbase.version}/plugins";

      shellHook = ''
          export NAIN4_LIB=$PWD/install/nain4/lib
          export LD_LIBRARY_PATH=$NAIN4_LIB:$LD_LIBRARY_PATH;
          export PKG_CONFIG_PATH=$NAIN4_LIB/pkgconfig:$PKG_CONFIG_PATH;
      '';
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

    # 1. `nix build .#docker`
    # 2. `docker load < result`
    # 3. `docker run -it --rm cowsay-and-lolcat:0.1.0`
    # 4. cowsay hello | lolcat
    # 5. CTRL-d
    packages.docker = pkgs.dockerTools.buildLayeredImage {
      name = "cowsay-and-lolcat";
      tag = "0.1.0";
      contents = [ pkgs.bash pkgs.cowsay pkgs.lolcat ];
      config = { Cmd = [ "${pkgs.bash}/bin/bash" ]; };
      created = "now";
    };

    # Leading underscore prevents nosys from regenerating this for every system
    _templates = (import ../templates);

    _contains-systems = { systems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ]; };

    deps = {
      # The -prop variants are to be propagated to downstream packages (either by Nix (build, run) or by us (dev, test))
      dev        = with pkgs; [ mdbook ];   dev-prop = with pkgs; [ just clang-tools ] ++ self.deps.build-prop;
      build      = with pkgs; [ ];        build-prop = with pkgs; [ meson ninja cmake pkg-config argparse ];
      test       = with pkgs; [ ];         test-prop = with pkgs; [ catch2_3 ];
      run        = with pkgs; [ ];          run-prop = with pkgs; [ just geant4-data my-geant4 qt5.wrapQtAppsHook ];
      g4-data-package = pkgs.geant4.data; # Needed for exporting G4*DATA envvars in client app
    };

  }
