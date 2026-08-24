#ifndef COEFFICIENTS_H
#define COEFFICIENTS_H

// This includes the common structures, including WeightProfile, the string type and the vector type
#include "Types.h"
#include <string>
#include <vector>

using namespace std;

/* This class stores the list of named weight profiles. Each image purpose type (decorative/informative/functional/complex) has its own
weight profile, because, for example, for a decorative image a good result means a short or empty description, but for an informative image a good result means an accurate and complete description.
Only the "informative" profile is calibrated against the ExpertAnnotations.txt data (see the Calibration class) - the other three profiles are reasoned,
but not empirically verified, because the Flickr8k data has no labeling by image purpose. */
class Coefficients {
private:
    WeightProfile makeProfile(string name, double wp, double wr, double wf1, double wb, double wm, double wrl, double wc, double ws);
    // I check whether the profile's 8 weights sum to close to 1.0, and warn in the console if not (but do not stop).
    void warnIfWeightsDontSumToOne(WeightProfile profile);

public:
    vector<WeightProfile> profiles; // Here are all the known weight profiles

    Coefficients();                               // Constructor prepares 4 initial profiles
    WeightProfile getProfile(string purposeName); // Finds a profile by name, defaults to "informative"
    WeightProfile getDefaultProfile();            // Returns the "informative" profile
};

#endif
