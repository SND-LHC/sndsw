#!/bin/bash
set -o errexit -o pipefail -o noclobber
source $4/config.sh "$@"
set -o nounset

# Iterate through every <run> automatically
echo "Reading the XML file: $xml"
for run in $(xmllint --xpath 'count(/runlist/runs/run)' "${xml}"); do :; done  # pre-check count

n=$(xmllint --xpath 'count(/runlist/runs/run)' "${xml}")
for i in $(seq 1 $n) 
do
    run_number=$(xmllint --xpath "string(/runlist/runs/run[$i]/run_number)" "${xml}")
    start_year=$(xmllint --xpath "string(/runlist/runs/run[$i]/start)" "${xml}")
    year=${start_year:0:4}
    end=$(xmllint --xpath "string(/runlist/runs/run[$i]/end)" "${xml}")
    n_events=$(xmllint --xpath "string(/runlist/runs/run[$i]/n_events)" "${xml}")
    n_files=$(xmllint --xpath "string(/runlist/runs/run[$i]/n_files)" "${xml}")
    n_files=$((n_files-1))  # to make the seq 0 based
    path=$(xmllint --xpath "string(/runlist/runs/run[$i]/path)" "${xml}")
    geo_file=$( root -l -b -q -e '.L sndGeometryGetter.cxx+' -e "std::string csv=std::string(gSystem->Getenv(\"SNDSW_ROOT\"))+\"/analysis/tools/geo_paths.csv\"; \
                std::cout << snd::analysis_tools::GetGeoPath(csv, ${run_number}) << std::endl;" | tail -n 1 )

    for j in $(seq 0 "$n_files")
    do
        js=$(printf "%03d" $((j)))
        neutrinoFilterGoldenSample ${path}/sndsw_raw-0${js}.root filtered_MC_00${run_number}_${j}.root ${cutset}
        python3 $SNDSW_ROOT/shipLHC/run_muonRecoSND.py -f filtered_MC_00${run_number}_${j}.root -g ${geo_file} -c passing_mu_DS -sc 1 -s ./ -hf linearSlopeIntercept -o
        python3 ${SNDSW_ROOT}/analysis/neutrinoFilterGoldenSample_stage2.py -f ${run_number}/filtered_MC_00${run_number}_${j}.root -t filtered_MC_00${run_number}_${j}__muonReco.root -o filtered_MC_00${run_number}_${j}_stage2_noscifi2.root -g ${geo_file}; 
    done
    
    mkdir -p ${output}/${run_number}/
    xrdcp -f ./filtered_MC_00${run_number}_*.root ${output}/${run_number}/
    xrdcp -f ./filtered_MC_00${run_number}_*__muonReco.root ${output}/${run_number}/
    xrdcp -f ./filtered_MC_00${run_number}_*_stage2_noscifi2.root ${output}/${run_number}/
    rm -f ./filtered_MC_00${run_number}_*.root
    rm -f ./filtered_MC_00${run_number}_*__muonReco.root
    rm -f ./filtered_MC_00${run_number}_*_stage2_noscifi2.root
    echo "Finished processing run number $run_number"
done