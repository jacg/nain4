rec {
  basic = {
    path = ./basic;
    description = "Basic nain4 application";
    welcomeText = ''
You have created a simple nain4 application which you can use as the basis of your own nain4 project

`cd` into the directory of the project and type one of

+ `direnv allow` (if `direnv` is installed and configured on your machine)
+ `nix develop` (if you do not have `direnv`)

Some downloading and compilation may take place. When this is complete you can try

+ `just` - runs the application in batch mode with the provided `macs/run.mac` Geant4 macro file.

+ `just run` - runs the application in interactive mode.

   If you are on NixOS or MacOS, graphics should work out of the box. On non-NixOS linuxes and WSL, you will probably have to use the `nixGL` helper, like this:

   ```sh
   nix run --impure github:guibou/nixGL -- just run
   ```

   The first time you use `nixGL` it will probably take quite some time to compile.

+ `just run 100` - runs the application in batch mode with `/run/beamOn 100`



To evolve this into your own application

1. `git init && git add . && git commit -m "Bootstrap project from template"`
2. `grep CHANGEME` will give you hints about places where you will probably want to change names, descriptions and metadata to something more appropriate for your project.
3. Edit the code to suit your needs.
'';
  };

  default = basic;
}
