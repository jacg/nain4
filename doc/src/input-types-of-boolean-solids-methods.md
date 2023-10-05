# Input types for boolean solids' methods

`nain4` provides a collection of methods to create boolean solids
described in [n4 boolean solids](./n4-boolean-solids.md). These
functions accept either a `n4::shape` or `G4VSolid*`. However, when
building a geometry, it's fairly easy to get confused with the
different stages of the creation of components (`solid`,
`(logical)volume`, `place(ment)`). This can produce errors that might
be difficult to understand. `nain4` attempts to clarify them
by providing explicit compile-time messages for the most
likely cases. Here we describe all these errors and try to give a
through explanation of their causes and how to handle them.

For this explanation we will use the addition of two boxes as an
example, but the rationale does not depend on the shapes or the
methods used. Here is the definition of the boxes:

```c++
n4::box box1 = n4::box("box1").cube(1*m);
n4::box box2 = n4::box("box2").cube(2*m);
G4Box*  box3 = new G4Box{"box3", 3*m, 3*m, 3*m};
```

## The correct syntax
For reference, here are three possible options to create a union
solid from these boxes:

```c++
box1.add(box2        ); // OK
box1.add(box2.solid()); // OK
box1.add(box3        ); // OK
```

Here, `add` could be replaced with its alias `join` to produce the same
result. Also, notice that the union solid is not created until
`.solid()`, `volume(...)` or `.place(...)` is called on the result of
`add(...)`.

## Possible errors
All the errors share the same message, with a hint of what the type
of the incorrect input is. The error message looks like this:

```
[n4::boolean_shape::<METHOD>]
Attempted to create a boolean shape using <TYPE_HINT>.
Only n4::shape and G4VSolid* are accepted.
For more details, please check https://jacg.github.io/nain4/explanation/boolean_solid_input_types.md
```

where `<METHOD>` indicates the method which generated the error (this
might help in the case of chained operations) and `<TYPE_HINT>`
indicates the input type that gave rise to the error.

### `<TYPE_HINT> = a G4LogicalVolume`

The variable passed to the boolean-creation method is of type
`G4LogicalVolume*`. The most likely causes are:

1. [G4-style] this variable was bound to a `new G4LogicalVolume(...)`, e.g.
   ```c++
   G4Box*           box2_solid = new G4Box(...);
   G4LogicalVolume* box2_logic = new G4LogicalVolume(box2_solid, ...);
   box1.add(box2_logic); // WRONG
   ```
   The solution is to pass `box2_solid` instead of `box2_logic`.

2. [n4-style] you called `.volume(...)` on the input solid, e.g.
   ```c++
   box1.add(box2.volume(...)); // WRONG
   ```
   The solution is to remove the call to `volume(...)` or call `.solid()`
   instead.


### `<TYPE_HINT> = a G4PVPlacement`

The variable passed to the boolean-creation method is of type
`G4PVPlacement*`. The most likely causes are:
1. [G4-style] this variable was assigned to `new G4PVPlacement(...)`, e.g.
   ```c++
   G4Box*           box2_solid = new G4Box(...);
   G4LogicalVolume* box2_logic = new G4LogicalVolume(box2_solid, ...);
   G4PVPlacement*   box2_place = new G4PVPlacement(box2_logic, ...);
   box1.add(box2_place); // WRONG
   ```
   The solution is to pass `box2_solid` instead of `box2_place`.

2. [n4-style] you called `.place(...).other().methods().now()` on the input solid, e.g.
   ```c++
   box1.add(box2.place(...).other().methods().now()); // WRONG
   ```
   The solution is to remove the call to
   `place(...).other().methods().now()` or call `.solid()` instead.


### `<TYPE_HINT> = a n4::place`

The variable passed to the boolean-creation method is of type
`n4::place`. The most likely cause is that you called `.place(...)`
(without calling `.now()`) on the input solid, e.g.

```c++
box1.add(box2.place(...).other().methods()); // WRONG
```

The solution is to remove the call to `place(...).other().methods()` or
call `.solid()` instead.


### `<TYPE_HINT> = an unknown type`

The type of the variable passed to the boolean-creation method is
neither a valid one (`n4::shape`, `G4VSolid*`) nor one of the specific
cases described above. In this case `nain4` cannot provide any
help. You must find the latest assignment of this variable and
understand what its type is. A LSP plugin in you text editor might
help.


## Requesting the handling of other cases

If you find that there is another case that `nain4` should handle,
please open an issue at `https://github.com/jacg/nain4/issues`!
