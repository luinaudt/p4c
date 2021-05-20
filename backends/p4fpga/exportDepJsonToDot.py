"""
Copyright 2021-present Thomas Luinaud

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import networkx
import json
import getopt
import sys
import os
from pathlib import Path

def convertFolder(inputFolder, outputFolder, deparserName):
    if not os.path.exists(outputFolder):
        print("making output folder {}".format(outputFolder))
        os.mkdir(outputFolder)
    if not os.path.isdir(outputFolder):
        print("if input is a folder output must be a folder")
        sys.exit(1)
    for i in os.listdir(inputFolder):
        if os.path.isdir(i):
            convertFolder(os.path.join(inputFolder, i),
                          os.path.join(outputFolder,i),
                          deparserName)
        outFile = Path(os.path.join(outputFolder, i))
        outFile = str(outFile.parent.joinpath(outFile.stem + ".dot"))
        convertFile(str(os.path.join(inputFolder,i)), outFile ,deparserName)
        

def convertFile(inputFile, outputFile, deparserName):
    if Path(inputFile).suffix != ".json":
        print("{} is not a json suffix".format(inputFile))
        print("exit convertion")
        return
    print("processing {}".format(inputFile))
    with open(inputFile,'r') as f:
        deparser = json.load(f)[deparserName]
    graph = networkx.readwrite.json_graph.node_link_graph(deparser["graph"], directed=True)
    networkx.drawing.nx_agraph.write_dot(graph, outputFile)

def convert(fileName = "a.out", outfileName= "a.dot", deparserName = "deparser"):
    if os.path.isdir(fileName):
        convertFolder(fileName, outfileName, deparserName)
    else:
        convertFile(fileName, outfileName, deparserName)
    

def main(prog, argv):
    inputFile = "a.out"
    outputFile = "a.json"
    deparserName = "deparser"
    if len(argv) <=0 :
        print("argument are required")
        print(f"{prog} [-i inputFile] [-o outputFile] [--deparserName name]")
        sys.exit(2)
    try:
        opts, _= getopt.getopt(argv, "ho:i:",
                                        ["help", "outputFile", "inputFile",
                                        "deparserName"])
    except getopt.GetoptError:
        print(f"{prog} [-i inputFile] [-o outputFile] [--deparserName name]")
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print(f"{prog} [-i inputFile] [-o outputFile] [--deparserName name]")
            sys.exit(0)
        elif opt in ("-o", "--outputFile"):
            outputFile = arg
        elif opt == "--deparserName":
            deparserName = arg
        elif opt in ("-i", "--inputFile"):
            inputFile = arg
    if os.path.exists(inputFile):
        convert(inputFile, outputFile, deparserName)
    else:
        print("{} does not exists".format(inputFile))
        sys.exit(1)
    

if __name__ == "__main__":
    # execute only if run as a script
    main(sys.argv[0], sys.argv[1:])
    exit()