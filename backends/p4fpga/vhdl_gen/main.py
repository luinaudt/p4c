#!/bin/python
from jsonToHDL import deparserStateMachines
import sys
import os
import argparse
import json

def comp(deparserJson, outputFolder, busWidth=64):
    deparserSplit = deparserStateMachines(deparserJson, busWidth)
    deparserSplit.exportToDot(outputFolder)
    deparserSplit.exportToPng(outputFolder)

    


def main(argv):
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--output", help="output folder vhdl code", required=True)
    parser.add_argument("--deparserName", help="Name of the deparser (in the json files)", required=False, default="deparser")
    args, jsonNames = parser.parse_known_args()
    if jsonNames is None or len(jsonNames) == 0:
        print("need a json file none where given")
        sys.exit(1)
    elif len(jsonNames) > 1:
        print("only one json convert is currently supported")
        sys.exit(1)

    jsonFile = jsonNames[0] # only one json accepted currently
    P4Program = None
    with open(jsonFile,'r') as f:
        P4Program = json.loads(f.read())


    deparser = P4Program[args.deparserName]
    busWidth = P4Program["outputBus"] # name might have to be configurable
    
    output = os.path.join(os.getcwd(), args.output)
    if not os.path.exists(output):
        os.mkdir(output)
    comp(deparser, output, busWidth)


if __name__ == "__main__":
    # execute only if run as a script
    main(sys.argv)
    exit()


