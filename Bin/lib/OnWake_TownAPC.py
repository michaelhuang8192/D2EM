from D2EM import *
from Town import HireMerc
from Stash import RemoteStash
from Town import IdentifyAll
from D2EMExtended import TpToTown, WalkToWp
from ItemCheck import GetInvGold, GetStashGold

tick = 0

def OnWake_TAPC():
    global tick
    
    if SetActiveMercHire() and GetTickCount() - tick > 5000:
        DLog("Enable APC for ActiveMercHire()", 0)
        EnableAPC(True)
        InsertAPC(ActiveMercHire, True)
        DLog("Disable APC for ActiveMercHire()", 0)
        EnableAPC(False)
        tick = GetTickCount()

def OnWake_TAPC_Stash():
    global tick
    
    if SetActiveIdStash() and GetTickCount() - tick > 5000:
        DLog("Enable APC for ActiveIdStash()", 0)
        EnableAPC(True)
        InsertAPC(ActiveIdStash, True)
        DLog("Disable APC for ActiveIdStash()", 0)
        EnableAPC(False)
        tick = GetTickCount()
        
def GetTpInfo():
    while True:
        ids = GetPortalIds()
        if not ids or not ids[0]:
            DLog("Find no tps, GetTpInfo()", 1)
            Sleep(0)
            continue
        
        id = ids[0]
        tpPos = GetPos(2, id)
        
        DLog("Gathering tp info, GetTpInfo()", 0)
        if tpPos:
            my_rm = GetRoom()
            if my_rm and IsTownLevel(my_rm[4]):
                tpLvl = my_rm[4]
                break
        
        Sleep(0)
    
    return (tpPos[0], tpPos[1], tpLvl)

def ActiveMercHire(hire=True):
    mercId = GetMercId()
    if mercId:
        if not IsDead(1, mercId): return
        
    TpToTown()
    
    tpInfo = GetTpInfo()
    
    HireMerc()
    
    my_rm = GetRoom()
    
    if not (my_rm[4] == tpInfo[2]): WalkToWp(tpInfo[2])
    
    while True:
        Run(tpInfo[0], tpInfo[1])
        
        ids = GetPortalIds()
        if not ids or not ids[0]:
            Sleep(0)
            continue
        
        id = ids[0]
    
        if IntPortal(id): break
        
        Sleep(0)

def SetActiveMercHire(hire=True):
    if (GetInvGold() + GetStashGold()) < 50000 or not hire: return False
    
    if IsTownLevel(GetMapLevelNo()): return False
    
    mercId = GetMercId()
    if mercId:
        if not IsDead(1, mercId): return False
    
    return True

def ActiveIdStash(stash=True):
    TpToTown()
    
    tpInfo = GetTpInfo()
    
    RemoteStash()
    IdentifyAll()
    RemoteStash()
    
    my_rm = GetRoom()
    
    if not (my_rm[4] == tpInfo[2]): WalkToWp(tpInfo[2])
    
    while True:
        Run(tpInfo[0], tpInfo[1])
        
        ids = GetPortalIds()
        if not ids or not ids[0]:
            Sleep(0)
            continue
        
        id = ids[0]
    
        if IntPortal(id): break
        
        Sleep(0)
    
def SetActiveIdStash():
    if GetFreeGrid(3, 2, 4): return False
    
    if IsTownLevel(GetMapLevelNo()): return False
    
    return True