#include "testing.hh"

#include <catch2/generators/catch_generators.hpp>

TEST_CASE("nain material", "[nain][material]") {

  // nain4::material finds the same materials as the verbose G4 style
  SECTION("material NIST") {
    auto material_name = GENERATE("G4_AIR", "G4_WATER", "G4_H", "G4_A-150_TISSUE");
    auto nain_material = nain4::material(material_name);
    auto nist_material = G4NistManager::Instance()->FindOrBuildMaterial(material_name);
    REQUIRE(nain_material == nist_material);
    REQUIRE(nain_material != nullptr);
  }

  // Basic material properties make sense (except for solid water at RTP!)
  SECTION("material properties") {
    SECTION("water") {
      auto water = nain4::material("G4_WATER");
      CHECK     (water->GetName()                  == "G4_WATER");
      CHECK     (water->GetChemicalFormula()       == "H_2O");
      CHECK     (water->GetState()                 == G4State::kStateLiquid);
      CHECK_THAT(water->GetTemperature() /  kelvin, Within1ULP( 293.15));
      CHECK_THAT(water->GetPressure() / atmosphere, Within1ULP(   1.  ));
      CHECK_THAT(water->GetDensity() /     (kg/m3), Within1ULP(1000.  ));
    }
  }

  // Making and retrieving materials with nain4
  SECTION("material creation from N atoms") {

    // The values used to construct the material
    auto name = "n4test_FR4";
    auto density = 1.85 * g/cm3;
    auto state = kStateSolid;
    auto [nH, nC, nO] = std::make_tuple(12, 18, 3);

    // Grab elements and calculate some properties for use in tests lower down
    auto H = nain4::element("H"); auto mH = H->GetAtomicMassAmu();
    auto C = nain4::element("C"); auto mC = C->GetAtomicMassAmu();
    auto O = nain4::element("O"); auto mO = O->GetAtomicMassAmu();
    auto total_mass = nH*mH + nC*mC + nO*mO;

    // Make the material using nain4::material_from_elements
    auto fr4 = nain4::material_from_elements_N(name, density, {.state=state},
                                               {{"H", nH}, {"C", nC}, {O, nO}});
    CHECK(fr4 != nullptr);

    // Verify that the material can be retrieved with nain4::material
    auto fr4_found = nain4::material(name);
    CHECK(fr4 == fr4_found);

    // Elements used correctly?
    CHECK(fr4 -> GetElement(0) == H);
    CHECK(fr4 -> GetElement(1) == C);
    CHECK(fr4 -> GetElement(2) == O);

    // Correct number of each element?
    auto atoms = fr4 -> GetAtomsVector();
    CHECK(atoms[0] == nH);
    CHECK(atoms[1] == nC);
    CHECK(atoms[2] == nO);

    // Basic properties set corretly?
    CHECK     (fr4 -> GetNumberOfElements() == 3);
    CHECK     (fr4 -> GetState()            == state);
    CHECK_THAT(fr4 -> GetDensity()          ,  Within1ULP(density));

    // Fractional composition correct?
    auto fracs = fr4 -> GetFractionVector();
    CHECK_THAT(fracs[0], Within1ULP(nH*mH / total_mass));
    CHECK_THAT(fracs[1], Within1ULP(nC*mC / total_mass));
    CHECK_THAT(fracs[2], Within1ULP(nO*mO / total_mass));

    // Does fractional composition sum to 1?
    CHECK_THAT(std::accumulate(fracs, fracs + fr4->GetNumberOfElements(), 0.0), Within1ULP(1));
  }

  // Making and retrieving materials with nain4
  SECTION("material creation from mass fractions") {

    // The values used to construct the material
    auto name = "n4test_LYSO";
    auto density = 7.1 * g/cm3;
    auto state = kStateSolid;
    auto [fLu, fY, fSi, fO] = std::make_tuple(0.714, 0.040, 0.064, 0.182);

    // Make the material using nain4::material_from_elements
    auto lyso = nain4::material_from_elements_F(name, density, {.state=state},
                                                {{"Lu", fLu}, {"Y", fY}, {"Si", fSi}, {"O", fO}});
    CHECK(lyso != nullptr);

    // Verify that the material can be retrieved with nain4::material
    auto fr4_found = nain4::material(name);
    CHECK(lyso == fr4_found);

    // Grab elements and calculate some properties for use in tests lower down
    auto Lu = nain4::element("Lu");
    auto Y  = nain4::element("Y" );
    auto Si = nain4::element("Si");
    auto O  = nain4::element("O" );
    //auto total_mass = nH*mH + nC*mC + nO*mO;

    // Elements used correctly?
    CHECK(lyso -> GetElement(0) == Lu);
    CHECK(lyso -> GetElement(1) == Y );
    CHECK(lyso -> GetElement(2) == Si);
    CHECK(lyso -> GetElement(3) == O );

    // Atom counts produce nonsense when material built with mass fractions
    auto atoms = lyso -> GetAtomsVector();
    CHECK(atoms[0] == 1);
    CHECK(atoms[1] == 0);
    CHECK(atoms[2] == 0);
    CHECK(atoms[3] == 2);

    // Basic properties set corretly?
    CHECK     (lyso -> GetNumberOfElements() == 4);
    CHECK     (lyso -> GetState()            == state);
    CHECK_THAT(lyso -> GetDensity()          ,  Within1ULP(density));

    // Fractional composition correct?
    auto fracs = lyso -> GetFractionVector();
    CHECK(fracs[0] == fLu);
    CHECK(fracs[1] == fY );
    CHECK(fracs[2] == fSi);
    CHECK(fracs[3] == fO );

    // Does fractional composition sum to 1?
    CHECK_THAT(std::accumulate(fracs, fracs + lyso->GetNumberOfElements(), 0.0), Within1ULP(1));

  }
}

