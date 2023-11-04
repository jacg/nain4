final: prev: {
  argparse = prev.argparse.overrideAttrs (old: {
    src = prev.fetchFromGitHub {
      owner = "p-ranav";
      repo = "argparse";
      rev = "62052fefcb552e138a9d9e2807e883edcb09569a";
      hash = "sha256-eiq1yWYLBu8CdHZR0awKF4LcKHC73K/Nvg7sXiJ9g68=";
    };
    patches = [../patches/argparse-size_t.patch];
  });
}
