#!/bin/python
import os
from string import Template

tclSynthTmpl="""
cd ${project}
source vivado.tcl
synth_design
write_checkpoint -force ${project}/Synth.dcp
report_timing_summary -file ${report_folder}/post_synth_timing_summary.rpt
report_utilization -hierarchical -file ${report_folder}/post_synth_util_hier.rpt
"""
tclImplTmpl="""
read_checkpoint ${project}/Synth.dcp
${constraints}
link_design -part xcvu3p-ffvc1517-3-e -top top -mode out_of_context
opt_design
write_checkpoint -force ${project}/post_opt.dcp
place_design
report_timing_summary -file ${report_folder}/post_place_timing_summary.rpt
report_utilization -hierarchical -file ${report_folder}/post_place_util.rpt
write_checkpoint -force /${project}/post_place.dcp
route_design
report_timing_summary -file ${report_folder}/post_route_timing_summary.rpt
write_checkpoint -force ${project}/post_route.dcp
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

