from D2EM import *


def _Init(reason):
    if reason == 0:
        DLog("Init command line", 0)
        global parameters, commands, spin, tele, destLvl, up, rb, clear, pos, down, drop, toInv, toStash, bw, bwPosX, bwPosY
        spin = False
        tele = False
        up = False
        down = False
        rb = False
        clear = False
        bw = False
        pos = 0
        bwPosX = 25069
        bwPosY = 5071
        drop = toInv = toStash = False
        destLvl = 0
        parameters = {"-p" : "", "-f" : "", "-s" : "", "-w": "", "-p": "", "-x": "", "-y": ""}
        commands = {"dumpmap" : DumpMap, "spin" : Spin, "tele" : Tele, "getwarp" : PrintWarps, "up" : Up, "rb" : Rb, "kill":Kill, "down":Down, "drop":Drop, "tostash":ToStash, "toinv":ToInv, "bw":Bonewall}

def _Reset():
    global parameters
    parameters = {"-p" : "", "-f" : "", "-s" : "", "-w": "", "-x": "", "-y": ""}
    
def CommandParser(args):
    cl = args[0].split(" ")
    if not commands.has_key(cl[0]): return
    command = commands[cl[0]]
    addVar = False
    curPar = ""
    
    if len(cl[1:]) >= 2:
        for a in cl[1:]:
            if parameters.has_key(a) and cl[cl.index(a)+1] and cl[cl.index(a)+1][0] != "-":
                curPar = a
                addVar = True
            
            elif curPar and addVar:
                parameters[curPar] = a
                curPar = ""
                addVar = False
        
    command()
    
    _Reset()

def DumpMap():
    if parameters["-p"] and parameters["-f"]:
        DumpCurrentMap(parameters["-p"], int(parameters["-f"]))
    
    else:
        DumpCurrentMap("c:\\map1.txt", 0x1C01)
        DumpCurrentMap("c:\\map2.txt", 0xc2f)
        
def Spin():
    global spin
    spin = False
    if parameters["-s"] and parameters["-s"].lower() != "false":
        DLog("spin", 0)
        spin = True
        
def Tele():
    global destLvl, tele
    tele = False
    destLvl = 0
    
    if parameters["-w"]:
        destLvl = int(parameters["-w"])
        tele = True

def PrintWarps():
    warps = GetStaticUnits(5)
    if warps:
        for w in warps:
            DLog("warp cls id [%d]" % ( w[4], ), 0)
            
def Up():
    global up, rb, pos, spin
    up = True
    rb = False
    down = False
    spin = False
    pos = 0
    if parameters["-p"]: pos = int(parameters["-p"])
        
def Down():
    global down, rb, pos, spin
    down = True
    up = False
    rb = False
    spin = False
    pos = 0
    if parameters["-p"]: pos = int(parameters["-p"])
    
def Rb():
    global rb, spin, up
    spin = False
    up = False
    rb = True

def Kill():
    global clear
    clear = False
    if parameters["-s"] and parameters["-s"].lower() != "false":
        DLog("kill", 0)
        clear = True

def Drop():
    global drop
    drop = True

def ToStash():
    global toStash
    toStash = True
    
def ToInv():
    global toInv
    toInv = True

def Bonewall():
    global bw, bwPosX, bwPosY
    bw = True
    if parameters["-s"] and parameters["-s"].lower() == "false":
        DLog("stop bw", 0)
        bw = False
        
    if parameters["-x"]:
        try:
            bwPosX = int(parameters["-x"])
        except:
            DLog("Bonewall position X must be integer", 4)
    
    if parameters["-y"]:
        try:
            bwPosY = int(parameters["-y"])
        except:
            DLog("Bonewall position Y must be integer", 4)
            
AttachLib(_Init)