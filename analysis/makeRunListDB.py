import pymongo
import argparse
import xml.etree.ElementTree as ET
from datetime import datetime
from collections import defaultdict
import os

parser = argparse.ArgumentParser(
    prog="makeRunListDB",
    description="Extracts a list of runs from the SND@LHC DB, matching the conditions given in the arguments. Produces an XML file with the run list and a summary of the selection.")
parser.add_argument("name", type=str, help="run list name")
parser.add_argument("years", nargs="+", type=int, help="years to be included, e.g. 2022 2023")
parser.add_argument("min_events", type=int, help="minimum number of events in run")
parser.add_argument("min_lumi", type=float, help="minimum integrated luminosity for a run to be included, in fb-1")
parser.add_argument("min_stable_time", type=float, help="minimum stable beams time of the corresponding LHC fill for a run to be included, in minutes")
parser.add_argument("particle_B1", type=str, help="beam 1 particle, e.g. p+ or PB82")
parser.add_argument("particle_B2", type=str, help="beam 2 particle, e.g. p+ or PB82")
args = parser.parse_args()

client = pymongo.MongoClient("sndrundb.cern.ch")
db = client.sndrundb

pipeline = []

# Get the run list corresponding to the selected years
pipeline.append({"$match": {"$expr": {"$in": [{"$year": "$start"}, args.years]}}})

# Select runs with a minimum of min_events events:
pipeline.append({"$match": {"events": {"$gt": args.min_events}}})

# Combine data fill from the LPC
pipeline.append({"$lookup": {"from": "FILL_LPC", "localField": "fill", "foreignField": "_id", "as": "LPC"}})

# Select runs with at least min_lumi integrated luminosity
pipeline.append({"$match": {"LPC.ATLAS_Int_Lumi": {"$gt": args.min_lumi*1e6}}})

# Select runs with at least min_stable_time minutes of stable beams
pipeline.append({"$match": {"LPC.Stable_time": {"$gt": args.min_stable_time/60.}}})

# Select B1 particle
pipeline.append({"$match": {"LPC.Particle_B1": args.particle_B1}})

# Select B2 particle
pipeline.append({"$match": {"LPC.Particle_B2": args.particle_B2}})

# Expression for Calculating run length from start and stop datetimes
run_length_expr = {"$dateDiff": {"startDate": "$start", "endDate": "$stop", "unit": "minute"}}

# Extract the following data from the DB
pipeline.append({"$project":{"_id": 0, # Do not extract DB entry ID 
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
                 })

result = list((db["EcsData"].aggregate(pipeline)))

# Get the time now
now = datetime.now()

# Format into xml tree
root = ET.Element("runlist")

meta_data = ET.SubElement(root, "meta")
ET.SubElement(meta_data, "name").text = args.name
ET.SubElement(meta_data, "datetime").text = now.strftime("%Y-%m-%dT%H:%M:%S.%f")
selection = ET.SubElement(meta_data, "selection")
ET.SubElement(selection, "years").text = ','.join([str(y) for y in args.years])
for criterion in ["min_events", "min_lumi", "min_stable_time", "particle_B1", "particle_B2"]:
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
