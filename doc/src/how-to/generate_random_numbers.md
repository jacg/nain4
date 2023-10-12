# How to generate random numbers

`nain4` offers a range of utilities to generate random numbers. This includes
scalars, tuples and 3-vectors. All these methods are declared in the
[header](../reference/headers.md) `<n4-random.hh>` which is transitively
included by `<n4-utils.hh>`. They are gathered in the namespace `nain4::random`
and exported to the namespaces `nain4` and `n4`.


## Scalars

The methods described in this section generate a single number.

### Continuous distributions

- `uniform()`: generates a random floating point number homogeneously
  distributed in the range [0, 1).
- `uniform(low, high)`: generates a random floating point number
  homogeneously distributed in the range [`low`, `high`).
- `uniform_half_width(hw)`: generates a random floating point number
  homogeneously distributed in the range [`-hw`, `hw`).
- `uniform_width(w)`: generates a random floating point number
  homogeneously distributed in the range [`-w/2`, `w/2`).

### Discrete distributions

- `biased_coin(p_true)`: generates a random boolean with probability
  `p_true` of being `true`.
- `fair_die(n_sides)`: generates a random unsigned integer in the
  range [0, `n_sides - 1`] with equal probability.
- `biased_choice(weights)`: provides a generator of unsigned integers
  in the range [0, `weights.size() - 1`] with given `weights`. The
  generator must be called to obtain a number.


## Tuples

- `random_in_disc(r)`: generates a pair of floating point numbers `{x, y}` that satisfy `x^2 + y^2 <= r^2`.

## 3-vectors

The methods described in this section generate `G4ThreeVector`s.

- `random_in_sphere(r)`: generates a vector of floating point numbers `{x, y, z}` that satisfy `x^2 + y^2 + z^2 <= r^2`.

### Directions

`nain4` provides the `direction` builder to help generate random directions. By
default this tool generates unit vectors distributed isotropically in 4π, but it
can be configured to restrict the generation to certain directions.

The general usage pattern is

```c++
auto generator = n4::random::direction{}.<optional extra specifications>;
auto one_random_unit_vector = generator.get();
```

The `<optional extra specifications>` are described below. `θ` (theta) is the
polar angle measured with respect to the positive z-axis and φ (phi) is the
azimuthal angle with `φ=0` in the plane of the positive x-axis and increasing
towards the positive y-axis, unless the axes are re-oriented with `.axis` or
`.????`.

The following specifiers (with hopefully self-explanatory names) restrict the angles of the generated directions

- `min_theta(<angle in radians>)` TODO what happens if you pass <0
- `max_theta(<angle in radians>)` TODO what happens if you pass >π
- `min_theta_deg(<angle in degrees>)` TODO missing
- `max_theta_deg(<angle in degrees>)` TODO missing
- `min_cos_theta(<ratio>)`
- `max_cos_theta(<ratio>)`

TODO what happens if max < min

- `min_phi(<angle in radians>)` TODO what is the domain and what happens if you give a value outside
- `max_phi(<angle in radians>)` TODO what is the domain and what happens if you give a value outside


- `axis`: TODO we should consider replacing this with `rot_{x,y,z}` etc.


  rotate the reference frame such that the z-axis is aligned
  with `axis`. θ will be measured with respect to the rotated z axis
  and φ with respect to the rotated x axis (which I don't know how
  it's defined. either x or y is `axis.cross(zaxis)`).


- `bidirectional()`: accept both the current selection and its
  reflection in the origin.

- `exclude()`: invert the selection. It generates directions that do
  not satisfy any of the selected criteria. Bear in mind that this
  method is more computationally expensive as it needs to generate
  directions until the result not rejected. Very restricted
  directionalities are discouraged.

  TODO: hmm, can we not do better than rejection sampling here?

#### Examples

- Isotropic emission: `n4::direction{}`

- Opening angle wrt the positive z axis: `n4::direction().max_theta(theta)`

- Opening angle wrt the negative x axis:
  + `n4::direction().max_theta(theta).axis(-1, 0, 0)`
  + `n4::direction().max_theta(theta).axis(g4_three_vec_with_above_components)`

- Only one octant:
  `n4::direction().min_cos_theta(0).min_phi(CLHEP::halfpi).max_phi(CLHEP::pi)`

- Back-to-back beams wrt the positive z axis with some opening angle:
  `n4::direction().max_theta(theta).bidirectional()`

- Isotropic except for some small angle around the negative z-axis:
  `n4::direction().min_theta(7*CLHEP::pi/8).exclude()`
