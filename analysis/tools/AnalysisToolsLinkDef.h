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
#pragma link C++ function snd::analysis_tools::peakScifiHits(const TClonesArray &, int, float, float);
#pragma link C++ function snd::analysis_tools::getScifiHits(const TClonesArray &, int, bool);
#pragma link C++ function snd::analysis_tools::selectScifiHits(const TClonesArray &, int, bool, int, float, float, float, float, bool);
#pragma link C++ function snd::analysis_tools::selectScifiHits(const TClonesArray &, int, bool, const std::vector<float> &, bool);
#pragma link C++ function snd::analysis_tools::filterScifiHits(const TClonesArray &, std::vector<float> &, int, std::string);
#pragma link C++ function snd::analysis_tools::filterScifiHits(const TClonesArray &, int, std::string);
#pragma link C++ function snd::analysis_tools::densityScifi(int,const TClonesArray &, int, int, bool);
#pragma link C++ function snd::analysis_tools::densityCheck(const TClonesArray &, int, int, int, bool);
#pragma link C++ function snd::analysis_tools::showerInteractionBlock(const TClonesArray &, std::vector<float> &, int, std::string);
#pragma link C++ function snd::analysis_tools::showerInteractionBlock(const TClonesArray &, int, std::string);

#endif
