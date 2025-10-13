#ifndef SND_SHOWERTOOLS_H
#define SND_SHOWERTOOLS_H        

#include <vector>

#include "sndConfiguration.h"
#include "sndScifiPlane.h"
#include "sndUSPlane.h"
#include "Math/Vector3D.h"
#include "Math/Point3D.h"

namespace snd {
    namespace analysis_tools {
        // Returns first SciFi station with shower. If no shower is found in SciFi, returns -1
        int GetScifiShowerStart(const std::vector<ScifiPlane> &scifi_planes);
        // Returns last SciFi station with shower. If no shower is found in SciFi, returns -1
        int GetScifiShowerEnd(const std::vector<ScifiPlane> &scifi_planes);
        // Returns first US station with shower. If no shower is found in US, returns -1
        int GetUSShowerStart(const std::vector<USPlane> &us_planes);
        // Returns last US station with shower. If no shower is found in US, returns -1
        int GetUSShowerEnd(const std::vector<USPlane> &us_planes);
        // Returns a 3D reference point and the normalized direction of the shower fitting centroids in SciFi and US
        std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZVector> GetShowerInterceptAndDirection(const Configuration &configuration, const std::vector<ScifiPlane> &scifi_planes, const std::vector<USPlane> &us_planes);
        // Filters out non showering planes
        std::pair<std::vector<ScifiPlane>, std::vector<USPlane>> GetShoweringPlanes(const std::vector<ScifiPlane> &scifi_planes, const std::vector<USPlane> &us_planes);
    }
}

#endif