TEST_CASE("nain material_properties", "[nain][material_properties]") {
  SECTION("add") {
    auto key_1         = "RINDEX";
    auto key_2         = "ABSLENGTH";
    auto key_3         = "SCINTILLATIONTIMECONSTANT1";

    auto energies_1    = std::vector<G4double>{1 , 4 , 6};
    auto energies_2    = std::vector<G4double>{2 ,     7};

    auto   values_1    = std::vector<G4double>{3., 5., 8.};
    auto const_value_2 = CLHEP::pi;
    auto const_value_3 = 42.0;

    auto mp = n4::material_properties()
      .add(key_1, energies_1,      values_1) // vec vec
      .add(key_2, energies_2, const_value_2) // vec const
      .add(key_3,             const_value_3) //     const
      .done();

    auto property_1 = mp -> GetProperty(key_1);
    for (auto i=0; i<3; i++) {
      auto e1 = energies_1[i];
      CHECK_THAT(property_1 -> Energy(i) , Within1ULP(         e1));
      CHECK_THAT(property_1 -> Value (e1), Within1ULP(values_1[i]));
    }

    auto property_2 = mp -> GetProperty(key_2);
    for (auto i=0; i<2; i++){
      auto e2 = energies_2[i];
      CHECK_THAT(property_2 -> Energy(i) , Within1ULP(           e2));
      CHECK_THAT(property_2 -> Value (e2), Within1ULP(const_value_2));
    }

    CHECK     (mp ->    ConstPropertyExists(key_3));
    CHECK_THAT(mp -> GetConstProperty      (key_3), Within1ULP(const_value_3));
  }

  SECTION("initializer lists") {
    auto key      = "RINDEX";
    auto energies = {3., 4., 5.};
    auto values   = {6., 7., 8.};
    auto mp       = n4::material_properties() .add(key, energies, values) .done();

    auto property = mp -> GetProperty("RINDEX");
    for (auto i=0; i<3; i++) {
      auto e = 3 + i;
      CHECK_THAT(property -> Energy(i), Within1ULP(3 + i));
      CHECK_THAT(property -> Value (e), Within1ULP(6 + i));
    }
  }

  SECTION("NEW") {
    auto key_1 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_VEC_VEC";
    auto key_2 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_VEC_CONST";
    auto key_3 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_CONST";

    auto energies_1 = std::vector<G4double>{1., 3., 4.};
    auto energies_2 = std::vector<G4double>{2.,     5.};

    auto      values_1 = std::vector<G4double>{7., 8., 9.};
    auto const_value_2 = CLHEP::pi;
    auto const_value_3 = 42.;

    auto mp = n4::material_properties()
      .NEW(key_1, energies_1,      values_1)
      .NEW(key_2, energies_2, const_value_2)
      .NEW(key_3,             const_value_3)
      .done();

    auto property_1 = mp -> GetProperty(key_1);
    for (auto i=0; i<3; i++){
      auto e1 = energies_1[i];
      CHECK_THAT(property_1 -> Energy(i) , Within1ULP(         e1));
      CHECK_THAT(property_1 -> Value (e1), Within1ULP(values_1[i]));
    }

    auto property_2 = mp -> GetProperty(key_2);
    for (auto i=0; i<2; i++){
      auto e2 = energies_2[i];
      CHECK_THAT(property_2 -> Energy(i) , Within1ULP(           e2));
      CHECK_THAT(property_2 -> Value (e2), Within1ULP(const_value_2));
    }

    CHECK     (mp ->    ConstPropertyExists(key_3));
    CHECK_THAT(mp -> GetConstProperty      (key_3), Within1ULP(const_value_3));
  }


  SECTION("copy another mpt's values by hand") {
    auto key_1 = "RINDEX";
    auto key_2 = "ABSLENGTH";
    auto key_3 = "SCINTILLATIONTIMECONSTANT1";
    auto key_4 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_VEC_VEC";
    auto key_5 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_VEC_CONST";
    auto key_6 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_CONST";

    auto energies_1 = std::vector<G4double>{1., 5.,  7.};
    auto energies_2 = std::vector<G4double>{2.,      8.};
    auto energies_4 = std::vector<G4double>{3., 6.,  9.};
    auto energies_5 = std::vector<G4double>{4.,     10.};

    auto      values_1 = std::vector<G4double>{11., 12., 13.};
    auto const_value_2 = CLHEP::pi;
    auto const_value_3 = 42.;
    auto      values_4 = std::vector<G4double>{14., 15., 16.};
    auto const_value_5 = CLHEP::twopi;
    auto const_value_6 = 42. * 42.;

    auto mp1 = n4::material_properties()
      .add(key_1, energies_1,      values_1)
      .add(key_2, energies_2, const_value_2)
      .add(key_3,             const_value_3)
      .NEW(key_4, energies_4,      values_4)
      .NEW(key_5, energies_5, const_value_5)
      .NEW(key_6,             const_value_6)
      .done();

    auto mp2 = n4::material_properties()
      .add(key_1, mp1 -> GetProperty(key_1))
      .add(key_2, mp1 -> GetProperty(key_2))
      .add(key_3, mp1 -> GetConstProperty(key_3)) // Must use Const!!!!
      .NEW(key_4, mp1 -> GetProperty(key_4))
      .NEW(key_5, mp1 -> GetProperty(key_5))
      .NEW(key_6, mp1 -> GetConstProperty(key_6)) // Must use Const!!!!
      .done();

    CHECK( mp2 -> GetProperty(key_1)  ==  mp1 -> GetProperty(key_1) );
    CHECK( mp2 -> GetProperty(key_2)  ==  mp1 -> GetProperty(key_2) );
    CHECK( mp2 -> GetProperty(key_3)  ==  mp1 -> GetProperty(key_3) );
    CHECK( mp2 -> GetProperty(key_4)  ==  mp1 -> GetProperty(key_4) );
    CHECK( mp2 -> GetProperty(key_5)  ==  mp1 -> GetProperty(key_5) );
    CHECK( mp2 -> GetProperty(key_6)  ==  mp1 -> GetProperty(key_6) );
  }

  SECTION("copy another mpt's values by key") {
    auto key_1 = "RINDEX";
    auto key_2 = "ABSLENGTH";
    auto key_3 = "SCINTILLATIONTIMECONSTANT1";
    auto key_4 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_VEC_VEC";
    auto key_5 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_VEC_CONST";
    auto key_6 = "SOMETHING_THAT_DID_NOT_EXIST_BEFORE_CONST";

    auto energies_1 = std::vector<G4double>{1., 5.,  7.};
    auto energies_2 = std::vector<G4double>{2.,      8.};
    auto energies_4 = std::vector<G4double>{3., 6.,  9.};
    auto energies_5 = std::vector<G4double>{4.,     10.};

    auto      values_1 = std::vector<G4double>{11., 12., 13.};
    auto const_value_2 = CLHEP::pi;
    auto const_value_3 = 42.;
    auto      values_4 = std::vector<G4double>{14., 15., 16.};
    auto const_value_5 = CLHEP::twopi;
    auto const_value_6 = 42. * 42.;

    auto mp1 = n4::material_properties()
      .add(key_1, energies_1,      values_1)
      .add(key_2, energies_2, const_value_2)
      .add(key_3,             const_value_3)
      .NEW(key_4, energies_4,      values_4)
      .NEW(key_5, energies_5, const_value_5)
      .NEW(key_6,             const_value_6)
      .done();

    auto mp2 = n4::material_properties()
      .copy_from    (mp1,  key_1        ) // single key
      .copy_from    (mp1, {key_2, key_3}) // multiple keys
      .copy_NEW_from(mp1,  key_4        ) // single key
      .copy_NEW_from(mp1, {key_5, key_6}) // multiple keys
      .done();

    CHECK( mp2 -> GetProperty(key_1)  ==  mp1 -> GetProperty(key_1) );
    CHECK( mp2 -> GetProperty(key_2)  ==  mp1 -> GetProperty(key_2) );
    CHECK( mp2 -> GetProperty(key_3)  ==  mp1 -> GetProperty(key_3) );
    CHECK( mp2 -> GetProperty(key_4)  ==  mp1 -> GetProperty(key_4) );
    CHECK( mp2 -> GetProperty(key_5)  ==  mp1 -> GetProperty(key_5) );
    CHECK( mp2 -> GetProperty(key_6)  ==  mp1 -> GetProperty(key_6) );
  }
}
