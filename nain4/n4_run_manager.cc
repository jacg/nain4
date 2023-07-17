#include "n4_run_manager.hh"

#include <memory>

namespace nain4 {

run_manager& run_manager::init() {

    // TODO warn about unset user actions
    // std::vector<std::string> missing;

    manager_ -> Initialize();

    return *this;
}

} // namespace nain4
