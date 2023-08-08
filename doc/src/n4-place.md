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

<!-- TODO replace with an mdbook-admonish thing, or some other higher-level feature  -->
<font color="red">NOTE: the volume is not placed until the `.now()` method is called.</font> See [Lazy evaluation](#lazy-evaluation) for more details. 

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

## Lazy evaluation


