#!/bin/python
from networkx.algorithms.structuralholes import constraint
from jsonToHDL import deparserStateMachines
import sys
from gen_vivado import gen_scripts, gen_vivado, export_constraints
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

def vivado_gen_wrapper(outputDir, deparser, args):
    """ wrapper for gen_vivado from : 
    https://github.com/luinaudt/deparser/blob/master/src/compiler/gen_vivado.py
    """
    projectParam = {"projectName": args.projectName,
                    "busWidth": args.busWidth,
                    "boardDir": os.path.join(os.getcwd(), "board", args.genVivado)}
    if not os.path.exists(projectParam["boardDir"]):
        raise NameError(f"{args.genVivado}, is not a valid board name, look at board folder")
    depParam = deparser.getVHDLParam()
    projectParam["phvBusWidth"] = depParam["phvBusWidth"]
    projectParam["phvValidityWidth"] = depParam["phvValidityWidth"]
    projectParam["phvValidityDep"] = depParam["phvValidity"]
    projectParam["phvBusDep"] = depParam["phvBus"]
    projectParam["deparserName"] = depParam["name"]
    vhdlDir = deparser.getMainOutputDir()
    projectDir = os.path.join(outputDir, args.genVivado)
    constraintsDir = os.path.join(outputDir, "constraints")
    buildFile = str(os.path.join(outputDir, args.genVivado, "build.tcl"))
    gen_vivado(projectParam, vhdlDir, projectDir)
    constraintsFile = export_constraints(projectParam, constraintsDir)
    gen_scripts(buildFile, projectDir, constraintsFile)
    

def DeparserComp(deparser, outputFolder, busWidth=64):
    deparserSplit = deparserStateMachines(deparser, busWidth)
    outputImgFold = os.path.join(outputFolder, "img")
    if not os.path.exists(outputImgFold):
            os.mkdir(outputImgFold)
    deparserSplit.exportToDot(outputImgFold)
    deparserSplit.exportToPng(outputImgFold)
    outputRtl = os.path.join(outputFolder, "rtl")
    phvBus = genPHVInfo(deparser["PHV"])
    return deparserSplit.exportToVHDL(outputRtl, phvBus)
    

def compJsonArg(filename, outputFolder, args):
    if os.path.isdir(filename):
        print(f"compiling file in {filename}")
        for file in os.listdir(filename):
            newFile=os.path.join(filename,file)
            newOutput=os.path.join(outputFolder, file.split(".")[0])
            compJsonArg(newFile, newOutput, args)
        return
    print(f"compiling {filename} -> {outputFolder}")
    deparserName = "deparser"
    P4Json = None
    with open(filename,'r') as f:
        P4Json = json.loads(f.read())
    deparser=P4Json[deparserName]
    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)
    vhdlDep = DeparserComp(deparser, outputFolder, P4Json["outputBus"])
    if args.genVivado:
        vivado_gen_wrapper(outputFolder, vhdlDep, args)

def main(argv):
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--output", help="output folder vhdl code", required=True)
    parser.add_argument("--projectName", help="Name of the project", required=False, default="project1")
    parser.add_argument("--genVivado", help="Generate vivado files with the board file given", required=False, default=None)
    parser.add_argument("--busWidth", help="fifo bus width", required=False, default=64)
    args, jsonNames = parser.parse_known_args()

    if jsonNames is None or len(jsonNames) == 0:
        print("need a json file none where given")
        sys.exit(1)
    elif len(jsonNames) > 1:
        print("only one json convert is currently supported")
        sys.exit(1)
    
    for jsonFile in jsonNames:
        compJsonArg(jsonFile, os.path.join(os.getcwd(), args.output), args)


if __name__ == "__main__":
    # execute only if run as a script
    main(sys.argv)
    exit()


