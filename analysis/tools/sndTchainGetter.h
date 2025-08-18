#ifndef SND_TCHAIN_GETTER
#define SND_TCHAIN_GETTER

#include <string>
#include <memory>

#include "TChain.h"

namespace snd {
    namespace analysis_tools {
        std::unique_ptr<TChain> GetTChain(const std::string& csv_file_path, int run_number, int n_files = -1);  
        std::unique_ptr<TChain> GetTChain(const std::string& file_name);
        std::string GetDataBasePath(const std::string& csv_file_path, int run_number); 
    }
}

#endif
