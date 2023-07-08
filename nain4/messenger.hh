#include <G4GenericMessenger.hh>

namespace nain4 {
  using G4CMD = G4GenericMessenger::G4GenericMessenger::Command;


  struct command {
    command(G4CMD& handle_) : handle(handle_) {}

    #define FORWARD(NEW_NAME, TYPE, OLD_NAME) command& NEW_NAME(TYPE v){handle.OLD_NAME(v); return *this;}
    FORWARD(  dimension, G4String, SetUnitCategory)
    FORWARD(       unit, G4String, SetUnit)
    FORWARD(description, G4String, SetGuidance)
    FORWARD(      range, G4String, SetRange)
    FORWARD(    options, G4String, SetCandidates)
    FORWARD( default_to, G4String, SetDefaultValue)

    command& required(){handle.SetParameterName(get_name(), true); return *this;}
    command& optional(){handle.SetParameterName(get_name(), false); return *this;}
    #undef FORWARD

  private:
    G4CMD& handle;

    inline G4String get_name() { return handle.command->GetCommandName(); }
  };

  struct messenger : public G4GenericMessenger {
    using G4GenericMessenger::G4GenericMessenger;

    template<class VAR>
    command add(G4String name, VAR& variable, G4String doc="") {
      auto g4cmd = DeclareProperty(name, variable, doc);
      return command{g4cmd};
    }
  };

}
