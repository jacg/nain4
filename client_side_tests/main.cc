#include <G4Box.hh>
#include "nain4.hh"

int main() {
  auto water = nain4::material("G4_WATER");
  auto lx = 1.;
  auto ly = 2.;
  auto lz = 3.;

  auto box = nain4::volume<G4Box>("test_box", water, lx, ly, lz);
  nain4::place(box).at(0., 0., 0.).now();

  return 0;
}
