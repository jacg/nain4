# Constructing solids and logical volumes

Header: `<n4-volumes.hh>`

## Summary

### Constructing a `G4VSolid`

```c++
auto ball = n4::sphere("ball").r(1.2*m).solid();
```
<details>
  <summary><font size=-2>Click to show/hide equivalent in pure Geant4</font></summary>

  ```c++
  auto ball = new G4Sphere("ball", 0, radius, 0, CLHEP::twopi, 0, CLHEP::pi);
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
  <summary><font size=-2>Click to show/hide equivalent in pure Geant4</font></summary>

  ```c++
  auto copper = G4NistManager::Instance() -> FindOrBuildMaterial("G4_Cu");
  auto ball_solid = new G4Sphere("ball", 0, radius, 0, CLHEP::twopi, 0, CLHEP::pi);
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
auto ball   = n4::volume<G4Sphere>("ball", copper, 0, radius, 0, CLHEP::twopi, 0, CLHEP::pi);
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
  <summary><font size=-2>Click to show/hide equivalent in pure Geant4</font></summary>

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

The `G4VSolid`s, and hence also the `n4::shape`s, are parameterized by combinations of four principal types of coordinate

1. Cartesian lengths, `x`, `y` and `z`
2. Radial length, `r`
3. Azimuthal angle, `φ`
4. Polar angle, `θ`

`nain4` provides a consistent set of methods for setting these, in any `n4::shape` that uses them. These methods are described here, the `n4::shapes` are described in the next section.

### Cartesian lengths

The principal methods for setting Cartesian lengths are

+ `x`
+ `half_x`

and their equivalents for `y` and `z`.

If you set a Cartesian length more than once in the same shape, the last setting overrides previous ones. For example:

```c++
.x(1*m).half_x(3*m)  // `x` set to 12 m
.x(1*m).     x(3*m)  // `x` set to  3 m
```

`n4::shape`s which depend on more than one Cartesian length, typically provide extra methods for setting various combinations, for example `n4::box` offers extra methods `cube`, `xyz`, `xy`, `xz` and `yz` along with their `half_` variants.

### Radial length: `r`

Three methods are provided for specifying the two degrees of freedom in radial lengths:

+ `r_inner`
+ `r_delta`
+ `r`

Valid combinations of these methods are

<style>
.thick {
    border-left-width: 8px;
    border-left-colour: #888;
  }
</style>

<table>
  <tr>
    <th colspan="3">Methods used</th>
    <th colspan="3">Implied value</th>
  </tr>
  <tr>
    <td colspan="3"></td><td class="thick">r_inner</td><td>r_delta</td><td>r</td>
  </tr>
  <tr><td>       </td><td>       </td><td>r</td>  <td class="thick">0          </td><td>r          </td><td>                 </td></tr>
  <tr><td>r_inner</td><td>       </td><td>r</td>  <td class="thick">           </td><td>r - r_inner</td><td>                 </td></tr>
  <tr><td>       </td><td>r_delta</td><td>r</td>  <td class="thick">r - r_delta</td><td>           </td><td>                 </td><td>TODO</td></tr>
  <tr><td>r_inner</td><td>r_delta</td><td> </td>  <td class="thick">0          </td><td>           </td><td>r_inner + r_delta</td></tr>
</table>

Some shapes, such as `n4::cons` (`G4Cons`), have multiple radii. In such cases the method names acquire a number, to distinguish between them `r*` -> `r1*`, `r2*`.

### Azimuthal angle: `φ`

Three methods are provided for specifying the two degrees of freedom in azimuthal angles:

+ `phi_start`
+ `phi_delta`
+ `phi_end`

Valid combinations of these methods are

<style>
.thick {
    border-left-width: 8px;
    border-left-colour: #888;
  }
</style>

<table>
  <tr>
    <th colspan="3">Methods used</th>
    <th colspan="3">Implied value</th>
  </tr>
  <tr>
    <td colspan="3"></td><td class="thick">phi_start</td><td>phi_delta</td><td>phi_end</td>
  </tr>
  <tr><td>         </td><td>         </td><td>       </td>  <td class="thick">0      </td><td>2π         </td><td>2π       </td></tr>
  <tr><td>phi_start</td><td>         </td><td>       </td>  <td class="thick">       </td><td>2π - start </td><td>2π       </td></tr>
  <tr><td>         </td><td>phi_delta</td><td>       </td>  <td class="thick">0      </td><td>           </td><td>δ        </td></tr>
  <tr><td>         </td><td>         </td><td>phi_end</td>  <td class="thick">0      </td><td>end        </td><td>         </td></tr>
  <tr><td>         </td><td>phi_delta</td><td>phi_end</td>  <td class="thick">end - δ</td><td>           </td><td>         </td><td>TODO</td></tr>
  <tr><td>phi_start</td><td>         </td><td>phi_end</td>  <td class="thick">       </td><td>end - start</td><td>         </td></tr>
  <tr><td>phi_start</td><td>phi_delta</td><td>       </td>  <td class="thick">       </td><td>           </td><td>start + δ</td></tr>
</table>

### Polar angle: `θ`

Three methods are provided for specifying the two degrees of freedom in polar angles:

+ `theta_start`
+ `theta_delta`
+ `theta_end`

Valid combinations of these methods are

<style>
.thick {
    border-left-width: 8px;
    border-left-colour: #888;
  }
</style>

