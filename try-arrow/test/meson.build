project( 'test-arrow'
       , 'cpp'
       , version : 'v0.0.1'
       , default_options : [ 'buildtype=debugoptimized'
                           , 'debug=true'
                           , 'optimization=2'
                           , 'cpp_std=c++20'
                           , 'prefix=@0@/../../install/test-arrow'.format(meson.source_root())
                           ]
       )

install_prefix = get_option('prefix')

catch2 = dependency( 'catch2'
                   , method  : 'pkg-config'
                   , required: true
                   )

try_arrow_test_deps    = [catch2]
try_arrow_test_include = include_directories('.')
try_arrow_test_sources = [ 'test-arrow.cc'
                     , 'test-run-manager.cc'
                     , 'trivial-full-app-test.cc'
                     , 'catch2-main-test.cc'
                     ]

geant4_include = geant4.get_variable(cmake    : 'Geant4_INCLUDE_DIRS')
nain4_include  =  nain4.get_variable(pkgconfig: 'includedir'         )

test_executable = 'nain4-test'

nain4_test_exe = executable( test_executable
                           , nain4_test_sources
                           , include_directories: [nain4_test_include, nain4_include, geant4_include]
                           , dependencies       : nain4_test_deps
                           , install            : true
                           )

conf_data = configuration_data({
'install_prefix'  : install_prefix,
'test_executable' : test_executable,
})

configure_file(
  input         : 'run-each-test-in-separate-process.sh.in',
  output        : 'run-each-test-in-separate-process.sh',
  configuration : conf_data,
  install       : true,
  install_dir   : install_prefix
)
