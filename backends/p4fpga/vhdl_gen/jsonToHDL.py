""" This script convert a json deparser to VHDL code
Source from luinaudt/deparser github repo
requires networkx packages
"""
import networkx as nx
from vhdl_gen import exportDeparserToVHDL
from os import path
class deparserStateMachines(object):
    def __init__(self, deparser, busSize):
        """ init for deparserStateMachines
        deparser : a jsonDeparser
        busSize, size of the output bus in bits
        """
        # for future modification:
        # node structure in the graph
        self.nodesStructure = {
            "headerName" : 'HdrName',
            "headerPosition" : 'HdrPos', 
            "emitWidth" : 'HdrLen'}
        self.depG = nx.readwrite.json_graph.node_link_graph(
                                deparser["graph"], directed=True)
        self.headers = deparser["PHV"]
        self.init =  deparser["startState"]
        self.last = deparser["lastState"]
        self.busSize = busSize
        self.nbStateMachine = int(busSize/8)
        self.stateMachines = []
        for i in range(self.nbStateMachine):
            tmp = nx.DiGraph()
            tmp.add_node(self.init)
            tmp.add_node(self.last)
            self.stateMachines.append(tmp)
        self.genStateMachines()

    def _getHdrName(self, state):
        headerName = self.nodesStructure["headerName"]
        stateInfo = self.depG.nodes[state]
        if headerName not in stateInfo:
            return state
        stateHdr = stateInfo[headerName]
        if isinstance(stateHdr, list) and  len(stateHdr) > 1:
            raise ValueError("support only one header per state")
        if len(stateHdr) < 1:
            raise ValueError("No header")
        return stateHdr[0]

    def genStateMachines(self):
        paths = nx.all_simple_paths(self.depG, self.init, self.last)
        paths2 = nx.all_simple_edge_paths(self.depG, self.init, self.last)
        for n, p in enumerate(paths2):
            if n % 1000 == 999:
                print("gen stateMachine path: {}".format(n))
            st = 0
            prev_hdr = []
            for i in self.stateMachines:
                prev_hdr.append(p[0][0])
            #print(p)
            for edge in p:
                h = self._getHdrName(edge[1])
                for i in range(int(self.headers[h]/8)):
                    new_node = "{}_{}".format(h, i*8)
                    self.stateMachines[st].add_node(new_node,
                                                    header=h,
                                                    pos=(i*8, (i+1)*8-1))
                    if i < len(self.stateMachines):
                        self.stateMachines[st].add_edge(prev_hdr[st],
                                                        new_node,
                                                        label=h)
                    else:
                        self.stateMachines[st].add_edge(prev_hdr[st],
                                                        new_node)
                    prev_hdr[st] = new_node
                    st = (st + 1) % len(self.stateMachines)
            #we connect last state
            for i, m in enumerate(self.stateMachines):
                m.add_edge(prev_hdr[i], p[-1][1])

    def exportToDot(self, folder, basename="state_machine"):
        """ export all states machines to dot file
        """

        for i, st in enumerate(self.getStateMachines()):
            outFile = path.join(folder, f"{basename}_{i}.dot")
            nx.nx_pydot.write_dot(st, outFile)

    def exportToPng(self, folder, basename="state_machine"):
        """ export all states machines to png
        """
        for i, st in enumerate(self.getStateMachines()):
            tmp = nx.nx_pydot.to_pydot(st)
            tmp.write_png(path.join(folder, f"{basename}_{i}.png"))

    def exportToVHDL(self, outputFolder, baseName, phvBus):
        return exportDeparserToVHDL(self, outputFolder, phvBus, baseName)

    def printStPathsCount(self):
        for i, st in enumerate(self.getStateMachines()):
            nb = 0
            for j in nx.all_simple_paths(st, self.init, self.last):
                nb += 1
            print("state machine {} posseses {} path".format(
                i, nb))

    def getStateMachine(self, num):
        if num >= len(self.stateMachines):
            print("not a valid number")
            return None
        return self.stateMachines[num]

    def getStateMachines(self):
        return self.stateMachines
