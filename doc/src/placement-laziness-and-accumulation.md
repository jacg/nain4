# Placement: Laziness and Accumulation

The auxiliary placement methods belong to an object of type `n4::place`.
Instances of this type can accumulate state.

This can be useful for reducing repetition: compare the following two code
samples, which produce identical results

```c++
// Store complicated configuration
auto place_box = n4::box("box").cube(1*m).place(gold).in(world).at_x(1*m);
// Reuse it many times, emphasizing what differs: rot_z(angle) and copy_no(N)
place_box.clone().rot_z(23*deg).copy_no(1).now();
place_box.clone().rot_z(54*deg).copy_no(2).now();
place_box.clone().rot_z(91*deg).copy_no(3).now();
```
```c++
// Repeat complicated configuration explicitly: signal lost in the noise 
n4::box("box").cube(1*m).place(gold).in(world).at_x(1*m).copy_no(1).rot_z(23*deg).now();
n4::box("box").cube(1*m).place(gold).in(world).at_x(1*m).copy_no(2).rot_z(54*deg).now();
n4::box("box").cube(1*m).place(gold).in(world).at_x(1*m).copy_no(3).rot_z(91*deg).now();
```

However, note the presence of `.clone()` on each line of the first sample.
Without it, the effect of each call to `rot_z` would be accumulated in
`place_box`

```c++
place_box = n4::box("box").cube(1*m).place(copper).in(world);

// Place boxes at 1, 2 and 3 metres
for (auto k : {1,2,3}) { place_box.clone().at_x(k*m).now(); }
// Place boxes at 1, 3 and 6 metres
for (auto k : {1,2,3}) { place_box        .at_x(k*m).now(); }
```
