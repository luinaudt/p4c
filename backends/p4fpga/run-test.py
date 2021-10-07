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
from subprocess import Popen,PIPE, TimeoutExpired
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
    if not os.path.exists(dir):
        os.mkdir(dir)
    if not os.path.exists(os.path.join(dir, jsonFolder)):
        os.mkdir(os.path.join(dir, jsonFolder))
    if not os.path.exists(os.path.join(dir, p4Folder)):
        os.mkdir(os.path.join(dir, p4Folder))

def process(args, otherArgs):
    # command example : ./p4c/p4fpga --top4 Last,FPGABackend --dump . -o test_new.json src/testComp/t0.p4
    commands = []
    for i in os.listdir(os.path.join(args.inputFolder)):
        if Path(i).suffix != ".p4":
            continue
        jsonOutFile = Path(os.path.join(args.outputTest, "json", i))
        jsonOutFile = str(jsonOutFile.parent.joinpath(jsonOutFile.stem + ".json"))
        dumpFolder = str(os.path.join(args.outputTest, P4FOLDER))
        cmdArgs = [args.compiler, 
                   "-o", str(jsonOutFile), 
                   "--dump", str(dumpFolder),
                   "--top4", ",".join(args.passToDump)]
        cmdArgs.extend(otherArgs)
        cmdArgs.append(os.path.join(args.inputFolder, i))
        commands.append(cmdArgs)
    if args.dryRun:
        launchCompilation(commands, -1)
    else:
        launchCompilation(commands, args.nbThreads)


def launchCompilation(commands, nbThread=1):
    """ 
    commands must be a list
    launch all commands : multithreads
    """
    localList = []
    for cmdArgs in commands:
        print(" ".join(cmdArgs))
        if nbThread<0:
            continue
        local = Local()
        local.process = Popen(cmdArgs)
        localList.append(local)
        while len(localList) >= nbThread:
            for local in localList:
                try:
                    local.process.wait(0.1)
                except TimeoutExpired:
                    continue
                else:
                    localList.remove(local)
    #wait everything finish
    while len(localList) > 0:
        for local in localList:
            try:
                local.process.wait(0.1)
            except TimeoutExpired:
                continue
            else:
                localList.remove(local)

def main(argv):
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--inputFolder", help="folder of p4 source files", required=True)
    parser.add_argument("--outputTest", help="output folder results", required=True)
    parser.add_argument("--dryRun", help="dry-run, no execution", action="store_true", required=False)
    parser.add_argument("--compiler", help="compiler executable", required=True)
    parser.add_argument("--nbThreads", help="number of parallel compilation",type=int, default=1, required=False)
    parser.add_argument("--passToDump", help="pass to dump", nargs='+', default=["Last","Backend"], required=False)
    args, otherArgs = parser.parse_known_args()
    #args = parser.parse_args()
    error = False
    if not os.path.exists(args.inputFolder):
        print("input programs : {} does not exists".format(args.inputFolder))
        error = True
    if os.path.exists(args.outputTest):
        print("Test result : {} does exists".format(args.outputTest))
        error = True
        rep = input("overwrite y/n ? ")
        if rep in ("y", "Y"):
            error = False
        else:
            print("exit, output folder already exist")
    if not os.path.exists(args.inputFolder):
        print("Compiler {} does not exists".format(args.compiler))
        error = True
    if error: exit(1)
    if not args.dryRun:
        mkOutputDir(args.outputTest, JSONFOLDER, P4FOLDER)
    process(args, otherArgs)
    
    
if __name__ == "__main__":
    main(sys.argv)
