import re

listId = [("a" , True),
          ("adfkla", True),
          ("fdsafjkl_9f99fasd0000", True),
          ("1fdasf", False),
          ("aAaZs0_0fdl", True),
          ("unvalid__DD_", False),
          ("unvalid__DD", False),
          ("unvalid____DD", False),
          ("unvalid___DD_", False),
          ("unvalid___DD45", False),
          ("unvalidDD_", False),
          ("_unvalidDD", False) ]   

def isValidHDLId(name):
    """ remove unvalid character to have a valid hdlID
    """
    vhdlIdPatern = "[a-z](_?[a-z0-9])*[a-z0-9]?"
    res = re.fullmatch(vhdlIdPatern, name, re.IGNORECASE)
    if res:
        return True
    else:
        return False

def runTest():
    fail = False
    for i, v in listId:
        if v != isValidHDLId(i):
            print(i)
            fail = True
    if fail:
        print("failed")
    
if __name__ == "__main__":
    # execute only if run as a script
    runTest()
    exit()