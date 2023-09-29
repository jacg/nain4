function os_is() {
    OS=$1
    kernel_version=$(uname -v)
    echo $kernel_version | grep $OS > /dev/null
}

function system_requires_nixgl() {
    ! os_is "NixOS" && ! os_is "Darwin"
}

function cli_requires_nixgl() {
    CLI_WANTS_GRAPHICS=$(echo \"$@\" | grep -E " \-\-vis| \-g")
    if [ -z "$CLI_WANTS_GRAPHICS" ]
    then
        echo CLI does NOT require graphics
        false
    else
        echo CLI requires graphics
        true
    fi
}

if system_requires_nixgl && cli_requires_nixgl $@
then
    echo Running with nixGL
    nix --extra-experimental-features "nix-command flakes" run --impure github:guibou/nixGL -- "$@"
else
    echo Running without nixGL
    "$@"
fi
