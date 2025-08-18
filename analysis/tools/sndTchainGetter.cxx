#include "sndTchainGetter.h"

#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <memory>

#include "TChain.h"
#include <TString.h>

std::string snd::analysis_tools::GetDataBasePath(const std::string& csv_file_path, int run_number) {
    std::ifstream file(csv_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open CSV file: " + csv_file_path);
    }

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string token;

        std::getline(ss, token, ',');
        int min_run = std::stoi(token);

        std::getline(ss, token, ',');
        int max_run = std::stoi(token);

        std::getline(ss, token);
        std::string path = token;

        if (run_number >= min_run && run_number <= max_run) {
            return path;
        }
    }
    throw std::runtime_error("Run number not found in CSV mapping.");
}

std::unique_ptr<TChain> snd::analysis_tools::GetTChain(const std::string& csv_file_path, int run_number, int n_files){
    std::string base_folder = GetDataBasePath(csv_file_path, run_number);
    auto tchain = std::make_unique<TChain>("rawConv");
    if (n_files == -1) {
        tchain->Add(Form("%srun_%06d/sndsw_raw-*", base_folder.c_str(), run_number));
    }
    else {
        for (int i = 0; i<n_files; ++i){
            tchain->Add(Form("%srun_%06d/sndsw_raw-%04d.root", base_folder.c_str(), run_number, i));
        }
    }
    return tchain;
};

std::unique_ptr<TChain> snd::analysis_tools::GetTChain(const std::string& file_name){
    auto tchain = std::make_unique<TChain>("rawConv");
    tchain->Add(file_name.c_str());
    return tchain;
};