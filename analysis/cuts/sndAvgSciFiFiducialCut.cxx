#include "sndAvgSciFiFiducialCut.h"

#include "TChain.h"

namespace snd::analysis_cuts {
  avgSciFiFiducialCut::avgSciFiFiducialCut(double vertical_min_cut, double vertical_max_cut, double horizontal_min_cut, double horizontal_max_cut, TChain * ch, bool reverseCuts) : sciFiBaseCut(ch){

    reversed = reverseCuts;

    vertical_min = vertical_min_cut;
    vertical_max = vertical_max_cut;
    horizontal_min = horizontal_min_cut;
    horizontal_max = horizontal_max_cut;
    
    cutName = "Avg SciFi Ver channel in ["+std::to_string(vertical_min)+","+std::to_string(vertical_max)+"] Hor in ["+std::to_string(horizontal_min)+","+std::to_string(horizontal_max)+"]";

    shortName = "AvgSFChan";
    nbins = std::vector<int>{128*2, 128*2};
    range_start = std::vector<double>{0, 0};
    range_end = std::vector<double>{128*12, 128*12};
    plot_var = std::vector<double>{-1, -1};
  }

  bool avgSciFiFiducialCut::passCut(){
    initializeEvent();
    
    double avg_ver = 0.;
    unsigned int n_ver = 0;
    double avg_hor = 0.;
    unsigned int n_hor = 0;

    sndScifiHit * hit;
    TIter hitIterator(scifiDigiHitCollection);

    while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
      if (hit->isValid()){
	int mat = hit->GetMat();
	int sipm = hit->GetSiPM();
	int channel = hit->GetSiPMChan();

	int x = channel + sipm*128 + mat*4*128;

	if (hit->isVertical()){
	  avg_ver += x;
	  n_ver++;
	} else {
	  avg_hor += x;
	  n_hor++;
	}
      }
    }
    
    if ((n_ver+n_hor) == 0) {
      plot_var[0] = -1;
      plot_var[1] = -1;
      return false;
    }
    
    if (n_ver) {
      avg_ver /= n_ver;
      plot_var[0] = avg_ver;
    } else {
      plot_var[0] = -1;
    }

    if (n_hor) {
      avg_hor /= n_hor;
      plot_var[1] = avg_hor;
    } else {
      plot_var[1] = -1;
    }

    if (n_ver == 0) return false;
    if (n_hor == 0) return false;

    if (not reversed) {
      if (avg_hor < horizontal_min) return false;
      if (avg_hor > horizontal_max) return false;
      if (avg_ver < vertical_min) return false;
      if (avg_ver > vertical_max) return false;
    } else {
      if ((avg_hor > horizontal_min) and (avg_hor < horizontal_max)) return false;
      if ((avg_ver > vertical_min) and (avg_ver < vertical_max)) return false;
    }
    return true;
  }
}	     
