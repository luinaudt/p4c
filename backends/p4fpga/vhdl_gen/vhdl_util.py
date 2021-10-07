from warnings import warn
from random import randint 
from math import log2, ceil
from vhdl_keywords import vhdlKeywordsSet as vhdlReservedWord
import re
""" Some utility function for VHDL generation
"""


def getLog2In(nbInput):
    if nbInput==0:
        # special cases if no input, should be simplified after
        return 0
    return int(ceil(log2(nbInput)))

def getValidVHDLId(name):
    """ remove unvalid character to have a valid hdlID
    """
    retour = None
    vhdlIdPatern = "[a-z](_?[a-z0-9])*[a-z0-9]?"
    pattern = re.compile(vhdlIdPatern, re.IGNORECASE)
    res = pattern.fullmatch(name)
    if res:
        retour= name
    else:
        output=[]
        res = pattern.search(name)
        while res:
            start, end = res.span()
            output.append(res.string[start:end])
            res = pattern.search(name, end)
        retour= "_".join(output)
    if retour in vhdlReservedWord:
        return f"{retour}_res"
    return retour
    
def int2vector(val, vecSize):
    """
    return the string to have a std_logic_vector
    """
    return "std_logic_vector({})".format(int2unsigned(val, vecSize))


def int2unsigned(val, size=None):
    """ return to unsigned of a val
    if size unsupported add comment with random value
    """
    if size is None:
        return "unsigned({})".format(val)
    elif isinstance(size, str) or isinstance(size, int):
        return "to_unsigned({},{})".format(val, size)
    else:
        ret = "to_unsigned({},{}) -- ERROR {}".format(val,
                                                      size,
                                                      randint(0, 100))
        warn("unsupported value returned : {}".format(ret))
        return ret