<table>
  <tr>
    <th colspan="3">Methods used</th>
    <th colspan="3">Implied value</th>
  </tr>
  <tr>
    <td colspan="3"></td><td class="thick">theta_start</td><td>theta_delta</td><td>theta_end</td>
  </tr>
  <tr><td>           </td><td>           </td><td>         </td>  <td class="thick">0      </td><td>π          </td><td>π        </td></tr>
  <tr><td>theta_start</td><td>           </td><td>         </td>  <td class="thick">       </td><td>π - start </td><td>π        </td></tr>
  <tr><td>           </td><td>theta_delta</td><td>         </td>  <td class="thick">0      </td><td>           </td><td>δ        </td></tr>
  <tr><td>           </td><td>           </td><td>theta_end</td>  <td class="thick">0      </td><td>end        </td><td>         </td></tr>
  <tr><td>           </td><td>theta_delta</td><td>theta_end</td>  <td class="thick">end - δ</td><td>           </td><td>         </td><td>TODO</td></tr>
  <tr><td>theta_start</td><td>           </td><td>theta_end</td>  <td class="thick">       </td><td>end - start</td><td>         </td></tr>
  <tr><td>theta_start</td><td>theta_delta</td><td>         </td>  <td class="thick">       </td><td>           </td><td>start + δ</td></tr>
</table>


## Available Shapes

### `n4::box`

Returns `G4Box*`: cuboid with side lengths `x`, `y` and `z`, centred on the origin with sides parallel to the `x`/`y`/`z` axes.

#### Example
```c++
auto cross_section_length = 10*cm, y_length = 50*cm;
G4Box* box = n4::box("box").xz(cross_section_length).y(y_length).solid();
```
<details>
  <summary><font size=-2>Click to show/hide equivalent in pure Geant4</font></summary>

  ```c++
  auto cross_section_length = 10*cm, y_length = 50*cm;
  G4Box* box = new G4Box("box", cross_section_length/2, y_length/2, cross_section_length/2);
  ```
</details>

#### Methods

##### Full-length methods
All these methods take full (as opposed to half-) lengths:
+ `x(lx)`, `y(ly)`, `z(ly)`: set one dimension.
+ `xy(l)`, `xz(l)`, `yz(l)`: set two dimensions to the same value.
+ `cube(l)`: set all dimensions to the same value.
+ `xyz(lx, ly, lz)` (SUBJECT TO CHANGE) set all dimensions by providing a value for each.

##### Half-length methods
All the aforementioned full-length methods have alternatives which accept half-lengths: `half_x(lx/2)`, `half_cube(l/2)`, `half_xy(lx/2, ly/2)`, etc.
##### Overriding
If any value is specified more than once, the last setting overrides any earlier ones. Thus, the following three lines are equivalent. 

```c++
.cube(1*m          ).z(2*m)
.xyz (1*m, 1*m, 1*m).z(2*m) // TODO xyz is being revised
.xy  (1*m          ).z(2*m)
```
While the first two work, the last one states the intent most clearly.

### `n4::sphere`

Returns `G4Sphere*` or `G4Orb*`, depending on values provided.

+ `G4Sphere`: section of a spherical shell, between specified azimuthal (φ) and polar (θ) angles.
+ `G4Orb`: special case of `G4Sphere`.

#### Examples

+ A solid sphere
  ```c++
  G4Orb = n4::sphere("ball").r(1*m).solid()
  ```

  <details>
    <summary><font size=-2>Click to show/hide equivalent in pure Geant4</font></summary>

    ```c++
    G4Orb* ball = new G4Orb("ball", 0, 1*m, 0, CLHEP::twopi, 0, CLHEP::pi);
    ```
  </details>

+ A hollow sphere
  ```c++
  G4Sphere hollow = n4::sphere("hollow").r(2*m).r_delta(10*cm).solid()
  ```
  <details>
    <summary><font size=-2>Click to show/hide equivalent in pure Geant4</font></summary>

    ```c++
    auto outer = 2*m, thickness = 10*cm;
    G4Sphere* hollow = new G4Sphere("hollow", outer - thickness, outer, 0, CLHEP::twopi, 0, CLHEP::pi);
    ```
  </details>


+ A spherical wedge
  ```c++
  G4Sphere wedge = n4::sphere("wedge").r(1*m).phi_start(20*deg).phi_end(30*deg).solid()
  ```
  <details>
    <summary><font size=-2>Click to show/hide equivalent in pure Geant4</font></summary>

    ```c++
    auto start_phi = 20*deg, end_phi = 30*deg;
    G4Sphere* wedge = new G4Sphere("wedge", 0, 1*m, start_phi, end_phi - start_phi, 0, CLHEP::pi);
    ```
  </details>

#### Methods

+ `r_inner(l)`
+ `r_delta(l)`
+ `r(l)`
+ `phi_start(a)`
+ `phi_delta(a)`
+ `phi_end(a)`
+ `theta_start(a)`
+ `theta_delta(a)`
+ `theta_end(a)`

See the sections about setting [radial lengths](#radial-length-r), [azimuthal angles](#azimuthal-angle-φ) and [polar angles](#polar-angle-θ) for more details.

### `n4::tubs`
### `n4::trd`
### `n4::cons`

## Obviously missing shapes

### `n4::torus`
### `n4::trap`
### `n4::para`
### `n4::polycone`
### `n4::elliptical_tube`
### `n4::ellipse`
### `n4::ellipsoid`
### `n4::elliptical_cone`
### `n4::tet` (...rahedron)
### `n4::hype` (...rbolic cone)
### `n4::twisted_*`
### `n4::breps` Boundary Represented Solids
### `n4::tesselated_solids`



TODO: document envelope_of
