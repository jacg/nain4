{ self
, nixpkgs # <---- This `nixpkgs` has systems removed e.g. legacyPackages.zlib
, oldpkgs
, ...
}: let

  pkgs-old = oldpkgs.legacyPackages.pkgs;
  pkgs = (nixpkgs.legacyPackages
    .extend (import ./overlays/argparse.nix))
    .extend add-debug-symbols-to-geant4;
  # Would prefer something along the lines of
  #    pkgs = import nixpkgs { overlays = [ (import ./overlays/argparse.nix) ]; };
  # but not sure how to get it to work with `nosys`

  add-debug-symbols-to-geant4 = final: previous: {
    geant4 = previous.geant4.overrideAttrs (old: {
      dontStrip = true;
      NIX_CFLAGS_COMPILE =
        (if builtins.hasAttr "NIX_CFLAGS_COMPILE" old then old.NIX_CFLAGS_COMPILE else "")
        + " -ggdb -Wa,--compress-debug-sections";
    });
  };

  g4 = { thread ? false , inventor ? false , qt ? false, xm ? false, ogl ? false, python ? false, raytrace ? false }:
    (pkgs-old.geant4.override {
      enableMultiThreading = thread;
      enableInventor       = inventor;
      enableQt             = qt;
      enableXM             = xm;
      enableOpenGLX11      = ogl;
      enablePython         = python;
      enableRaytracerX11   = raytrace;
    });

  my-geant4 = g4 { qt = true; };

  geant4-data = with pkgs-old.geant4.data; [
    G4PhotonEvaporation
    G4RealSurface
    G4EMLOW
    G4RadioactiveDecay
    G4ENSDFSTATE
    G4SAIDDATA
    G4PARTICLEXS
    G4NDL
  ];

  # Exporter of G4 envvars
  export-g4-env =
    let
      g4-data = self.deps.g4-data-package;
    in
      pkgs.writeShellScriptBin "export-g4-env" ''
        # TODO replace manual envvar setting with with use of packages' setupHooks
        export G4NEUTRONHPDATA="${g4-data.G4NDL}/share/Geant4-11.0.4/data/G4NDL4.6"
        export G4LEDATA="${g4-data.G4EMLOW}/share/Geant4-11.0.4/data/G4EMLOW8.0"
        export G4LEVELGAMMADATA="${g4-data.G4PhotonEvaporation}/share/Geant4-11.0.4/data/G4PhotonEvaporation5.7"
        export G4RADIOACTIVEDATA="${g4-data.G4RadioactiveDecay}/share/Geant4-11.0.4/data/G4RadioactiveDecay5.6"
        export G4PARTICLEXSDATA="${g4-data.G4PARTICLEXS}/share/Geant4-11.0.4/data/G4PARTICLEXS4.0"
        export G4PIIDATA="${g4-data.G4PII}/share/Geant4-11.0.4/data/G4PII1.3"
        export G4REALSURFACEDATA="${g4-data.G4RealSurface}/share/Geant4-11.0.4/data/G4RealSurface2.2"
        export G4SAIDXSDATA="${g4-data.G4SAIDDATA}/share/Geant4-11.0.4/data/G4SAIDDATA2.0"
        export G4ABLADATA="${g4-data.G4ABLA}/share/Geant4-11.0.4/data/G4ABLA3.1"
        export G4INCLDATA="${g4-data.G4INCL}/share/Geant4-11.0.4/data/G4INCL1.0"
        export G4ENSDFSTATEDATA="${g4-data.G4ENSDFSTATE}/share/Geant4-11.0.4/data/G4ENSDFSTATE2.3"
      '';

  # Utility for making Nix flake apps. A nix flake app allows "remote" execution of pre-packaged code.
  make-app = args:
    let
      g4-data = self.deps.g4-data-package;
      app-package = pkgs.writeShellScriptBin args.executable ''
        export PATH=${pkgs.lib.makeBinPath [ args.package ]}:$PATH
        # export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath [ self.packages.geant4 ] }:$LD_LIBRARY_PATH
        source "${export-g4-env}/bin/export-g4-env"
        exec ${args.executable} ${args.args}
      '';
    in { type = "app"; program = "${app-package}/bin/${args.executable}"; };

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
    G4_DIR = "${pkgs.geant4}";
      G4_EXAMPLES_DIR = "${pkgs.geant4}/share/Geant4-11.0.4/examples/";
      QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs-old.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs-old.libsForQt5.qt5.qtbase.version}/plugins";

      shellHook = ''
          export NAIN4_LIB=$PWD/install/nain4/lib
          export LD_LIBRARY_PATH=$NAIN4_LIB:$LD_LIBRARY_PATH;
          export PKG_CONFIG_PATH=$NAIN4_LIB/pkgconfig:$PKG_CONFIG_PATH;
      '';
    };

  in {

    inherit export-g4-env;

    packages.default = self.packages.nain4;

    # TODO: switch to clang environment
    packages.nain4 = pkgs.stdenv.mkDerivation {
      pname = "nain4";
      version = "0.2.0";
      src = "${self}/nain4/src";

      # https://nixos.org/guides/nix-pills/basic-dependencies-and-hooks#id1470
      # propagatedBuildInputs ... will be used as buildInputs by dependencies
                nativeBuildInputs = self.deps.build;      # local            build             environment
      propagatedNativeBuildInputs = self.deps.prop-build; # local and client build             environment # appears NOT to propagate!
                      buildInputs = self.deps.run;        # local            build and runtime environment
      propagatedBuildInputs       = self.deps.prop-run;   # local and client build and runtime environment

      hook_g4_dir = "${pkgs-old.geant4}";
      hook_g4_examples = "${pkgs-old.geant4}/share/Geant4-11.0.4/examples/";
      hook_qt_stuff = "${pkgs-old.libsForQt5.qt5.qtbase.bin}/lib/qt-${pkgs-old.libsForQt5.qt5.qtbase.version}/plugins";
      setupHook = ./nain4-hook.sh;

    };

    packages.nain4-tests = pkgs.stdenv.mkDerivation {
      pname = "nain4-tests";
      version = "0.2.0";
      src = "${self}/nain4/test";
      nativeBuildInputs = with self; [ packages.nain4 ] ++ deps.build ++ deps.test;
    };

    devShells.gcc = pkgs.mkShell                                          (shell-shared // {
      name = "nain4-gcc-devenv";
      packages = dev-shell-packages;
    });

    devShells.clang = pkgs.mkShell.override { stdenv = clang_16.stdenv; } (shell-shared // {
      name = "nain4-clang-devenv";
      packages = dev-shell-packages ++ [ clang_16 ];
    });

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
        BASE_NAME=$2
        DESCRIPTION=$3
        if [[ -z $DESCRIPTION ]];
        then
          echo Missing argument to bootstrap-client-project
          echo
          echo Usage:
          echo
          echo "REQUIRED  first argument: directory where new project will be placed                   provided: $DIRECTORY"
          echo "REQUIRED second argument: project identifier \(may not contain spaces, slashes, etc.\) provided: $BASE_NAME"
          echo "REQUIRED  third argument: IN QUOTES single-line description of project"
          exit 1
        fi
        mkdir -p $DIRECTORY/scripts
        FQ_DIRECTORY=$(${pkgs.coreutils}/bin/readlink -f $DIRECTORY)
        ${pkgs.coreutils}/bin/cp -Tr ${self}/templates/basic                                    $FQ_DIRECTORY
        ${pkgs.coreutils}/bin/cp     ${self}/nain4/test/run-each-test-in-separate-process.sh.in $FQ_DIRECTORY
        ${pkgs.coreutils}/bin/cp     ${self}/scripts/count-warnings.sh                          $FQ_DIRECTORY/scripts
        chmod -R u+w $FQ_DIRECTORY
        nix develop  $FQ_DIRECTORY -c true # create flake.lock
        cd           $FQ_DIRECTORY
        ${pkgs.ripgrep}/bin/rg "ANCHOR" --files-with-matches . | ${pkgs.findutils}/bin/xargs ${pkgs.gnused}/bin/sed -i '/ANCHOR/d'

        REPLACE () {
          OLD=$1
          NEW=$2
          ${pkgs.ripgrep}/bin/rg --hidden "$OLD" --files-with-matches . | ${pkgs.findutils}/bin/xargs ${pkgs.gnused}/bin/sed -i "s|$OLD|$NEW|g"
        }
        REPLACE "CHANGEME-EXE"                           ''${BASE_NAME}
        REPLACE "CHANGEME-PROJECT-NAME"                  ''${BASE_NAME}
        REPLACE "CHANGEME-TESTS-PROJECT-NAME"            ''${BASE_NAME}-tests
        REPLACE "CHANGEME-PROJECT-TEST-EXE"              ''${BASE_NAME}-test
        REPLACE "CHANGEME-PACKAGE"                       ''${BASE_NAME}
        REPLACE "CHANGEME-ONE-LINE-PROJECT-DESCRIPTION" "''${DESCRIPTION}"

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
    _contains-systems = { systems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ]; };

    deps = {
      inherit make-app;
      args-from-cli = ''"$@"'';
      dev-shell-packages = client-dev-shell-packages;
      g4-data-package = pkgs-old.geant4.data; # Needed for exporting G4*DATA envvars in client app
      # TODO: leave for now, in case client needs it, but make these purely
      # internal once we're happy that everything works
      # The prop-* variants are to be propagated to downstream packages (either by Nix (build, run) or by us (dev, test))

      # Availability of packages is indicated in comments below with the following 2-letter codes:
      # ND: nain4  devshell,    NB: nain4  build time,     NR: nani4  run time
      # CD: client devshell,    CB: client build time,     CR: client run time

      #            ND                NB        NB NR
      dev = [ pkgs.mdbook ]; build = [ ]; run = [ ];
      # ND CD
      prop-dev   = with pkgs; self.deps.prop-build ++ [ just clang-tools catch2_3 ];
      # NB CB
      prop-build = with pkgs; [ meson ninja cmake pkg-config argparse boost182 ];
      # NB CB NR CR  +  ND CD, via addition to prop-dev
      prop-run   = with pkgs; [ just geant4-data my-geant4 pkgs-old.qt5.wrapQtAppsHook ];
      # How about things needed at run but not build time?
    };

  }
