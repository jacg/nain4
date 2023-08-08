# Placement of physical volumes

<!-- TODO: put this in some common .css -->

<style>
details.g4 > summary::before {
  font-size: 80%;
  color: #888;
  content: "Click to show/hide equivalent in pure Geant4 ";
}
details.g4 {
  border-style: none none none solid;
  border-color: #888;
  padding-left: 1em;
}
</style>


## Summary
### World volumes

`nain4`'s placement API can be used to place a logical volume as it is being created

```c++
auto air = n4::material("G4_AIR");
n4::box("world").cube("1*m").place(air).now();
```
<details class="g4"> <summary></summary>

  ```c++
  auto air = G4NistManager::Instance() -> FindOrBuildMaterial("G4_AIR");
  auto world_size = 1*m;
  auto world_solid = new G4Box("world", world_size/2, world_size/2, world_size/2);
  auto world_logical = new G4LogicalVolume(world_solid, air, "world")
  new G4PVPlacement(nullptr, {}, world_logical, "world", nullptr, false, 0);
  ```
</details>

or to place an existing logical volume
```c++
auto air = n4::material("G4_AIR");
G4LogicalVolume* existing_logical_volume = n4::box("world").cube(1*m).volume(air);
n4::place(existing_logical_volume).now();
```
<font size=-1>(The Geant4 equivalent of this sample is essentially identical to that of the previous sample.)</font>

In the above examples, no mother volume was specified in the placement phase,
therefore these volumes are *world* volumes: all other volumes must be placed
within the world volume—either directly, or in its daughters—and they must not
protrude beyond the bounds of the world volume.

<!-- TODO replace with an mdbook-admonish thing, or some other higher-level
feature --> <font color="red">NOTE: the volume is not placed until the `.now()`
method is called.</font> See [Placement: Laziness and
Accumulation](./placement-laziness-and-accumulation.md) for more details.

### Daughter volumes

+ Daughter volumes are placed inside a mother volume with the `.in(mother)`
  method.
+ The position and orientation, as well as other information about the
  placement, can be specified with the placement API's auxiliary methods.

Just like in the case of world volumes, a logical volume can be placed as it is being created

```c++
auto copper = n4::material("G4_Cu");
n4::box("box").cube(1*cm).place(copper).in(world).at_y(3*cm).rot_z(30*deg).now();
```
<details class="g4"> <summary></summary>

  ```c++
  auto copper = G4NistManager::Instance() -> FindOrBuildMaterial("G4_Cu");
  auto box_size = 1*cm;
  auto box_solid = new G4Box("box", box_size/2, box_size/2, box_size/2);
  auto box_logical = new G4LogicalVolume(box_solid, copper, "box")
  auto rot_z_30 = new G4RotationMatrix();
  rot_z_30 -> rotateY(30*deg);
  new G4PVPlacement(rot_z_30, {0, 3*cm, 0}, box_logical, "box", world, false, 0);
  ```
</details>

or in two separate steps
```c++
auto copper = n4::material("G4_Cu");
auto box = n4::box("box").cube(1*cm).volume(copper);
n4::place(box).in(world).at_y(3*cm).rot_z(30*deg).now();
```
## Methods

+ Mother volume
  - `in(logical-volume)`
  - `in(phisical-volume)` // not implemented yet
  - `in(n4::place)` // not implemented yet

  If no mother volume is specified, the placed volume becomes the world volume.
  There can be only one world volume in a Geant4 application. `nain4` issues a
  run-time error if more than one world volume is created.

+ Translations
  - `at(G4ThreeVector)`
  - `at(x,y,z)`
  - `at_x(x)`, `at_y(y)`, `at_z(z)`

  The above methods specify the displacement of the daughter volume's origin with respect to the mother volume's origin.

  By default there is no displacement between the two.

  The effect of these methods is cumulative. Thus the following are equivalent

  ```c++
  .at  (1*m,      2*m,      3*m)
  .at_x(1*m).at_y(2*m).at_z(3*m)
  ```
  as are these
  ```c++
  .at_x(101*m)
  .at_x(1*m).at_x(100*m)
  ```
  NOTE: [Displacements and rotations are not commutative](#displacements-and-rotations-are-not-commutative).


+ Rotations
  - `rotate(G4RotationMatrix)`, `rot(G4RotationMatrix)` (identical meanings)
  - `rotate_x(angle)`, `rotate_y(angle)`, `rotate_z(angle)`
  - `rot_x(angle)`, `rot_y(angle)`, `rot_z(angle)` (shorter-named equivalents of the above)

  The above methods specify the rotation of the daughter volume with respect to its mother volume.

  By default there is no rotation between the two.

  The effect of these methods is cumulative. Thus, the following two lines are equivalent

  ```c++
  .rot_x(60*deg).rot_x(30*deg)
  .rot_x(90*deg)
  ```

  NOTE: Rotations around distinct axes are not commutative. Thus the following two lines are **NOT** equivalent

  ```c++
  .rot_x(90*deg).rot_y(90*deg)
  .rot_y(90*deg).rot_x(90*deg)
  ```

  NOTE: [Displacements and rotations are not commutative](./displacements-and-rotations-are-not-commutative.md).



+ `name(string)` By default the physical volume inherits the name of the logical
  volume being placed. This method allows overriding the inherited name.

+ `copy_no(N)` Useful to distinguish between multiple placements of a single
  logical volume. Implicitly appends "-N" to the physical volume's name, in
  addition to setting an copy-number attribute.


+ Overlap checking

  Besides daughter volumes being placed entirely within their mothers, there
  should be no overlap between volumes. If this happens, the behaviour of
  tracking [becomes inconsistent](https://geant4-forum.web.cern.ch/t/whats-the-consequence-of-overlapping/4914/8).

  The following methods control overlap checking.

  When overlap checking is enabled, Geant4 will throw a runtime error if
  overlaps are detected, as opposed to allowing your program to run with an
  ill-defined geometry. However, this process may consume significant resources
  and slow down the construction of the geometry. Therefore, you may not want to
  have overlap checking enabled permanently.

  - `check_overlaps()` Activates checking for this placement only.

  - `n4::place::check_overlaps_switch_on()` Activate checking for all placements until further notice.
  - `n4::place::check_overlaps_switch_off()` Deactivate checking until further
    notice, for placements which do not have it explicitly enabled with
    `.check_overlaps()`.

+ Controlling state accumulation

  - `clone()` Create an independent copy of the current placement state.

  Observe the difference in the effect of these two loops, whose source differs
  only in the presence/absence of `.clone()`

  ```c++
  place_box = n4::box("box").cube(1*m).place(copper).in(world);

  // Place boxes at 1, 2 and 3 metres
  for (auto k : {1,2,3}) { place_box.clone().at_x(k*m).now(); }
  // Place boxes at 1, 3 and 6 metres
  for (auto k : {1,2,3}) { place_box        .at_x(k*m).now(); }
  ```
  See [Placement: Laziness and Accumulation](./placement-laziness-and-accumulation.md).
