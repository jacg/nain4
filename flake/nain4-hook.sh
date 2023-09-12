nain4 () {
    export G4_DIR=@hook_g4_dir@
    export G4_EXAMPLES_DIR=@hook_g4_examples@
    export QT_QPA_PLATFORM_PLUGIN_PATH=@hook_qt_stuff@
}
postHooks+=(nain4)
