import json
import ROOT
import warnings

def loadConstants(runNumberOrJSONpath=None, csvPath=None):
    """Load SiPM QDC calibration constants into ROOT global objects.

    The constants can be loaded either from a JSON file supplied directly or
    by mapping a run number to a JSON file using a CSV file. In Monte Carlo
    mode, the global calibration offset is set to zero.

    Parameters
    ----------
    runNumberOrJSONpath : int or str, optional
        If an integer is provided, it is interpreted as a run number and used
        to select the corresponding JSON file from ``csvPath`` (or the default
        mapping). Any negative run number loads the default constants for MC.
        If a string is provided, it is interpreted as the path to a JSON
        constants file, and ``csvPath`` is ignored. The JSON constants files
        contain subsystem-wide constant offsets (for VS, US, DS) as well as
        calibration constants for each individual SiPM. For the default files,
        see their paths in
        /eos/experiment/sndlhc/calibration/MuFilter/SiPMqdcCalibration/SiPMqdcCalibrationConstantsPaths.csv.
    csvPath : str, optional
        Path to a CSV file containing run-number ranges and JSON file paths.
        The CSV is expected to contain a header followed by rows of the form::

            minimum_run_number,maximum_run_number,json_path

        The lower and upper limits on the run number are BOTH inclusive.
        If omitted, when a run number is provided, the default mapping CSV
        file is used.

    Raises
    ------
    ValueError
        If ``runNumberOrJSONpath`` is neither an integer nor a string, or if
        the supplied run number cannot be found in the CSV mapping.
    FileNotFoundError
        If the JSON or CSV file cannot be opened.
    json.JSONDecodeError
        If the selected JSON file contains invalid JSON.

    Notes
    -----
    This function declares the required C++ header through ROOT and populates
    ``ROOT.SiPM_qdc_calibration_constants`` with the values from the selected
    JSON file.
    """
    
    if type(runNumberOrJSONpath) == str:
        JSONpath = runNumberOrJSONpath
        if csvPath is not None:
            warnings.warn("csvPath argument is ignored when runNumberOrJSONpath is a string.")
    elif type(runNumberOrJSONpath)==int:
        runNumber = runNumberOrJSONpath

        # any negative run number is treated as MC the same way,
        # unless a csv file (which might contain constants for different
        # negative run numbers, e.g. different MC) is provided
        if runNumber <= -1 and csvPath is None:
            warnings.warn("Any negative run number loads the default constants for MC, "
                          "unless a csv file with a mapping to constants for other "
                          "negative values is provided in the argument csvPath.")
            runNumber = -1

        if csvPath is None:  # use default mapping
            warnings.warn("Using default SiPM qdc calibration constant mapping.")
            csvPath = "/eos/experiment/sndlhc/calibration/MuFilter/SiPMqdcCalibration/SiPMqdcCalibrationConstantsPaths.csv"
        with open(csvPath) as f:
            warnings.warn("Reading SiPM qdc calibration constant mapping from " + csvPath + ".")
            lines = f.readlines()
        for line in lines[1:]:
            min_run, max_run, path = line.strip().split(',')
            if int(min_run) <= runNumber <= int(max_run):
                JSONpath = path
                break
        else:
            raise ValueError(f"Run number {runNumber} not found in CSV file {csvPath}.")
    else:
        raise ValueError("Error: runNumberOrJSONpath must be either an integer "
                         "(run number) or a string (path to constants JSON file).")

    ROOT.gInterpreter.Declare(r"""
    #include <map>
    #include "SiPMqdcCalibrationConstants.h"
    """)

    # Load constants and store in root object
    with open(JSONpath) as f:
        data = json.load(f)

    for k, v in data.items():
        ROOT.SiPM_qdc_calibration_constants[int(k)] = float(v)