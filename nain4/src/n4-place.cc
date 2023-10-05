#include <n4-place.hh>

#include <G4LogicalVolume.hh>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

namespace nain4 {

bool place::global_check_overlaps_ = false;

G4PVPlacement* place::now() {
  // ----- Name --------------------------------------------------
  // + By default, the name is copied from the child volume.
  // + If a copy_number is specified, it is appended to the name.
  // + All of this is overriden if a name is provided explicitly.
  G4String the_name;
  if (this->label) {
    the_name = this->label.value();
  } else {
    the_name = this->child.value() -> GetName();
    // TODO: G4 already appends the copy number to the name?
    if (this->copy_number) {
      auto suffix = "-" + std::to_string(copy_number.value());
      the_name += suffix;
    }
  }
  // TODO: Think about these later
  bool WTF_is_pMany   = false;

  return new G4PVPlacement{transformation,
                           child.value(),
                           the_name,
                           parent.value_or(nullptr),
                           WTF_is_pMany,
                           copy_number.value_or(0),
                           global_check_overlaps_ || local_check_overlaps_};
}

G4LogicalVolume* place::get_logical() {
  if (!child.has_value()) {
    auto name = label.value();
    std::cerr << "Called `n4::place::get_logical` on an incomplete"
              << " `n4::place` instance (" + name + "). Aborting."
              << std::endl;
    exit(EXIT_FAILURE);
  }

  return child.value();
}

} // namespace nain4

#pragma GCC diagnostic pop
