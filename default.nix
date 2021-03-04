# Use flake-compat to re-use `flake.nix` to define legacy-Nix-compatible `default.nix` (or `shell.nix`).

# TODO doesn't seem to provide `cmake` in `nix-shell`. `nix develop` does provide `cmake`.

(import (
  fetchTarball {
    url = "https://github.com/edolstra/flake-compat/archive/99f1c2157fba4bfe6211a321fd0ee43199025dbf.tar.gz";
    sha256 = "0x2jn3vrawwv9xp15674wjz9pixwjyj3j771izayl962zziivbx2"; }
) {
  src =  ./.;
}).defaultNix # Use `defaultNix` in `default.nix`; change to `shellNix` for use in `shell.nixt`
