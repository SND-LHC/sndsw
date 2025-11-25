xml=$1
output=$2
start=$4
end=$5
cutset=$6
mode=$7

export ALIBUILD_WORK_DIR=/afs/cern.ch/user/s/schuetha/work/public/data_work_flow/sw
source /cvmfs/sndlhc.cern.ch/SNDLHC-2025/Jan30/setUp.sh
eval `alienv -a slc9_x86-64 load --no-refresh sndsw/master-local1`
echo "Finished setting up SNDSW env"
export EOSSHIP=root://eosuser.cern.ch