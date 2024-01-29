{pkgs} :
{
  shell-shared = {
    G4_DIR = "${pkgs.geant4}";

    shellHook = ''
      export CHANGEME_SNAKE_LIB_PATH=$PWD/install/CHANGEME-PROJECT-NAME/lib
      export LD_LIBRARY_PATH=$CHANGEME_SNAKE_LIB_PATH:$LD_LIBRARY_PATH;
      export PKG_CONFIG_PATH=$CHANGEME_SNAKE_LIB_PATH/pkgconfig:$PKG_CONFIG_PATH;
    '';
  };
}
