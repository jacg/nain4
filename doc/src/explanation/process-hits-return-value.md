# Return value of G4VSensitiveDetector::ProcessHits

`G4VSensitiveDetector`'s method `ProcessHits` *must* be implemented by the user. `nain4` provides `n4::sensitive_detector` to ease implementing `G4VSensitiveDetector` subclasses. Because `ProcessHits` is mandatory, `nain4` obliges the user to provide an implementation as a construction argument to `n4::sensitive_detector`.

The aforementioned method returns `bool` for historical reasons. (This seems to be undocumented, but see [this Geant4 forum comment](https://geant4-forum.web.cern.ch/t/return-value-of-g4vsensitivedetector-processhits/1993/2).)

The summary is:

+ The actual value returned doesn't matter: it is ignored by G4.
+ One day the Geant4 developers may decide to change the return type to `void` at which point old code will give compilation errors.
