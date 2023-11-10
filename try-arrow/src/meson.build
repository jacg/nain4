project( 'try-arrow'
       , 'cpp'
       , version : 'v0.0.1'
       , default_options : [ 'buildtype=debugoptimized'
                           , 'debug=true'
                           , 'optimization=2'
                           , 'cpp_std=c++20'
                           , 'prefix=@0@/../../install/try-arrow'.format(meson.source_root())
                           ]
       )

arrow = dependency( 'arrow'
                  , method : 'pkg-config'
                  , required : true
                  )

parquet = dependency('parquet')

try_arrow_deps    = [arrow, parquet]
try_arrow_include = include_directories('.')
try_arrow_sources = [ 'try-arrow.cc' ]
try_arrow_exe = executable( 'try-arrow'
                           , try_arrow_sources
                           , include_directories: [ try_arrow_include ]
                           , dependencies       : [ try_arrow_deps ]
                           , install            : true
                           )
