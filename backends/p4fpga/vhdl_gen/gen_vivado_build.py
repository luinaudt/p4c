#!/bin/python
import os
from string import Template

tclSynthTmpl="""
cd $project
source vivado.tcl
set_property STEPS.SYNTH_DESIGN.ARGS.MAX_BRAM 0 [get_runs synth_1]
set_property STEPS.SYNTH_DESIGN.ARGS.MAX_URAM 0 [get_runs synth_1]
launch_runs synth_1
wait_on_run synth_1
open_run synth_1
file mkdir ${report_folder}
report_timing_summary -file ${report_folder}/post_synth_timing_summary.rpt
report_utilization -hierarchical -file ${report_folder}/post_synth_util_hier.rpt
"""
tclImplTmpl="""
launch_runs impl_1
wait_on_run impl_1
open_run impl_1
report_timing_summary -file ${report_folder}/post_place_timing_summary.rpt
report_utilization -hierarchical -file ${report_folder}/post_place_util.rpt
"""

def cleanVivadoDir(directory):
    for d, p, f in os.walk(directory):
        for fichier in f:
            if fichier == "vivado.tcl":
                continue
            os.remove(os.path.join(d, fichier))
    while len(os.listdir(directory)) > 1:
        for p, d, _ in os.walk(directory):
            if p == directory:
                continue
            if len(d) == 0:
                os.rmdir(p)

