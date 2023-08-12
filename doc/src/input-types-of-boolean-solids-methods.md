# Input types for boolean solids' methods

`nain4` provides a collection of methods to create boolean solids
described in [n4 boolean solids](./n4-boolean-solids.md). These
functions accept either a `n4::shape` or `G4VSolid`. However, when
building a geometry, it's fairly easy to get confused with the
different stages of the creation of components (`solid`,
`volume`/`logical`, `place`/`placement`). This can produce errors that
might be difficult to understand. `nain4` attempts to clarify these
errors by providing explicit messages for the most likely
occurrences. Here we gather all these errors and try to give a through
explanation of their causes and explain how to handle them.

For this explanation we will use the addition of two boxes as an
example, but the rationale does not depend on the shapes or the
methods used. Here is the definition of the boxes:

```c++
auto box1 = n4::box("box1").cube(1*m);
auto box2 = n4::box("box2").cube(2*m);
```

## The correct syntax
For reference, here are the two possible options to create a union
solid:

```c++
box1.add(box2        ); // OK
box1.add(box2.solid()); // OK
```

Here, `add` could be replace with its alias `join` to produce the same
result. Also, notice that the union solid is not created until
`.solid()`, `volume(...)` or `.place(...)` are called on the result of
`add(...)`.

## Possible errors
All the errors share the same message, with a hint of what's the type
of the input. The error message looks like this:

```
[n4::boolean_shape::<METHOD>]
Attempted to create a boolean shape using <TYPE_HINT>.
Only n4::shape and G4VSolid* are accepted.
For more details, please check https://jacg.github.io/nain4/explanation/boolean_solid_input_types.md
```

where `<METHOD>` indicates the method where the error occurred (which
might help in the case of chained operations) and `<TYPE_HINT>`
indicates the input type that gave rise to the error.

### `<TYPE_HINT> = a G4LogicalVolume`

The variable passed to the boolean-creation method is of type
`G4LogicalVolume*`. The most likely causes are:
1.+ [G4-style] this variable was assigned to `new G4LogicalVolume(...)`, e.g.
```c++
auto box2_solid = new G4Box(...);
auto box2_logic = new G4LogicalVolume(...);
box1.add(box2_logic); // WRONG
```

2. [n4-style] you called `.volume(...)` on the input solid, e.g.
```c++
box1.add(box2.volume(...)); // WRONG
```

The solution for 1. is to pass `box2_solid` instead of
`box2_logic`. The solution for 2. is to remove the call to
`volume(...)` or call `.solid()` instead.

### `<TYPE_HINT> = a G4PVPlacement`

The variable passed to the boolean-creation method is of type
`G4PVPlacement*`. The most likely causes are:
1.+ [G4-style] this variable was assigned to `new G4PVPlacement(...)`, e.g.
```c++
auto box2_solid = new G4Box(...);
auto box2_logic = new G4LogicalVolume(...);
auto box2_place = new G4PVPlacement(...);
box1.add(box2_place); // WRONG
```

2. [n4-style] you called `.place(...).now()` on the input solid, e.g.
```c++
box1.add(box2.place(...).other().methods().now()); // WRONG
```

The solution for 1. is to pass `box2_solid` instead of
`box2_place`. The solution for 2. is to remove the call to
`place(...).other().method().now()` or call `.solid()` instead.

### `<TYPE_HINT> = a n4::shape`

The variable passed to the boolean-creation method is of type
`n4::shape`. The most likely cause is that you called `.place(...)` on
the input solid, e.g.

```c++
box1.add(box2.place(...).other().methods()); // WRONG
```

The solution is to remove the call to `place(...).other().method()` or
call `.solid()` instead.

### `<TYPE_HINT> = an unknown type`

The type of the variable passed to the boolean-creation method is
neither a valid one (`n4::shape`, `G4VSolid*`) nor one of the specific
cases described above. In this case `nain4` cannot provide any
help. You must traceback the latest assignment of this variable and
understand what is its type. A LSP pluggin in you test editor might
help you.


## Requesting the handling of other cases

If you find that there is another case that `nain4` should handle,
please open an issue at `https://github.com/jacg/nain4/issues`!
