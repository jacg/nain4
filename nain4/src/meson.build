project( 'nain4'
       , 'cpp'
       , version : 'v0.1.10'
       , default_options : [ 'buildtype=debugoptimized'
                           , 'debug=true'
                           , 'optimization=2'
                           , 'default_library=shared'
                           , 'cpp_std=c++17'
                           , 'prefix=@0@/../../install/nain4'.format(meson.source_root())
                           , 'libdir=lib' # removes architecture from path
                           ]
       )


geant4 = dependency( 'Geant4'
                   , method  : 'cmake'
                   , required: true
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
                               ])

nain4_deps    = [geant4]
nain4_include = include_directories('.')

nain4_includes = [ 'g4-mandatory.hh'
                 , 'n4-utils.hh'
                 , 'n4-volumes.hh'
                 , 'n4_run_manager.hh'
                 , 'n4_ui.hh'
                 , 'nain4.hh'
                 ]

nain4_sources = [ 'g4-mandatory.cc'
                , 'n4-utils.cc'
                , 'n4-volumes.cc'
                , 'n4_run_manager.cc'
                , 'n4_ui.cc'
                , 'nain4.cc'
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
            )
