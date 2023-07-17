#include "n4_run_manager.hh"

#include <cstdlib>
#include <iostream>
#include <memory>


namespace nain4 {

run_manager&& run_manager::init() {
  bool must_exit = false;
  for (auto [name, count]: user_init_set_count) {
    if (count != 1) {
      std::cerr << "\n\n\n\nERROR: " << name << " was set " << count << " times." << std::endl;
      must_exit = true;
    }
  }
  if (must_exit) {
    std::cerr << "\n\n\nERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR\n" << std::endl;
    std::cerr << "Each user initialization should be set exactly once.\n\n\n\n" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  manager_ -> Initialize();

  return std::move(*this);
}

} // namespace nain4
