from D2EM import *
import Data
import time

def OnDraw():
    wz = GetWindowSize()
    
    life = GetStat(0x06)
    max_life = GetStat(0x07)
    mana = GetStat(0x08)
    max_mana = GetStat(0x09)
    if life and max_life and mana and max_mana:
        str = "%04d/%04d" % (life[1] >> 8, max_life[1] >> 8)
        TextOut(38, wz[1] - 60, str, 0x00ffffff)
    
        str = "%04d/%04d" % (mana[1] >> 8, max_mana[1] >> 8)
        TextOut(wz[0] - 100, wz[1] - 60, str, 0x00ffffff)
    
    pos = GetPos()
    if pos:
        #str = "<%04X,%04X>" % (pos[0], pos[1])
        str = "<%d,%d>" % (pos[0], pos[1])
        TextOut(wz[0] / 2 + wz[2] - 40, wz[1] / 2, str, 0x00ffffff)
        

def WalkToWp(level):
    wpClsId = WP_CLSID[GetActNo()-1]
    
    while True:
        wpUnit = GetNearbyUnits(2, wpClsId)
        if wpUnit:
            wpId = wpUnit[0][1]
            wpPos = GetPos(2, wpId)
            myPos = GetPos()
            
            if myPos and (GetDistance(wpPos[0], wpPos[1], myPos[0], myPos[1]) <= 4):
                break
            
            else: DLog("Too far away from wp, use static pos", 0)
            
            Run(wpPos[0], wpPos[1])
            Sleep(0)
            continue

        staticWp = GetStaticUnits(2, wpClsId)
        staticWpPos = [staticWp[0][2], staticWp[0][3]]
        Run(staticWpPos[0], staticWpPos[1])
        Sleep(0)
            
    IntWayPoint(wpId, WP_DST.index(level))
        
        
def GetCurLevelNo():
    my_rm = GetRoom()
    if my_rm: return my_rm[4]
    
    else: return 0
    
    
def TeleToWarp(destLevel):
    cls_id = FindWarpClsId(destLevel)
    
    if not cls_id:
        DLog("Fail to find warp clsId, dest lvl [%d]" % (destLevel, ), 3)
        return
    
    DLog("Tele to warp %d" % (cls_id, ), 0)
    warp = GetStaticUnits(5, cls_id)
    
    if not warp:
        DLog("Fail to find warp, dest lvl [%d]" % (destLevel, ), 3)
        return
    
    if not Teleport(warp[0][2], warp[0][3]): return
    
    unit = GetNearbyUnits(5, cls_id)
    
    if unit: IntUnit5(unit[0][1])
        
    else: DLog("Cannot find warp unit [%d]" % (destLevel, ), 3)
        
def HasTSC():
    skill = GetSkill(0xDC)
    if skill and not (skill[3] == 0): return True
    
    else: return False
    

def CastTp(ms=800, OnRecvWake=True):
    while True:
        if not HasTSC():
            DLog("Has No Tp scroll, fail to cast tp. Exit Game!", 1)
            ExitGame()
    
        if GetSkillUIdBySide(1)[0] != Data.BookOfTownportal:
            SelectSkill(Data.BookOfTownportal, 1)
        
        myPos = GetPos()
        
        CastSkill(myPos[0], myPos[1], 1)
        Sleep(0)
        DLog("Cast Tp!", 0)
        
        tpId = wait4_tp(ms, OnRecvWake)
        if tpId:
            DLog("Cast tp successfully, 0x%X" % (tpId, ), 0)
            Sleep(200)
            return tpId
        
def wait4_tp(ms, OnRecvWake):
    tick = GetTickCount()
    
    while True:
        Sleep(50)
        ids = GetPortalIds()
        
        if ids and ids[0]:
            me_pos = GetPos()
            pt_pos = GetPos(2, ids[0])
            
            if pt_pos and GetDistance(me_pos[0], me_pos[1], pt_pos[0], pt_pos[1]) < 10:
                return True
            
        if GetTickCount() - tick >= ms:
            return False
        
def TpToTown():
    while not IsTownLevel(GetCurLevelNo()):
        CastTp()
        ids = GetPortalIds()
        if not ids or not ids[0]:
            DLog("No portal id!", 0)
            Sleep(0)
            continue
        
        id = ids[0]
    
        if IntPortal(id):
            DLog("Int portal successfully 0x%X" % (id, ), 0)
            break

def HasCta():
    equipPos = GetMapFromInv(0)[1][1:]
    equipNum = [3, 4, 10, 11]
    
    for n in equipNum:
        stats = GetAllStats(4, equipPos[n])
        skillIdx = [6357138L, 6357141L, 6357147L]
        
        if stats:
            for s in stats:
                if s[0] in skillIdx:
                    skillIdx.remove(s[0])
                    
                    if not len(skillIdx):
                        #DLog("Has cta", 0)
                        return equipPos[n]
    
    DLog("You Have No Cta!", 2)
    return False

def HasTpTome():
    invItems = GetItemsFromInv(2)
    
    if invItems:
        for i in invItems:
            if i[0] == 518: return True
    
    return False
    
def GetNpcResist(idx, id):
    resist = GetStat(idx, 0, 1, id)
    if not resist: return 0
    
    if not (resist[1] & 0x80000000): return resist[1]
    
    else: return ((resist[1] ^ 0xFFFFFFFF) + 1) * (-1)
    
def FindWarpClsId(lvl_no):
    warps = GetStaticUnits(5)
    for w in warps:
        if w[4] == lvl_no: return w[1]
        
    return None

def GetCharClass(id=0):
    player = GetPlayer(id)
    if player:
        return player[1]
        
    else: DLog("Fail to get player class", 1)

def GetBody():
    cids = []

    while True:
        cids = GetCorpseIds()
        
        if not cids: break
        
        Loot(cids[0])
        
        DLog("Loot", 0)
        
        if Wait4GetBody():
            open("c:\\%s-%s.txt" % (GetName(0,0), time.ctime().replace(":", "-")), "w").close()
            break
        
def Wait4GetBody():
    tick = GetTickCount()
    
    while True:
        Sleep(50)
        cids = GetCorpseIds()
        
        if not cids: return True
            
        if GetTickCount() - tick >= 800:
            return False
        
        