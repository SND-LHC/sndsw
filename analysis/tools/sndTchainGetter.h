#ifndef SND_TCHAIN_GETTER
#define SND_TCHAIN_GETTER

#include <string>
#include <memory>

#include "TChain.h"

namespace snd {
    namespace analysis_tools {
        // With n_files == -1 all run partitions are added to the chain. 
        // With empty csv_file_path, it gets the official one from the sndsw installation
        std::unique_ptr<TChain> GetTChain(int run_number, int n_files = -1, const std::string& csv_file_path = "");  
        std::unique_ptr<TChain> GetTChain(const std::string& file_name);
        std::string GetDataBasePath(int run_number, std::string csv_file_path = ""); 
    }
}

#endif
