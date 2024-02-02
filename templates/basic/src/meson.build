project( 'CHANGEME-PROJECT-NAME'
       , 'cpp'
       , version : 'v0.0.0'
       , default_options : [ 'buildtype=debugoptimized'
                           , 'debug=true'
                           , 'optimization=2'
                           , 'cpp_std=c++20'
                           , 'prefix=@0@/../install/CHANGEME-PROJECT-NAME/'.format(meson.source_root())
                           , 'libdir=lib'
                           ]
       )

nain4  = dependency( 'nain4'
                   , method  : 'pkg-config'
                   , required: true
                   )

geant4 = dependency( 'Geant4'
                   , method     : 'cmake'
                   , required   : true
                   , components : ['ui_all', 'vis_all']
                   , modules    : nain4.get_variable(pkgconfig: 'geant4_modules').split(','))

CHANGEME_SNAKE_LIB_deps     = [nain4, geant4]
CHANGEME_SNAKE_LIB_include  = include_directories('.')
CHANGEME_SNAKE_LIB_includes = ['LXe.hh']
CHANGEME_SNAKE_LIB_sources  = ['LXe.cc']

geant4_include = geant4.get_variable(cmake    : 'Geant4_INCLUDE_DIRS')
nain4_include  =  nain4.get_variable(pkgconfig: 'includedir'         )


CHANGEME_SNAKE_LIB = shared_library( 'CHANGEME_LIB'
                                   , CHANGEME_SNAKE_LIB_sources
                                   , include_directories: [CHANGEME_SNAKE_LIB_include, geant4_include]
                                   , dependencies       : CHANGEME_SNAKE_LIB_deps
                                   , install            : true
                                   )

CHANGEME_SNAKE_EXE = executable( 'CHANGEME-EXE'
                               , ['CHANGEME-PROJECT-NAME.cc']
                               , include_directories: [CHANGEME_SNAKE_LIB_include, nain4_include, geant4_include]
                               , dependencies       : CHANGEME_SNAKE_LIB_deps
                               , link_with          : CHANGEME_SNAKE_LIB
                               , install            : true
                               )

install_headers(CHANGEME_SNAKE_LIB_includes)

pkg = import('pkgconfig')
pkg.generate( CHANGEME_SNAKE_LIB
            , description: 'CHANGEME-ONE-LINE-PROJECT-DESCRIPTION'
            , name       : 'CHANGEME-PROJECT-NAME'
            , filebase   : 'CHANGEME-PROJECT-NAME'
            )
