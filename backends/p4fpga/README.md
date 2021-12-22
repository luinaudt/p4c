# P4fpga Backend
This is backend for fpga targets.

Currently only support deparser code generation

# requirements :
python3
pip install networkx colorama pygraphviz

# running test :

Lancer un test en utilisant le compilateur P4FPGA

`python run-test.py --inputFolder source --outputFolder output --compiler p4fpga`
exemple dans le docker
` python ./p4fpga/run-test.py --compiler ./p4c/p4fpga --inputFolder p4fpga/test/paper_src/ --outputTest p4fpga/test/paper_res --nbThreads=11 --deparser-nosplit`

# Hardware generations 
all script are in vhdl_gen folder

- generate vhdl files:
`main.py --output <outputFolder> --busWidth <width> <folder json sources>`
- generate vhdl files and vivado build:
`main.py --genVivado <folder in board folder> --output <outputFolder> --busWidth <width> <folder json sources>`

when genVivado : a build file is created for each cases in the json folder and a `genAllVivadoBuild.tcl` file is created.
This files launch the build of all the deparsers.

