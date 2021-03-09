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

def convert(fileName = "a.out", outfileName= "a.dot", deparserName = "deparser"):
    with open(fileName,'r') as f:
        deparser = json.load(f)[deparserName]
    graph = networkx.readwrite.json_graph.node_link_graph(deparser, directed=True)
    networkx.drawing.nx_agraph.write_dot(graph, outfileName)

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
    convert(inputFile, outputFile, deparserName)
    

if __name__ == "__main__":
    # execute only if run as a script
    main(sys.argv[0], sys.argv[1:])
    exit()