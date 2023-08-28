rec {
  basic = {
    path = ./basic;
    description = "Basic nain4 application";
    welcomeText = ''
          You just created a simple nain4 application which you can use as the basis of your own nain4 project

          + `just`  - does something (probably runs the tests)
          + `just run` - does something else (probably runs the GUI)
          + `just run 100` - does something even else (probably runs beamOn 100)

          To turn this into your own app

          + `git init`
          + `grep CHANGEME` and follow instructions in the lines that appear
        '';
  };

  default = basic;
}
