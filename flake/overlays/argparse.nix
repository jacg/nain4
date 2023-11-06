final: prev: {
  argparse = prev.argparse.overrideAttrs (old: {
    src = prev.fetchFromGitHub {
      owner = "p-ranav";
      repo = "argparse";
      rev = "af442b4da0cd7a07b56fa709bd16571889dc7fda";
      hash = "sha256-0fgMy7Q9BiQ/C1tmhuNpQgad8yzaLYxh5f6Ps38f2mk=";
    };
  });
}
