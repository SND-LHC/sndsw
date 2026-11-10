import argparse
import xml.etree.ElementTree as ET

def count_runs(xml_file, min_events, outfile):
    tree = ET.parse(xml_file)
    root = tree.getroot()

    runs_selected = []
    n = 0

    # Find all <run> anywhere under the root
    for run in root.findall(".//run"):
        run_number = run.findtext("run_number")
        n_events_text = run.findtext("n_events", "0")

        # Skip if missing run_number
        if run_number is None:
            continue

        try:
            n_events = int(n_events_text)
        
        except ValueError:
            # Bad number in XML – skip this run
            continue
        
        n += 1

        if n_events >= min_events:
            runs_selected.append(run_number)

    floor    = n // min_events
    mod      = n %  min_events

    with open(outfile, "w") as f:
        for rn in range(1, floor + 1 if mod == 0 else floor + 2):
            if rn == 1:
                f.write(f"{rn}\t{rn*min_events-1}\n")

            if (rn)*min_events - 1 >= n and mod != 0:
                f.write(f"{(rn - 1)*min_events}\t{n}\n")
                break
            
            elif rn > 1 and rn <= floor + 1 and mod != 0:
                f.write(f"{(rn-1)*min_events}\t{rn*min_events-1}\n")
            
            elif rn > 1 and rn <= floor + 1 and mod == 0:
                f.write(f"{(rn-1)*min_events}\t{rn*min_events}\n")
    
    return len(runs_selected)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Select runs from an XML runlist based on minimum number of events."
    )
    parser.add_argument(
        "-xml", "--xml_file", required=True,
        help="Path to the XML runlist file."
    )
    parser.add_argument(
        "-n", "--min-events", dest="min_events", type=int, default=0,
        help="number of event per job."
    )
    parser.add_argument(
        "-o", "--output", default="se.txt",
        help="Output text file with selected run_numbers (default: se.txt)."
    )

    args = parser.parse_args()

    n_selected = count_runs(args.xml_file, args.min_events, args.output)
    print(f"Total runs selected: {n_selected}")