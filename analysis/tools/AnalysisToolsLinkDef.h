#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;
#pragma link C++ nestedfunctions;

#pragma link C++ namespace snd::analysis_tools;
#pragma link C++ defined_in namespace snd::analysis_tools;

#pragma link C++ function snd::analysis_tools::getSciFiHitsPerStation(TClonesArray *, std::vector<int> &, std::vector<int> &);
#pragma link C++ function snd::analysis_tools::getTotalSciFiHits(std::vector<int> &, std::vector<int>);
#pragma link C++ function snd::analysis_tools::getTotalSciFiHits(TClonesArray *);
#pragma link C++ function snd::analysis_tools::getFractionalHitsPerScifiPlane(std::vector<int> &, std::vector<int> &);
#pragma link C++ function snd::analysis_tools::getFractionalHitsPerScifiPlane(TClonesArray *);
#pragma link C++ function snd::analysis_tools::findScifiStation(std::vector<int> &, std::vector<int> &, float);
#pragma link C++ function snd::analysis_tools::findScifiStation(TClonesArray *, float);
#pragma link C++ function snd::analysis_tools::peakScifiHits(TClonesArray *, int, double, double);
#pragma link C++ function snd::analysis_tools::getScifiHits(TClonesArray *, int, bool);
#pragma link C++ function snd::analysis_tools::getScifiHits(TClonesArray *, int, bool, float, bool);
#pragma link C++ function snd::analysis_tools::filterScifiHits(int, float);
#pragma link C++ function snd::analysis_tools::filterScifiHits(TClonesArray, int, float, std::string);
#pragma link C++ function snd::analysis_tools::densityScifi(int, TClonesArray *, int, int, bool);
#pragma link C++ function snd::analysis_tools::densityCheck(TClonesArray *, int, int, bool);

#endif
