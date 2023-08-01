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
   .initialize();
""")
,

    "Run manager without physics"
    :
    ("no member named 'geometry'"
    ,"""
n4::run_manager::create()
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
   .physics(dummy_physics_list)
   .initialize();
""")
,

    "Run manager without geometry 2"
    :
    ( "no member named 'actions'"
    , """
n4::run_manager::create()
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
   .physics   (dummy_physics_list)
   .actions   (dummy_actions)
   .geometry  (dummy_geometry)
   .initialize();
""")
,

}


test_template = """
#include "nain4.hh"
#include "compile_time_common.hh"

int main() {{

{snippet}

}}
"""

main_folder    = tempfile.mkdtemp()
nain4          = os.path.join(os.environ["PWD"], "..", "install")
all_failed     = True
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

    failed      = process.returncode != 0 and error_match in process.stderr.decode()
    all_failed &= failed

    colour  = green if failed else red
    message = "FAILED COMPILING as expected" if failed else "COMPILED SUCCESFULLY (and it shouldn't)"
    print(colour, "\b\b", test_name, message, reset_colour)

    if not failed:
        print( "#" * 80 + "\n"
             + " " * 35 + "FULL OUTPUT\n"
             + "-" * 80 + "\n"
             + process.stdout.decode()
             + "-" * 80 + "\n"
             + process.stderr.decode()
             + "#" * 80 + "\n"
             )
colour  =    green if all_failed else      red
summary = "PASSED" if all_failed else "FAILED"
print( colour
     , "=" * 40 +  "\n "
     + "=" * 11 + f" SUMMARY : {summary} " + "=" * 11 + "\n "
     + "=" * 40
     , reset_colour)

sys.exit(0 if all_failed else 1)
