#!/bin/python
from jsonToHDL import deparserStateMachines
import sys
import os
import argparse
import json


def genPHVInfo(phv, baseName='phv_'):
    headAssoc = {} # data hdr bus
    valAssoc = {} # validity bit
    phvWidth = 0
    i = 0
    for n, w in phv.items():
        headAssoc[n] = (phvWidth, phvWidth + w - 1)
        phvWidth += w
        valAssoc[n] = i
        i += 1
    headAssoc = (headAssoc, phvWidth)
    valAssoc = (valAssoc, i)
    info = [{"name": "{}bus".format(baseName),
            "width": headAssoc[1],
            "data": headAssoc[0]},
            {"name": "{}val".format(baseName),
            "width": valAssoc[1],
            "data": valAssoc[0]}]
    return info



def DeparserComp(deparser, outputFolder, busWidth=64):
    deparserSplit = deparserStateMachines(deparser, busWidth)
    outputImgFold = os.path.join(outputFolder, "img")
    if not os.path.exists(outputImgFold):
            os.mkdir(outputImgFold)
    deparserSplit.exportToDot(outputImgFold)
    deparserSplit.exportToPng(outputImgFold)
    phvBus = genPHVInfo(deparser["PHV"])
    deparserSplit.exportToVHDL(outputFolder, phvBus)

def compJsonArg(filename, outputFolder, deparserName):
    
    if os.path.isdir(filename):
        print(f"compiling file in {filename}")
        for file in os.listdir(filename):
            newFile=os.path.join(filename,file)
            newOutput=os.path.join(outputFolder, file.split(".")[0])
            compJsonArg(newFile, newOutput, deparserName)
        return
    print(f"compiling {filename} -> {outputFolder}")
    P4Json = None
    with open(filename,'r') as f:
        P4Json = json.loads(f.read())
    deparser=P4Json[deparserName]
    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)
    DeparserComp(deparser, outputFolder, P4Json["outputBus"])


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
    
    for jsonFile in jsonNames:
        compJsonArg(jsonFile, os.path.join(os.getcwd(), args.output), args.deparserName)


if __name__ == "__main__":
    # execute only if run as a script
    main(sys.argv)
    exit()


