#ifndef nain4_ui_hh
#define nain4_ui_hh

#include <G4Types.hh>
#include <G4String.hh>

#include <optional>


namespace nain4 {

class ui {
public:
  ui(int argc, char** argv);
  void run();

private:
  std::optional<G4int   >    n_events;
  std::optional<G4String> early_macro;
  std::optional<G4String>  late_macro;
  std::optional<G4String>   vis_macro;
};

} // namespace nain4

namespace n4{ using namespace nain4; }

#endif // nain4_ui_hh
