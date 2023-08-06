# Constructing solids and logical volumes

Header: `<n4-volumes.hh>`

## Summary

### Constructing a `G4VSolid`

```c++
auto ball = n4::sphere("ball").r(1.2*m).solid();
```
<details>
  <summary>See equivalent in pure Geant4</summary>

  ```c++
  auto ball = new G4Sphere("ball", 0, radius, 0, CLHEP::pi, 0, CLHEP::twopi);
  ```
  <font size=-2>(In this specific example, `n4::sphere` notices that it would be more efficient to create a `G4Orb` instead of a `G4Sphere` and does that for you automatically.)</font>
</details>

### Constructing a `G4LogicalVolume`

Frequently, after having made a `G4VSolid` you immediately use it to make a `G4LogicalVolume` with the same name. `nain4` allows you to do this in a single step:

```c++
auto copper = n4::material("G4_Cu");
auto ball   = n4::sphere("ball").r(1.2*m).volume(copper);
```
<details>
  <summary>See equivalent in pure Geant4</summary>

  ```c++
  auto copper = G4NistManager::Instance() -> FindOrBuildMaterial("G4_Cu");
  auto ball_solid = new G4Sphere("ball", 0, radius, 0, CLHEP::pi, 0, CLHEP::twopi);
  auto ball = new G4VLogicalVolume(ball_solid, copper, "ball");
  ```
</details>

If you need to do this as two separate steps

```c++
auto copper     = n4::material("G4_Cu");
auto ball_solid = n4::sphere("ball").r(1.2*m).solid();
auto ball       = n4::volume(ball_solid, copper);
```

Not all `G4VSolid`s are supported by the `nain4::shape` interface, yet. In such cases `n4::volume` lets you combine the construction of a solid and corresponding logical volume in a single step. Here is how you could do it if `n4::sphere` did not exist:

```c++
auto copper = n4::material("G4_Cu");
auto ball   = n4::volume<G4Sphere>("ball", copper, 0, radius, 0, CLHEP::pi, 0, CLHEP::twopi);
```
The arguments passed after the name and material, are forwarded to the specified `G4VSolid`'s constructor.

### Placing a volume
Should you want to place your `G4LogicalVolume` immediately, `nain4` allows you to include this in a single step, too:
```c++
auto safe = ...
auto gold = n4::material("G4_Au");
n4::box("nugget").cube(2*cm).place(gold).in(safe).now();
```
<details>
  <summary>See equivalent in pure Geant4</summary>

  ```c++
  auto safe = ...
  auto gold = G4NistManager::Instance() -> FindOrBuildMaterial("G4_Au");
  auto nugget_solid = new G4Box("nugget", 2*cm/2, 2*cm/2, 2*cm/2);
  auto nugget_logical = new G4VLogicalVolume(nugget_solid, gold, "nugget");
  new G4PVPlacement(nullptr, {}, nugget_logical, "nugget", safe, false, 0);
  ```
</details>

However, frequently you need to keep a handle to the logical volume, in order to be able to place things into it later. In such cases you would break this into two separate steps:

```c++
auto safe = ...
auto gold = n4::material("G4_Au");
auto nugget = n4::box("nugget").cube(2*cm).volume(gold);
n4::place(nugget).in(safe).now();
```

`nain4`'s placement utilities have a rich interface, which is described in [Placement of physical volumes](./n4-place.md).


### Summary
```c++
G4VSolid        * s = n4::SOLID("name", ...).solid();
G4VLogicalVolume* v = n4::SOLID("name", ...).volume(material);
G4PVPlacement   * p = n4::SOLID("name", ...).place (material).in(volume) ... .now();
```

## Specifying Dimensions

## Available Shapes

