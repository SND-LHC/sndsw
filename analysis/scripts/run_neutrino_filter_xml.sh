#!/bin/bash

xml=$1
output=$2
mode=$3
cutset=$4

# Iterate through every <run> automatically
echo "Reading the XML file: $xml"
for run in $(xmllint --xpath 'count(/runlist/runs/run)' "${xml}"); do :; done  # pre-check count

mode_list=(STAGE1 RECO STAGE2)

if [[ ! ${mode_list[@]} =~ $mode ]]
then
    echo Mode $mode not available. It must be one of "${mode_list[*]}"
    echo Exitting.
    exit
fi

if [ -z ${SNDSW_ROOT+x} ]
then
    echo "Setting up SNDSW" 
    export ALIBUILD_WORK_DIR=/afs/cern.ch/user/s/schuetha/work/public/data_work_flow/sw
    source /cvmfs/sndlhc.cern.ch/SNDLHC-2025/Jan30/setUp.sh  # recommended latest version
    eval `alienv -a slc9_x86-64 load --no-refresh sndsw/master-local1`
    echo "Finished setting up SNDSW env"
    export EOSSHIP=root://eosuser.cern.ch
fi

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

    if [[ "$year" == "2022" || "$year" == "2023" ]]; then
        geo_file="/eos/experiment/sndlhc/legacy_geofiles/2023/geofile_sndlhc_TI18_V4_2023.root"
    else
        echo "May need other Geofile year than 2023 Legacy. Exitting."
    fi 

    if [ "$mode" == "STAGE1" ] 
    then
        for j in $(seq 0 "$n_files")
        do
            if [ -f "${output}/${run_number}/${mode}/filtered_MC_00${run_number}_${j}.root" ]
            then
                echo "File ${output}/${run_number}/${mode}/filtered_MC_00${run_number}_${j}.root exists. Skipping."
                continue
	        fi
            js=$(printf "%03d" $((j)))
            neutrinoFilterGoldenSample ${path}/sndsw_raw-0${js}.root filtered_MC_00${run_number}_${j}.root ${cutset}
            mkdir -p ${output}/${run_number}/${mode}/
            xrdcp -f ./filtered_MC_00${run_number}_${j}.root ${output}/${run_number}/${mode}/
            rm -f ./filtered_MC_00${run_number}_${j}.root
        done
    
    elif [ "$mode" == "RECO" ] 
    then
        for j in $(seq 0 "$n_files")
        do
            if [ -f "${output}/${run_number}/${mode}/filtered_MC_00${run_number}_${j}__muonReco.root" ]
            then
                echo "File ${output}/${run_number}/${mode}/filtered_MC_00${run_number}_${j}.root exists. Skipping."
                continue
            fi
            python3 $SNDSW_ROOT/shipLHC/run_muonRecoSND.py -f ${output}/${run_number}/STAGE1/filtered_MC_00${run_number}_${j}.root -g ${geo_file} -c passing_mu_DS -sc 1 -s ./ -hf linearSlopeIntercept -o
            mkdir -p ${output}/${run_number}/${mode}/
            xrdcp -f ./filtered_MC_00${run_number}_${j}__muonReco.root ${output}/${run_number}/${mode}/
            rm -f ./filtered_MC_00${run_number}_${j}__muonReco.root
        done

    elif [ "$mode" == "STAGE2" ] 
    then
        for j in $(seq 0 "$n_files")
        do
            if [ -f "${output}/${run_number}/${mode}/filtered_MC_00${run_number}_${j}_stage2_noscifi2.root" ]
            then
                echo "File "${output}/${run_number}/${mode}/filtered_MC_00${run_number}_${j}_stage2_noscifi2.root" exists. Skipping."
                continue
            fi
            python3 ${SNDSW_ROOT}/analysis/neutrinoFilterGoldenSample_stage2.py -f ${output}/${run_number}/STAGE1/${run_number}/filtered_MC_00${run_number}_${j}.root -t ${output}/${run_number}/RECO/filtered_MC_00${run_number}_${j}__muonReco.root -o filtered_MC_00${run_number}_${j}_stage2_noscifi2.root -g ${geo_file};
            mkdir -p ${output}/${run_number}/${mode}/
            xrdcp -f ./filtered_MC_00${run_number}_${j}_stage2_noscifi2.root ${output}/${run_number}/${mode}/
            rm -f ./filtered_MC_00${run_number}_${j}_stage2_noscifi2.root
        done
    
    else
        echo "Mode $mode not recognized. Exitting."
        exit
    fi
    echo "Finished processing run number $run_number"
done