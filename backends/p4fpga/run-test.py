#!/usr/bin/env python3
# Copyright 2021-present Thomas Luinaud
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import argparse
from subprocess import Popen,PIPE
from pathlib import Path
import shutil

SUCCESS = 0
FAILURE = 1

class Options(object):
    def __init__(self):
        self.cleanupTmp = True          # if false do not remote tmp folder created
        self.p4filename = ""            # file that is being compiled
        self.compilerSrcDir = ""        # path to compiler source tree
        self.verbose = False
        self.replace = False            # replace previous outputs
        self.dumpToJson = False
        self.compilerOptions = []
        self.runDebugger = False
        self.runDebugger_skip = 0
        self.generateP4Runtime = False
# standard value, might be change in the future
JSONFOLDER = "json"
P4FOLDER = "p4"

class Local(object):
    # object to hold local vars accessable to nested functions
    pass

def mkOutputDir(dir, jsonFolder, p4Folder):
    os.mkdir(dir)
    os.mkdir(os.path.join(dir, jsonFolder))
    os.mkdir(os.path.join(dir, p4Folder))

def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--inputFolder", help="folder of p4 source files", required=True)
    parser.add_argument("--outputTest", help="output folder results", required=True)
    parser.add_argument("--compiler", help="compiler executable", required=True)
    parser.add_argument("--passToDump", help="pass to dump", required=False, default="Last,Backend")
    args = parser.parse_args()
    error = False
    if not os.path.exists(args.inputFolder):
        print("input programs : {} does not exists".format(args.inputFolder))
        error = True
    if os.path.exists(args.outputTest):
        print("Test result : {} does exists".format(args.outputTest))
        error = True
        rep = input("replace result in Folder y/n ? ")
        if rep in ("y", "Y"):
            error = False
            shutil.rmtree(args.outputTest, ignore_errors=True)    
    
    if not os.path.exists(args.inputFolder):
        print("Compiler {} does not exists".format(args.compiler))
        error = True
    if error: exit(1)
    mkOutputDir(args.outputTest, JSONFOLDER, P4FOLDER)

# command example : ./p4c/p4fpga --top4 Last,FPGABackend --dump . -o test_new.json src/testComp/t0.p4
    for i in os.listdir(os.path.join(args.inputFolder)):
        jsonOutFile = Path(os.path.join(args.outputTest, "json", i))
        jsonOutFile = str(jsonOutFile.parent.joinpath(jsonOutFile.stem + ".json"))
        dumpFolder = str(os.path.join(args.outputTest, P4FOLDER))
        cmdArgs = [args.compiler, 
                   "-o", str(jsonOutFile), 
                   "--dump", str(dumpFolder),
                   "--top4", ",".join(args.passToDump.split(" ")),
                   os.path.join(args.inputFolder, i)]
        print(" ".join(cmdArgs))
        local = Local()
        local.process = Popen(cmdArgs)
        local.process.wait()

    
    
if __name__ == "__main__":
    main(sys.argv)