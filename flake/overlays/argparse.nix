final: prev: {
  argparse = prev.argparse.overrideAttrs (old: {
    src = prev.fetchFromGitHub {
      owner = "p-ranav";
      repo = "argparse";
      rev = "d28188f4d542b07de9bad3e89aa53d8bf1a53a09";
      hash = "sha256-kIR59nva1QIpMjZ3ATc6rXgqNd2p8WKLzug7cqyEUiU=";
    };
  });
}
