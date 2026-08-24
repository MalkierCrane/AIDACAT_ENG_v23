// This includes the Coefficients class declaration, the math function for absolute value, and console output for the warning
#include "Coefficients.h"
#include <cmath>
#include <iostream>

using namespace std;

// This function creates one weight profile from 8 numbers, so that each field doesn't need to be written separately for each profile in the constructor
WeightProfile Coefficients::makeProfile(string name, double wp, double wr, double wf1, double wb, double wm, double wrl, double wc, double ws) {

    WeightProfile profile;
    profile.name = name;

    profile.wPrecision = wp;
    profile.wRecall = wr;
    profile.wF1 = wf1;
    profile.wBleu = wb;
    profile.wMeteor = wm;
    profile.wRougeL = wrl;
    profile.wCider = wc;
    profile.wSemantic = ws;

    warnIfWeightsDontSumToOne(profile);

    return profile;
}

/* This function checks whether the profile's weights sum to close to 1.0, and warns if not. This does not stop the program, 
since this is just a self-help aid so as not to miss a typo in the profile numbers */
void Coefficients::warnIfWeightsDontSumToOne(WeightProfile profile) {

    double sum = profile.wPrecision + profile.wRecall + profile.wF1 + profile.wBleu +
                 profile.wMeteor + profile.wRougeL + profile.wCider + profile.wSemantic;

    double diff = fabs(sum - 1.0);

    // If the difference is too large, I print a warning to the console, but do not stop the program
    if (diff > 0.001) {
        cout << "Warning: profile '" << profile.name << "' weight sum is " << sum << ", not 1.0" << endl;
    }
}

/* The constructor prepares 4 initial weight profiles. Only the "informative" profile is later adjusted using calibrate mode - the rest are reasoned,
but unverified assumptions, based on the W3C WAI alt-text decision tree. */
Coefficients::Coefficients() {

    // For an informative image, precision and completeness are important, so precision/recall/semantic are high
    profiles.push_back(makeProfile("informative", 0.13, 0.13, 0.09, 0.13, 0.17, 0.13, 0.09, 0.13));

    /* For a decorative image, the alt text should usually be empty, so the semantic and precision weight is higher,
    to penalize unnecessary, superfluous descriptions */
    profiles.push_back(makeProfile("decorative", 0.18, 0.10, 0.10, 0.10, 0.12, 0.10, 0.10, 0.20));

    // For a functional image (button, link), the action that the image performs is important, so the semantic weight is the highest
    profiles.push_back(makeProfile("functional", 0.20, 0.08, 0.10, 0.10, 0.12, 0.10, 0.05, 0.25));

    // For a complex image (chart, diagram), recall is important - whether the description covers all the content
    profiles.push_back(makeProfile("complex", 0.10, 0.20, 0.10, 0.10, 0.15, 0.10, 0.15, 0.10));
}

// This function finds the profile by name
WeightProfile Coefficients::getProfile(string purposeName) {

    for (int i = 0; i < (int)profiles.size(); i++) {
        if (profiles[i].name == purposeName) {
            return profiles[i];
        }
    }

    // If the profile is not found (for example, purpose="n/a" or a typo), I return the default profile
    return getDefaultProfile();
}

/* The function returns the default profile, used in experiment and calibrate modes, where the image's purpose is not determined.
I take the first one in the list, rather than searching by name again, to avoid infinite recursion if someone accidentally deleted the "informative" profile. */
WeightProfile Coefficients::getDefaultProfile() {

    WeightProfile fallback = makeProfile("informative", 0.13, 0.13, 0.09, 0.13, 0.17, 0.13, 0.09, 0.13);

    if (profiles.size() == 0) {
        return fallback;
    }

    for (int i = 0; i < (int)profiles.size(); i++) {
        if (profiles[i].name == "informative") {
            return profiles[i];
        }
    }

    // If there is no "informative" profile, I return the first available profile
    return profiles[0];
}
