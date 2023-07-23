#include "n4_run_manager.hh"

namespace nain4 {

run_manager* run_manager::rm_instance   = nullptr;
bool         run_manager::create_called = false;
}
