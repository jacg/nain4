project( 'nain4'
       , 'cpp'
       , version : 'v0.2.0'
       , default_options : [ 'buildtype=debugoptimized'
                           , 'debug=true'
                           , 'optimization=2'
                           , 'default_library=shared'
                           , 'cpp_std=c++20'
                           , 'prefix=@0@/../../install/nain4'.format(meson.source_root())
                           , 'libdir=lib' # removes architecture from path
                           ]
       )


argparse = dependency( 'argparse'
                     , method : 'pkg-config')

boost = dependency( 'boost'
                  , method : 'pkg-config')

geant4_modules = [ 'Geant4::G3toG4'             , 'Geant4::G4FR'
                 , 'Geant4::G4GMocren'          , 'Geant4::G4OpenGL'
                 , 'Geant4::G4RayTracer'        , 'Geant4::G4ToolsSG'
                 , 'Geant4::G4Tree'             , 'Geant4::G4VRML'
                 , 'Geant4::G4analysis'         , 'Geant4::G4digits_hits'
                 , 'Geant4::G4error_propagation', 'Geant4::G4event'
                 , 'Geant4::G4gdml'             , 'Geant4::G4geometry'
                 , 'Geant4::G4geomtext'         , 'Geant4::G4global'
                 , 'Geant4::G4graphics_reps'    , 'Geant4::G4intercoms'
                 , 'Geant4::G4interfaces'       , 'Geant4::G4materials'
                 , 'Geant4::G4mctruth'          , 'Geant4::G4modeling'
                 , 'Geant4::G4parmodels'        , 'Geant4::G4particles'
                 , 'Geant4::G4physicslists'     , 'Geant4::G4processes'
                 , 'Geant4::G4ptl'              , 'Geant4::G4readout'
                 , 'Geant4::G4run'              , 'Geant4::G4tools'
                 , 'Geant4::G4track'            , 'Geant4::G4tracking'
                 , 'Geant4::G4visHepRep'        , 'Geant4::G4vis_management'

                 # for some reason meson can't find these modules,
                 # even though they are suggested when the argument
                 # `modules` below is omitted. The curated list above
                 # matches that given by the command `geant4-config --libs`
                 #, 'Geant4::G4UIVisDefinitions'
                 #, 'Geant4::G4gl2ps'
                 #, 'Geant4::G4persistency'
                 #, 'Geant4::G4tasking'
]


geant4 = dependency( 'Geant4'
                   , method     : 'cmake'
                   , required   : true
                   , components : ['ui_all', 'vis_all']
                   , modules    : geant4_modules
                   )

nain4_deps    = [argparse, geant4]
nain4_include = include_directories('.')

nain4_includes = [ 'n4-all.hh'
                 , 'n4-boolean-shape.hh'
                 , 'n4-constants.hh'
                 , 'n4-defaults.hh'
                 , 'n4-exceptions.hh'
                 , 'n4-geometry-iterators.hh'
                 , 'n4-geometry.hh'
                 , 'n4-will-become-external-lib.hh'
                 , 'n4-inspect.hh'
                 , 'n4-main.hh'
                 , 'n4-mandatory.hh'
                 , 'n4-material.hh'
                 , 'n4-place.hh'
                 , 'n4-random.hh'
                 , 'n4-run-manager.hh'
                 , 'n4-sensitive.hh'
                 , 'n4-sequences.hh'
                 , 'n4-shape.hh'
                 , 'n4-stats.hh'
                 , 'n4-stream.hh'
                 , 'n4-testing.hh'
                 , 'n4-ui.hh'
                 , 'n4-utils.hh'
                 , 'n4-vis-attributes.hh'
                 , 'n4-volume.hh'
                 ]


nain4_sources = [ 'n4-boolean-shape.cc'
                , 'n4-constants.cc'
                , 'n4-geometry-iterators.cc'
                , 'n4-will-become-external-lib.cc'
                , 'n4-mandatory.cc'
                , 'n4-material.cc'
                , 'n4-place.cc'
                , 'n4-random.cc'
                , 'n4-run-manager.cc'
                , 'n4-sensitive.cc'
                , 'n4-sequences.cc'
                , 'n4-shape.cc'
                , 'n4-stream.cc'
                , 'n4-ui.cc'
                , 'n4-volume.cc'
                ]

geant4_include = geant4.get_variable(cmake: 'Geant4_INCLUDE_DIRS')

nain4_lib = shared_library( 'nain4'
                          , nain4_sources
                          , include_directories: [nain4_include, geant4_include]
                          , dependencies       : nain4_deps
                          , install            : true
                          )

nain4_dep = declare_dependency( include_directories: nain4_include
                              , link_with          : nain4_lib)


install_headers(nain4_includes)

pkg = import('pkgconfig')
pkg.generate( nain4_lib
            , description: 'nain4'
            , name       : 'nain4'
            , filebase   : 'nain4'
            , variables  : 'geant4_modules=' + ','.join(geant4_modules)
            )


cmake = import('cmake')

install_prefix = get_option('prefix')

conf_data = configuration_data({
  'install_prefix' : install_prefix,
})

cmake.configure_package_config_file(
  name          : 'nain4',
  input         : 'nain4Config.cmake.in',
  configuration : conf_data,
)
