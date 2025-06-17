"""Compare two geofiles with or without the calibration constants."""

from argparse import ArgumentParser

import SndlhcGeo

parser = ArgumentParser()
parser.add_argument(
    "-g1", "--geofile1", help="first geometry file: geo1", required=True
)
parser.add_argument(
    "-g2", "--geofile2", help="second geometry file: geo2", required=True
)
parser.add_argument(
    "--compare_calibration", help="compare calibration constants", action="store_true"
)
options = parser.parse_args()

geo1 = SndlhcGeo.GeoInterface(options.geofile1)
geo2 = SndlhcGeo.GeoInterface(options.geofile2)

print("===========")
if options.compare_calibration:
    print("Run comparing the geo calibration constants")
else:
    print("Run WITHOUT comparing the geo calibration constants")

calibration_subkeys = ["Loc", "RotPhi", "RotPsi", "RotTheta", "station", "DSTcor", "Shift"]
detector_list = ["EmulsionDet", "Scifi", "MuFilter"]

for detector in detector_list:
    print("Running over", detector)
    print("  Checking if geo1 elements exist in geo2")
    for key in geo1.snd_geo[detector].keys():
        if not options.compare_calibration:
            if any(subkey in key for subkey in calibration_subkeys) and not detector=='EmulsionDet':
                continue
        if key not in geo2.snd_geo[detector].keys():
            print("   ", key, "is missing from geo2")
    print("  Checking if geo2 elements exist in geo1")
    for key in geo2.snd_geo[detector].keys():
        if not options.compare_calibration:
            if any(subkey in key for subkey in calibration_subkeys) and not detector=='EmulsionDet':
                continue
        if key not in geo1.snd_geo[detector].keys():
            print("   ", key, "is missing from geo1")

    print("  Checking the values of the elements that exist in both files")
    for key in geo1.snd_geo[detector].keys():
        if not options.compare_calibration:
            if any(subkey in key for subkey in calibration_subkeys) and not detector=='EmulsionDet':
                continue
        if (
            key in geo2.snd_geo[detector].keys()
            and abs(geo1.snd_geo[detector][key] - geo2.snd_geo[detector][key]) != 0
        ):
            print(
                "   ",
                key,
                "Different value detected! geo1:",
                geo1.snd_geo[detector][key],
                "geo2:",
                geo2.snd_geo[detector][key],
            )
