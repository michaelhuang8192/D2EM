from D2EM import *
from D2EMExtended import *
from Belt import *
import Data
import Character
from ItemCheck import *
from Town import *
from Stash import *
from LifeCheck import *
import Party
import OnWake_TownAPC


def EnterRedTp():
    redTp = GetNearbyUnits(2, RED_TP_CLSID)
    if not redTp: ExitScript(1, "No Red Tp")
    
    tid = redTp[0][1]
    tpos = GetPos(2, tid)
    Run(tpos[0], tpos[1])
    
    old_lvlno = GetMapLevelNo()
    for i in range(10):
        IntPortal(tid)
        if GetMapLevelNo() != old_lvlno: break
        
    if old_lvlno == GetMapLevelNo(): ExitScript(1, "Failed To Enter Red TP")

def GatherCorpse():
    corpses = []
    if IsInTown(): return corpses

    my_rm = GetRoom()
    my_pos = GetPos()
    
    units = GetNearbyUnits(1)
    for u in units:
        if not my_rm or my_rm[4] != u[4] or IsTownLevel(u[4]): continue
        
        unit_pos = GetPos(1, u[1])
        if not unit_pos: continue
        dist = GetDistance(my_pos[0], my_pos[1], unit_pos[0], unit_pos[1])
        if dist >= 35: continue
        
        life = GetLife(1, u[1])
        if not life and IsAttackable(1, u[1]): corpses.append(u[1])
    
    return corpses

charType = GetCharClass()

GetBody()
CursorCheck()
    
TpToTown()
    
Shop()

if GetCurLevelNo() != Data.Harrogath: WalkToWp(Data.Harrogath)

#clawClsIds = range(175, 196)
while 1:
    RemoteStash()
    anya = GetNearbyUnits(1, 512)
    if not anya:
        Run(5112, 5118)
        Sleep(0)
        continue
    
    anyaId = anya[0][1]
    RunToUnit(1, anyaId)
    IntTownNPC(anyaId)
        
    ClickOnMenu(GetMenuIdx("trade"))
    Sleep(1000)
        
    items = GetItemsFromInv(-1, -1, 1, anyaId)
    for i in items:
        if IsInList(i[1]): BuyItem(anyaId, i[1], 0)
    
    PostEsc()
    
    EnterRedTp()
    
    corpseIds = GatherCorpse()
    if charType == 2 and corpseIds:
        for c in corpseIds:
            if GetSkillUIdBySide(1)[0] != Data.RaiseSkeleton: SelectSkill(Data.RaiseSkeleton, 1)
            CastSkillOnUnit(1, c, True)
            Sleep(400)
            
    EnterRedTp()
        
    Sleep(0)


DLog("End of Script: %s" % ("Clawbot", ), 0)
