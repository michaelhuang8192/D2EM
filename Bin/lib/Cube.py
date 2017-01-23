from D2EM import *


def HasCubeInStash():
    stashItems = GetItemsFromInv(6)
    
    for i in stashItems:
        if i[0] == 549: return i[1]
        
    return False
    
def OpenCube(id=0):
    if not id:
        units = GetItemsFromInv(6, 549)
        if not units:
            DLog("No Cube Id", 2)
            return
        id = units[0][1]
    
    myPos = GetPos()
    
    while not GetPanelFlag(PFG_CUBE):
        DLog("Open Cube[0x%X]" % (id, ), 0)
        UseItemFromInv(id, myPos[0], myPos[1])
        
        tick = GetTickCount()
        while not GetPanelFlag(PFG_CUBE):
            Sleep(50)
            if GetTickInterval(tick) >= 800:
                break
            
def CloseCube():
    while GetPanelFlag(PFG_CUBE):
        PostEsc()
        
        tick = GetTickCount()
        while GetPanelFlag(PFG_CUBE):
            Sleep(50)
            if GetTickInterval(tick) >= 800:
                break

def IsCubeEmpty():
    cube = GetMapFromInv(5)
    if cube:
        if len(cube[1]):
            for i in cube[1]:
                if i: return False
                
    return True

def HasItemInCube(clsId):
    cube = GetMapFromInv(5)
    if cube:
        if len(cube[1]):
            for i in cube[1]:
                unit = GetUnit(0, 4, i)
                if unit and unit[0] == clsId: return True
                
    return False

def AddItemToCube_S(itemId, cubeId):
    while True:
        cid = GetInvInfo()[2]
        if not cid: return
        
        AddItemToCube(itemId, cubeId)
        if wait4_cursor_clear(500):
            Sleep(200)
            return

def Transmute():
    ClickButton(0x18, 0)