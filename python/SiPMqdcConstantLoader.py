import json
import ROOT
import os

def loadConstants(runNumberOrJSONpath=None, csvPath=None):
    if type(runNumberOrJSONpath) == str:
        JSONpath = runNumberOrJSONpath
        if csvPath is not None:
            print("Warning: csvPath argument is ignored when runNumberOrJSONpath is a string.")
    elif type(runNumberOrJSONpath) == int:
        runNumber = runNumberOrJSONpath
        if csvPath is None:
            csvPath = os.path.join(os.environ['SNDSW_ROOT'], "analysis/tools", "SiPMqdcCalibrationConstantsPaths.csv")
        with open(csvPath) as f:
            lines = f.readlines()
        for line in lines[1:]:
            min_run, max_run, path = line.strip().split(',')
            if int(min_run) <= runNumber <= int(max_run):
                JSONpath = path
                break
        else:
            raise ValueError(f"Run number {runNumber} not found in CSV file {csvPath}.")
    else:
        raise ValueError("runNumberOrJSONpath must be either a string (path to JSON) or an integer (run number).")

    ROOT.gInterpreter.Declare(r"""
    #include <map>
    #include "SiPMqdcCalibrationConstants.h"
    """)

    with open(JSONpath) as f:
        data = json.load(f)

    for k, v in data.items():
        ROOT.SiPM_qdc_calibration_constants[int(k)] = float(v)