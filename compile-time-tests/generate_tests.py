import os
import sys
import shutil
import subprocess
import tempfile

# TEST_NAME : (ERROR_MATCH, SNIPPET)
tests = {
    "Run manager without anything"
    :
    ( "no member named 'initialize'"
    , """
n4::run_manager::create()
   .ui("program-name", argc, argv, false)
   .initialize();
""")
,

    "Run manager without physics"
    :
    ("no member named 'geometry'"
    ,"""
n4::run_manager::create()
   .ui("program-name", argc, argv, false)
   .geometry  (dummy_geometry)
   .actions   (dummy_actions)
   .initialize();
""")
,

    "Run manager without physics 2"
    :
    ( "no member named 'actions'"
    , """
n4::run_manager::create()
   .ui("program-name", argc, argv, false)
   .actions   (dummy_actions)
   .geometry  (dummy_geometry)
   .initialize();
""")
,

    "Run manager without geometry"
    :
    ( "no member named 'initialize'"
    , """
n4::run_manager::create()
   .ui("program-name", argc, argv, false)
   .physics(dummy_physics_list)
   .initialize();
""")
,

    "Run manager without geometry 2"
    :
    ( "no member named 'actions'"
    , """
n4::run_manager::create()
   .ui("program-name", argc, argv, false)
   .physics(dummy_physics_list)
   .actions(dummy_actions)
   .initialize();
""")
,

    "Run manager wrong order 1"
    :
    ( "no member named 'geometry'"
    , """
n4::run_manager::create()
   .ui("program-name", argc, argv, false)
   .geometry  (dummy_geometry)
   .physics   (dummy_physics_list)
   .actions   (dummy_actions)
   .initialize();
""")
,


    "Run manager wrong order 2"
    :
    ( "no member named 'actions'"
    , """
n4::run_manager::create()
   .ui("program-name", argc, argv, false)
   .actions   (dummy_actions)
   .physics   (dummy_physics_list)
   .geometry  (dummy_geometry)
   .initialize();
""")
,


    "Run manager wrong order 3"
    :
    ( "no member named 'actions'"
    , """
n4::run_manager::create()
   .ui("program-name", argc, argv, false)
   .physics   (dummy_physics_list)
   .actions   (dummy_actions)
   .geometry  (dummy_geometry)
   .initialize();
""")
,

}


test_template = """
#include <n4-all.hh>
#include "compile_time_common.hh"

int main(int argc, char** argv) {{

{snippet}

}}
"""

def found(needle, haystack):
    return needle in sanitized(haystack)

def sanitized(text):
    return (text
            .replace('’', "'")
            .replace('‘', "'")
            )


number_of_tests      = len(tests)
max_test_name_length = max(map(len, tests))

main_folder    = tempfile.mkdtemp()
nain4          = os.environ["NAIN4_INSTALL"]
n_failed       = 0
green          = "\033[92m"
red            = "\033[91m"
reset_colour   = "\033[0m"
for (test_name, (error_match, snippet)) in tests.items():
    test_folder_name = test_name.replace(" ", "_")
    test_folder      = os.path.join(main_folder, test_folder_name)
    filename         = os.path.join(test_folder, "main.cc")

    os.mkdir(test_folder)
    shutil.copy("compile_time_common.hh", test_folder)
    shutil.copy("CMakeLists.txt"        , test_folder)

    open(filename, "w").write(test_template.format(snippet=snippet))
    command = "NAIN4_INSTALL={1} cmake -S {0} -B {0} && cmake --build {0}"
    process = subprocess.run( command.format(test_folder, nain4)
                            , capture_output = True
                            , shell          = True)

    compiled    = process.returncode == 0
    found_match = found(error_match, process.stderr.decode())
    failed      = compiled or not found_match
    n_failed   += int(failed)

    colour  = red if failed else green
    message = (
        f"FAILED: compiler error does not contain expected message ({error_match})" if not found_match else
         "SUCCEEDED in generating expected compile-time error"                      if not compiled    else
         "FAILED to generate a compile-time error"
              )
    print(colour, f"{test_name:<{max_test_name_length}}", message, reset_colour)

    if failed:
        print( "#" * 80 + "\n"
             + " " * 35 + "FULL OUTPUT\n"
             + "-" * 80 + "\n"
             + process.stdout.decode()
             + "-" * 80 + "\n"
             + process.stderr.decode()
             + "#" * 80 + "\n"
             )
colour  =    red   if n_failed else green
summary = "FAILED" if n_failed else "PASSED"
print( colour
     , "=" * 40 +  "\n "
     + "=" * 11 + f" SUMMARY : {summary} " + "=" * 11 + "\n "
     + "=" * 11 + f" PASSED  : {number_of_tests - n_failed:>6} " + "=" * 11 + "\n "
     + "=" * 11 + f" FAILED  : {n_failed:>6} " + "=" * 11 + "\n "
     + "=" * 40
     , reset_colour)

sys.exit(int(n_failed > 0))
