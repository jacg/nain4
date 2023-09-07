
function os_is() {
    OS=$1
    kernel_version=$(uname -v)
    echo $kernel_version | grep $OS > /dev/null
}

function run_with_nixgl() {
    ! os_is "NixOS" && ! os_is "Darwin"
}

if run_with_nixgl; then
    nix --extra-experimental-features "nix-command flakes" run --impure github:guibou/nixGL -- $@
else
    $@
fi
