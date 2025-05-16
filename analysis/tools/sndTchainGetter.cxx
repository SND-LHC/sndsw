#include "sndTchainGetter.h"

#include <string>
#include <stdexcept>

#include "TChain.h"

TChain* snd::analysis_tools::GetTChain(int run_number, int n_files){
    TChain* tchain = new TChain("rawConv");
    if (n_files == -1) {
        if (run_number < 5422) {
            tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2022/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 5421 && run_number < 7357) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2023/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 7648 && run_number < 8318) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_241/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 8317 && run_number < 8581) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_242/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 8582 && run_number < 8942) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_243/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 8941 && run_number < 9155) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_244/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 9156 && run_number < 9285) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_245/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 9285 && run_number < 9378) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_246/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 9378 && run_number < 9462) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_247/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 9462 && run_number < 9612) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_248/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 9612 && run_number < 9691) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_249/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 9691 && run_number < 9881) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_2410/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 9880 && run_number < 10012 ) {
        tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_2411/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 10011 && run_number < 10423) {
            tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_2412/run_%06d/sndsw_raw-*", run_number));
        }
        // TB runs 
        else if (run_number > 100237 && run_number < 100680 ){
            tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/commissioning/testbeam_June2023_H8/run_%06d/sndsw_raw-*", run_number));
        }
        else if (run_number > 100840 && run_number < 100986){
            tchain->Add(Form("root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/commissioning/testbeam_24/run_%06d/sndsw_raw-*", run_number));
        }
        else {
            throw std::runtime_error{"Run not found."};
        }
    }
    else {
        std::string base_folder="";
        if      (run_number < 5422) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2022/";
        }
        else if (run_number >  5421 && run_number <  7357) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2023/";
        }
        else if (run_number >  7648 && run_number <  8318) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_241/";
        }
        else if (run_number >  8317 && run_number <  8581) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_242/";
        }
        else if (run_number >  8582 && run_number <  8942) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_243/";
        }
        else if (run_number >  8941 && run_number <  9155) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_244/";   
        }
        else if (run_number >  9156 && run_number <  9285) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_245/";
        }
        else if (run_number >  9285 && run_number <  9378) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_246/";
        }
        else if (run_number >  9378 && run_number <  9462) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_247/";
        }
        else if (run_number >  9462 && run_number <  9612) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_248/";
        }
        else if (run_number >  9612 && run_number <  9691) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_249/";
        }
        else if (run_number >  9691 && run_number <  9881) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_2410/";
        }
        else if (run_number >  9880 && run_number < 10012) {
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_2411/";
        }
        else if (run_number > 10011 && run_number < 10423){
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/run_2412/";
        }
        //TB runs
        else if (run_number > 100237 && run_number < 100680 ){
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/commissioning/testbeam_June2023_H8/";
        }
        else if (run_number > 100840 && run_number < 100986){
            base_folder="root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/commissioning/testbeam_24/";
        }
        else {
            throw std::runtime_error{"Run not found."};
        }
        for (int i = 0; i<n_files; ++i){
            tchain->Add(Form("%srun_%06d/sndsw_raw-%04d.root", base_folder.c_str(), run_number, i));
        }
    }
    return tchain;
};

TChain* snd::analysis_tools::GetTChain(std::string file_name){
    TChain* tchain = new TChain("rawConv");
    tchain->Add(file_name.c_str());
    return tchain;
};