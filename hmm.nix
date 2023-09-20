{ pkgs
, stdenv
, an-argument ? "DEFAULT-arg"
, fetchurl
, ...
}:

stdenv.mkDerivation (finalAttrs: {
  name = "hmm";
  src = ".hmm.nix";
  dontUnpack = true;

  an-attribute = "DEFAULT-att";
  buildPhase = ''
     mkdir -p $out
     echo "argument : ${an-argument}"            >> $out/the-file
     echo  attribute: ${finalAttrs.an-attribute} >> $out/the-file
  '';
})
