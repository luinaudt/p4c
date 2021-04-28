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

def usage(options):
    name = options.binary
    print(name, "usage:")



def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--inputFolder", help="folder of p4 source files", required=True)
    parser.add_argument("--outputTest", help="output folder resultst", required=True)
    parser.add_argument("--compiler", help="compiler path", required=True)
    args = parser.parse_args()
    error = False
    if not os.path.exists(args.inputFolder):
        print(f"input programs : {args.inputFolder} does not exists" )
        error = True
    if not os.path.exists(args.outputTest):
        print(f"Test result : {args.outputTest} does not exists" )
        error = True
    if not os.path.exists(args.inputFolder):
        print(f"Compiler {args.compiler} does not exists" )
        error = True
    if error: exit(1)
    
    
if __name__ == "__main__":
    main(sys.argv)