#include <n4-material.hh>
#include <n4-geometry.hh>

#include <G4Box.hh>

int main() {
  n4::box("test_box").xyz(1, 2, 3).place(nain4::material("G4_WATER")).now();
}
