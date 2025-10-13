#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ class snd::Configuration+;
#pragma link C++ class snd::analysis_tools::ScifiPlane+;
#pragma link C++ class snd::analysis_tools::USPlane+;

#pragma link C++ namespace snd::analysis_tools;
#pragma link C++ defined_in namespace snd::analysis_tools;

#pragma link C++ function snd::analysis_tools::getSciFiHitsPerStation(TClonesArray *, std::vector<int> &, std::vector<int> &);
#pragma link C++ function snd::analysis_tools::getTotalSciFiHits(std::vector<int> &, std::vector<int>);
#pragma link C++ function snd::analysis_tools::getTotalSciFiHits(TClonesArray *);
#pragma link C++ function snd::analysis_tools::getFractionalHitsPerScifiPlane(std::vector<int> &, std::vector<int> &);
#pragma link C++ function snd::analysis_tools::getFractionalHitsPerScifiPlane(TClonesArray *);
#pragma link C++ function snd::analysis_tools::findScifiStation(std::vector<int> &, std::vector<int> &, float);
#pragma link C++ function snd::analysis_tools::findScifiStation(TClonesArray *, float);
#pragma link C++ function snd::analysis_tools::peakScifiHits(const TClonesArray &, int, float, float, bool);
#pragma link C++ function snd::analysis_tools::getScifiHits(const TClonesArray &, int, bool);
#pragma link C++ function snd::analysis_tools::selectScifiHits(const TClonesArray &, int, bool, int, float, float, float, float, bool, bool);
#pragma link C++ function snd::analysis_tools::selectScifiHits(const TClonesArray &, int, bool, const std::map<std::string, float> &, bool, bool);
#pragma link C++ function snd::analysis_tools::filterScifiHits(const TClonesArray &, std::map<std::string, float> &, int, std::string, bool);
#pragma link C++ function snd::analysis_tools::filterScifiHits(const TClonesArray &, int, std::string, bool);
#pragma link C++ function snd::analysis_tools::calculateSiPMNumber(int);
#pragma link C++ function snd::analysis_tools::densityScifi(int,const TClonesArray &, int, int, bool);
#pragma link C++ function snd::analysis_tools::densityCheck(const TClonesArray &, int, int, int, bool);
#pragma link C++ function snd::analysis_tools::showerInteractionWall(const TClonesArray &,const std::map<std::string, float> &, int, std::string);
#pragma link C++ function snd::analysis_tools::showerInteractionWall(const TClonesArray &, int, std::string);
#pragma link C++ function snd::analysis_tools::GetDataBasePath(const std::string &, int);
#pragma link C++ function snd::analysis_tools::GetTChain(const std::string &, int, int);
#pragma link C++ function snd::analysis_tools::GetTChain(const std::string &);
#pragma link C++ function snd::analysis_tools::GetGeoPath(const std::string &,int);
#pragma link C++ function snd::analysis_tools::GetGeometry(const std::string &);
#pragma link C++ function snd::analysis_tools::FillScifi(const snd::Configuration &, TClonesArray *, Scifi *);
#pragma link C++ function snd::analysis_tools::FillUS(const snd::Configuration &, TClonesArray *, MuFilter *);
#pragma link C++ function snd::analysis_tools::GetScifiShowerStart(const std::vector<snd::analysis_tools::ScifiPlane &);
#pragma link C++ function snd::analysis_tools::GetScifiShowerEnd(const std::vector<snd::analysis_tools::ScifiPlane &);
#pragma link C++ function snd::analysis_tools::GetUSShowerStart(const std::vector<snd::analysis_tools::USPlane> &);
#pragma link C++ function snd::analysis_tools::GetUSShowerEnd(const std::vector<snd::analysis_tools::USPlane> &);
#pragma link C++ function snd::analysis_tools::GetShowerInterceptAndDirection(const snd::Configuration &, const std::vector<snd::analysis_tools::ScifiPlane> &, const std::vector<snd::analysis_tools::USPlane> &);
#pragma link C++ function snd::analysis_tools::GetShoweringPlanes(const std::vector<snd::analysis_tools::ScifiPlane> &, const std::vector<snd::analysis_tools::USPlane> &);

#endif
