#pragma once

#include <G4VisAttributes.hh>

namespace nain4 {

// keyword-argument construction of G4VisAttributes

// TODO: G4VisAttributes does NOT have virtual destructor, do we get UB by subclassing?
class vis_attributes : public G4VisAttributes {
public:
  using G4VisAttributes::G4VisAttributes; // Inherit ALL constructors
  using MAP = std::map<G4String, G4AttDef>;
#define FORWARD(NEW_NAME, TYPE, OLD_NAME) vis_attributes& NEW_NAME(TYPE v){ this->OLD_NAME(v); return *this; }
  FORWARD(visible                       , bool     , SetVisibility)
  FORWARD(daughters_invisible           , bool     , SetDaughtersInvisible)
  FORWARD(colour                        , G4Colour , SetColour)
  FORWARD(color                         , G4Color  , SetColor)
  FORWARD(line_style                    , LineStyle, SetLineStyle)
  FORWARD(line_width                    , G4double , SetLineWidth)
  FORWARD(force_wireframe               , bool     , SetForceWireframe)
  FORWARD(force_solid                   , bool     , SetForceSolid)
  FORWARD(force_aux_edge_visible        , bool     , SetForceAuxEdgeVisible)
  FORWARD(force_line_segments_per_circle, G4int    , SetForceLineSegmentsPerCircle)
  FORWARD(start_time                    , G4double , SetStartTime)
  FORWARD(  end_time                    , G4double , SetEndTime)
  FORWARD(att_values, const std::vector<G4AttValue>*, SetAttValues)
  FORWARD(att_defs  , const                     MAP*, SetAttDefs)
#undef FORWARD
};

}; // namespace nain4

namespace n4 { using namespace nain4; }
