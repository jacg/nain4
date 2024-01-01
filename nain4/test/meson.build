project( 'nain4-test'
       , 'cpp'
       , version : 'v0.2.0'
       , default_options : [ 'buildtype=debugoptimized'
                           , 'debug=true'
                           , 'optimization=2'
                           , 'cpp_std=c++20'
                           , 'prefix=@0@/../../install/nain4-test'.format(meson.source_root())
                           ]
       )

install_prefix = get_option('prefix')

nain4  = dependency( 'nain4'
                   , method  : 'pkg-config'
                   , required: true
                   )

catch2 = dependency( 'catch2'
                   , method  : 'pkg-config'
                   , required: true
                   )

geant4 = dependency( 'Geant4'
                   , method  : 'cmake'
                   , required: true
                   , components: ['ui_all', 'vis_all']
                   , modules : [ 'Geant4::G4ptl'              , 'Geant4::G4analysis'    , 'Geant4::G4digits_hits'
                               , 'Geant4::G4error_propagation', 'Geant4::G4event'       , 'Geant4::G4tools'
                               , 'Geant4::G3toG4'             , 'Geant4::G4geometry'    , 'Geant4::G4global'
                               , 'Geant4::G4graphics_reps'    , 'Geant4::G4intercoms'   , 'Geant4::G4interfaces'
                               , 'Geant4::G4materials'        , 'Geant4::G4parmodels'   , 'Geant4::G4particles'
                               , 'Geant4::G4persistency'      , 'Geant4::G4physicslists', 'Geant4::G4processes'
                               , 'Geant4::G4readout'          , 'Geant4::G4run'         , 'Geant4::G4tasking'
                               , 'Geant4::G4track'            , 'Geant4::G4tracking'    , 'Geant4::G4FR'
                               , 'Geant4::G4visHepRep'        , 'Geant4::G4RayTracer'   , 'Geant4::G4Tree'
                               , 'Geant4::G4VRML'             , 'Geant4::G4gl2ps'       , 'Geant4::G4GMocren'
                               , 'Geant4::G4vis_management'   , 'Geant4::G4modeling'    , 'Geant4::G4OpenGL'
                               , 'Geant4::G4UIVisDefinitions'
                               ]
                   )

nain4_test_deps    = [nain4, geant4, catch2]
nain4_test_include = include_directories('.')
nain4_test_sources = [ 'test-nain4.cc'
                     , 'test-external.cc'
                     , 'test-material.cc'
                     , 'test-random.cc'
                     , 'test-sensitive.cc'
                     , 'test-sequences.cc'
                     , 'test-shape.cc'
                     , 'test-vis-attributes.cc'
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
