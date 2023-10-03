#ifndef SHARED_HH
#define SHARED_HH

#include <G4GenericMessenger.hh>
#include <G4SystemOfUnits.hh>
#include <G4String.hh>
#include <G4Types.hh>

#include <fstream>
#include <memory>
#include <vector>


struct config {
  config();

  G4double csi_scint_yield = 3200 / MeV;
  G4String particle        = "e-";
  G4bool   cold            = false;
  static G4bool debug;

private:
  void set_random_seed(G4long seed);
  std::unique_ptr<G4GenericMessenger> msngr;
};

struct data {
  std::vector<            G4double >       total_edep{ 0,  0};
  std::vector<std::vector<G4double>>         gamma_zs{{}, {}};
  std::vector<std::vector<G4double>> times_of_arrival{{}, {}};
  G4int double_hits  = 0;
  G4int event_number = 0;
};

struct output {
  std::ofstream gamma_z_data_files[2];
  std::ofstream    time_data_files[2];
  std::ofstream    edep_data_files[2];
};

#endif // SHARED_HH
