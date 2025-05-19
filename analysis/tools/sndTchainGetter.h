#ifndef SND_TCHAIN_GETTER
#define SND_TCHAIN_GETTER

#include <string>

#include "TChain.h"

namespace snd {
    namespace analysis_tools {
        TChain* GetTChain(int run_number, int n_files = -1);  
        TChain* GetTChain(std::string file_name); 
    }
}

#endif
