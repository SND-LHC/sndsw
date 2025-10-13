import pymongo
import argparse
import xml.etree.ElementTree as ET
from datetime import datetime
from collections import defaultdict
import os

parser = argparse.ArgumentParser(
    prog="makeRunListDB",
    description="Extracts a list of runs from the SND@LHC DB, matching the conditions given in the arguments. Produces an XML file with the run list and a summary of the selection.")
parser.add_argument("--name", type=str, help="Run list name", required=True)
parser.add_argument("--years", nargs="+", type=int, help="Years to be included, e.g. 2022 2023", required=True)
parser.add_argument("--min_events", type=int, help="Minimum number of events in run", default=0)
parser.add_argument("--min_lumi", type=float, help="Minimum integrated luminosity for a run to be included, in fb-1", default=0.)
parser.add_argument("--min_stable_time", type=float, help="Minimum stable beams time of the corresponding LHC fill for a run to be included, in minutes", default=0.)
parser.add_argument("--particle_B1", type=str, help="Beam 1 particle, e.g. p+ or PB82", required=True)
parser.add_argument("--particle_B2", type=str, help="Beam 2 particle, e.g. p+ or PB82", required=True)
parser.add_argument("--min_energy", type=float, help="Minimum beam energy (in GeV)", default=0.)
parser.add_argument("--max_energy", type=float, help="Maximum beam energy (in GeV)", default=0.)
parser.add_argument("--min_bunches_IP1", type=int, help="Minimum number of bunches colliding at IP1", default=0)
parser.add_argument("--max_bunches_IP1", type=int, help="Maximum number of bunches colliding at IP1", default=0)
parser.add_argument("--include_runs", nargs="+", type=int, help="Runs to include, regardless of the other criteria", default=[])
parser.add_argument("--exclude_runs", nargs="+", type=int, help="Runs to exclude from the list", default=[])
args = parser.parse_args()

client = pymongo.MongoClient("sndrundb.cern.ch")
db = client.sndrundb

pipeline = []

# Get the run list corresponding to the selected years
pipeline.append({"$match": {"$expr": {"$in": [{"$year": "$start"}, args.years]}}})

# Select runs with a minimum of min_events events:
if args.min_events > 0:
    pipeline.append({"$match": {"events": {"$gt": args.min_events}}})

# Combine data fill from the LPC
pipeline.append({"$lookup": {"from": "FILL_LPC", "localField": "fill", "foreignField": "_id", "as": "LPC"}})

if args.min_lumi > 0.:
    # Select runs with at least min_lumi integrated luminosity
    pipeline.append({"$match": {"LPC.ATLAS_Int_Lumi": {"$gt": args.min_lumi*1e6}}})

if args.min_stable_time > 0.:
    # Select runs with at least min_stable_time minutes of stable beams
    pipeline.append({"$match": {"LPC.Stable_time": {"$gt": args.min_stable_time/60.}}})

# Select B1 particle
pipeline.append({"$match": {"LPC.Particle_B1": args.particle_B1}})

# Select B2 particle
pipeline.append({"$match": {"LPC.Particle_B2": args.particle_B2}})

if args.min_energy > 0:
    # Select runs with args.min_energy energy
    pipeline.append({"$match": {"LPC.Energy": {"$gt": args.min_energy}}})

if args.max_energy > 0:
    # Select runs with args.max_energy energy
    pipeline.append({"$match": {"LPC.Energy": {"$lt": args.max_energy}}})
    
if args.min_bunches_IP1 > 0:
    # Select runs with at least min_bunches_IP1 bunches colliding at IP1
    pipeline.append({"$match": {"LPC.Coll_IP_1_5": {"$gt": args.min_bunches_IP1}}})
    
if args.max_bunches_IP1 > 0:
    # Select runs with at most max_bunches_IP1 bunches colliding at IP1
    pipeline.append({"$match": {"LPC.Coll_IP_1_5": {"$lt": args.max_bunches_IP1}}})

if len(args.exclude_runs) > 0:
    pipeline.append({"$match": {"runNumber": {"$nin" : args.exclude_runs}}})

# Expression for Calculating run length from start and stop datetimes
run_length_expr = {"$dateDiff": {"startDate": "$start", "endDate": "$stop", "unit": "minute"}}

# Extract the following data from the DB
projection = {"$project":{"_id": 0, # Do not extract DB entry ID 
                          "run_number": "$runNumber", # Run number
                          "n_events": "$events", # Number of events
                          "start": 1, # Start date
                          "end": 1, # End date
                          "duration": run_length_expr, # Run duration
                          "n_files": {"$size": "$files"}, # Number of files
                          "path": {"$first": "$files.file"}, # Path of the first file
                          "fill_number": "$fill", # Fill number
                          "fill_int_lumi": {"$first": "$LPC.ATLAS_Int_Lumi"}, # Integrated luminosity
                          "fill_stable_time" : {"$first": "$LPC.Stable_time"}} # Stable beams duration
              }

pipeline.append(projection)

result = list(db["EcsData"].aggregate(pipeline))

if len(args.include_runs) > 0:
    include_pipeline = []
    include_pipeline.append({"$match": {"runNumber": {"$in": args.include_runs}}})
    include_pipeline.append(projection)

    include_result = list(db["EcsData"].aggregate(include_pipeline))

    result.extend(list(include_result))

result.sort(key=lambda x: x["run_number"])

# Get the time now
now = datetime.now()

# Format into xml tree
root = ET.Element("runlist")

meta_data = ET.SubElement(root, "meta")
ET.SubElement(meta_data, "name").text = args.name
ET.SubElement(meta_data, "datetime").text = now.strftime("%Y-%m-%dT%H:%M:%S.%f")
selection = ET.SubElement(meta_data, "selection")
ET.SubElement(selection, "years").text = ','.join([str(y) for y in args.years])
for criterion in ["min_events", "min_lumi", "min_stable_time", "particle_B1", "particle_B2", "min_energy", "max_energy", "min_bunches_IP1", "max_bunches_IP1", "exclude_runs", "include_runs"]:
    ET.SubElement(selection, criterion).text = str(getattr(args, criterion))
runs = ET.SubElement(root, "runs")

# Counters for summary statistics
n_runs = 0
totals = defaultdict(int)

# Run loop
for run in result:
    this_run = ET.SubElement(runs, "run")
    totals["n_runs"] += 1
    for field_name in ["run_number", "start", "end", "n_events", "duration", "n_files", "fill_number", "fill_int_lumi", "fill_stable_time", "path"]:
        try:
            data = run[field_name]
        except KeyError:
            continue

        if field_name == "path":
            data = os.path.dirname(data)
        
        ET.SubElement(this_run, field_name).text = str(data)    

        if field_name not in ["run_number", "start", "end", "fill_number", "path", "fill_int_lumi", "fill_stable_time"] and data is not None:
            totals["tot_"+field_name] += data

stats = ET.SubElement(meta_data, "statistics")
for key, data in totals.items():
    ET.SubElement(stats, key).text = str(data)
    
# Write to xml file
tree = ET.ElementTree(root)
ET.indent(tree, space="    ")
tree.write(args.name+"_"+str(now.timestamp())+".xml", encoding="utf-8", xml_declaration=True)
