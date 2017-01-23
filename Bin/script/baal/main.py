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
import OnPacketUpdate
import OnWake_TownAPC
import Leech


AttachEvent(EVT_ONWAKE, OnWake_TownAPC.OnWake_TAPC)
AttachEvent(EVT_ONPACKETUPDATE, OnPacketUpdate.OnPacketUpdate)

charClass = {0:Character.CsAmazon,
             3:Character.HammerPaladin,
             5:Character.Druid,
             6:Character.HybridSin}

charType = GetCharClass()

char = charClass[charType]()

def Init():
    global offset, rhs, castSide, ngMsg, kbMsg, pc_delay, pc_count, tp_offset, pt_m_offset, cast_pos
    
    ngMsgs = ("NG!", "Next Game", "ng", "Ng in 10 sec", "next game")
    kbMsgs = ("baal", "Kill Baal!", "Baal now!", "kill baal", "Baal", "baal!")
    
    ngMsg = ngMsgs[GetTickCount() % len(ngMsgs)]
    kbMsg = kbMsgs[GetTickCount() % len(kbMsgs)]
    
    tp_offset = ((27, -10), (27, 61), (-18, -9), (-18, 60))
    pt_bm_offset = (4, 33)
    pt_um_offset = (6, 20)
    offset = (0, 0)
    pt_m_offset = (offset,)
    rhs = 0
    castSide = 1
    pc_delay = 0
    pc_count = 5
    cast_pos = None
    
    if charType == 0:
        rhs = Data.LightningFury
        #pt_m_offset = ((0, 0), pt_bm_offset, (-13, 25), (22, 25), (19, 15), (12, 3))
        pt_m_offset = (pt_bm_offset,)
        offset = pt_bm_offset
        cast_pos = (15090,5024)
        pc_delay = 1500
        pc_count = 3
        
    elif charType == 3:
        pt_m_offset = (pt_um_offset,)
        offset = pt_um_offset
        rhs = Data.Concentration
        castSide = 0
    
    elif charType == 5:
        pt_m_offset = (pt_bm_offset,)
        offset = pt_bm_offset
        
    elif charType == 6:
        offset = pt_bm_offset
        pt_m_offset = (pt_bm_offset,)
        rhs = Data.LightningSentry
        
    DLog(OnPacketUpdate.baal)

def GetNearbyMonsters(in_range=((),())):
    units = []
        
    all_units = GetNearbyUnits(1)
    if not all_units: return units

    my_rm = GetRoom()
            
    for u in all_units:
        uid = u[1]
               
        if not my_rm or my_rm[4] != u[4] or IsTownLevel(u[4]): continue
                
        life = GetLife(1, uid)
        if not life or not IsAttackable(1, uid): continue
        
        if len(in_range[0]) and len(in_range[1]):
            xRange = in_range[0]
            yRange = in_range[1]
            
            u_pos = GetPos(1, uid)
            if u_pos and not (xRange[0] <= u_pos[0] <= xRange[1]): continue
            
            if u_pos and not (yRange[0] <= u_pos[1] <= yRange[1]): continue
        
        units.append(uid)
    
    return units

def ThroneConditionAlert(in_range):
    if len(GetPlayerIds()) <= 1: return
    
    units = GetNearbyMonsters(in_range)
    if not len(units): return
    
    if len(units) >= 40:
        SendMessage("Throne is messy, be careful! [# of M's: %d]" % (len(units), ))
        return
    
    msg = False
    
    soulKillers = (Data.SoulKiller1, Data.SoulKiller2, Data.SoulKiller3, Data.SoulKiller4)
    burningSouls = (Data.BurningSoul1, Data.BurningSoul2, Data.BurningSoul3)
    dMonsters = {soulKillers:0, burningSouls:0}
    for uid in units:
        unit = GetUnit(0, 1, uid)
        if units:
            for dm in dMonsters:
                if unit[0] in dm: dMonsters[dm] += 1
                
    if dMonsters[burningSouls] >= 8: msg = "%d BurningSouls in throne, watch out!" % (dMonsters[burningSouls], )
    
    elif dMonsters[soulKillers] >= 8: msg = "%d SoulKiller in throne, watch out!" % (dMonsters[soulKillers], )
    
    if msg: SendMessage(msg)
    
def GetSafeTpPos(ref):
    unitIds = GetNearbyMonsters()
    
    tpPos = (ref[0] + tp_offset[0][0], ref[1] + tp_offset[0][1])
    if not len(unitIds): return tpPos
    
    lmc = 100
    for tos in tp_offset:
        mc = 0
        for uid in unitIds:
            upos = GetPos(1, uid)
            if not upos: continue
            
            dist = GetDistance(ref[0] + tos[0], ref[1] + tos[1], upos[0], upos[1])
            if dist <= 20: mc += 1
        
        if mc == 0:
            tpPos = (ref[0] + tos[0], ref[1] + tos[1])
            DLog("break, found safe tp pos (%d, %d)" % (tos[0], tos[1]), 0)
            break
        
        if mc < lmc:
            DLog("Set new safe tp position")
            lmc = mc
            tpPos = (ref[0] + tos[0], ref[1] + tos[1])
            
    return tpPos

def HasTpInThrone(ref_pt):
    if GetDistance(GetPos()[0], GetPos()[1], ref_pt[0], ref_pt[1]) <= 70:
        pids = GetPortalIds()
        
        if pids:
            for pid in pids:
                ownerId = GetPortalOwnerId(pid)
                
                if ownerId == GetCurrentPlayerId():
                    tpPos = GetPos(2, pid)
                    if tpPos:
                        #dist = GetDistance(GetPos()[0], GetPos()[1], tpPos[0], tpPos[1])
                        #DLog("Found tp in Throne at pos (0x%X, 0x%X)" % (tpPos[0], tpPos[1]), 0)
                        #DLog("Distance from BaalThrone %d" % (dist, ), 0)
                        return True
            
            return False
        
    else: return True

def WaitForBaalMinions(ref_pt):
    global offset
    sk285 = sk286 = 0
    minionLeaders = ("Colenzo the Annihilator", "Achmel the Cursed", "Bartuc the Bloody", "Ventar the Unholy", "Lister the Tormentor")
    
    while True:
        my_pos = GetPos()
        if not OnPacketUpdate.baal["id"]:
            unit = GetNearbyUnits(1, 543)
            if unit:
                DLog("Set Baal ID: 0x%X" % (unit[0][1], ), 0)
        
                OnPacketUpdate.baal["id"] = unit[0][1]
        
        else:
            if not GetName(1, OnPacketUpdate.baal["id"]) and GetDistance(my_pos[0], my_pos[1], BaalThrone[2], BaalThrone[3]) <= 45:
                DLog("Baal walks into throne!", 0)
                break
            
        if len(GetPlayerIds()) > 1 and not HasTpInThrone(ref_pt): CastTp(500, False)
            
        if sk285 != OnPacketUpdate.baal[285]:
            sk285 = OnPacketUpdate.baal[285]
            if (sk285 == 1 or sk285 >= 6) and not OnPacketUpdate.barbBo : char.Precast()
            
            if sk285 < 6:
                offset = pt_m_offset[GetTickCount() % len(pt_m_offset)]
                
            DLog("BaalCorpseExplode %d" % (sk285, ), 0)
        
        if sk286 != OnPacketUpdate.baal[286]:
            sk286 = OnPacketUpdate.baal[286]
            DLog("BaalMonsterSpawn %d" % (sk286, ), 0)
            
            if rhs:
                if GetSkillUIdBySide(1)[0] != rhs: SelectSkill(rhs, 1)
                
                if cast_pos: cp = cast_pos
                
                else: cp = (my_pos[0], my_pos[1])
                
                if GetDistance(my_pos[0], my_pos[1], ref_pt[0]+offset[0], ref_pt[1]+offset[1]) <= 5: 
                    pc_tick = GetTickCount()
                    timeout = False
                    
                    while (GetTickCount() - pc_tick) <= pc_delay and not timeout:
                        if cast_pos:
                            bmUnits = GetNearbyMonsters()
                            if bmUnits:
                                for bmuId in bmUnits:
                                    if GetName(1, bmuId) in minionLeaders:
                                        bmuPos = GetPos(1, bmuId)
                                        if bmuPos:
                                            DLog("--- Resetting CP! ---")
                                            cp = bmuPos
                                            timeout = True
                                            break
                        
                        Sleep(50)
                        
                    for i in range(0, pc_count):
                        CastSkill(cp[0], cp[1], castSide)
                        Sleep(500)
                        
            if sk286 >= 5: offset = (0, 0)
            
        char.Attack()
        
        if GetDistance(my_pos[0], my_pos[1], ref_pt[0]+offset[0], ref_pt[1]+offset[1]) > 3:
            DLog("Teleport under baal minions", 0)
            Teleport(ref_pt[0]+offset[0], ref_pt[1]+offset[1])
        
        Sleep(0)
        
def EnterChamber():
    while GetMapLevelNo() != Data.TheWorldstoneChamber:
        unit = GetNearbyUnits(2, 563)
        if unit:
            id = unit[0][1]
            pos = GetPos(2, id)
            
            Teleport(pos[0], pos[1])
            if IntPortal(id): break           
            
        Sleep(0)
        
def KillBaal():
    baal544id = False
    if len(GetPlayerIds()) > 1 and (GetTickCount() % 2): SendMessage(kbMsg)
    
    while 1:
        if not baal544id:
            unit = GetNearbyUnits(1, 544)
            if unit: baal544id = unit[0][1]
                
        if baal544id and not GetLife(1, baal544id): break
        
        char.Attack()

GetBody()
CursorCheck()

Init()
    
TpToTown()

Shop()
HireMerc()
RepairAll()
RemoteStash()
IdentifyAll()
RemoteStash()
StashGold()

char.xRange = (0x3ae0, 0x3b0e)
char.yRange = (0x138a, 0x13e4)

WalkToWp(Data.TheWorldStoneKeepLevel2)
char.Precast()
TeleToWarp(Data.TheWorldStoneKeepLevel3)
TeleToWarp(Data.ThroneOfDestruction)
BaalThrone = GetStaticUnits(1, 543)[0]
Teleport(BaalThrone[2]+4, BaalThrone[3]+33)
DLog("BaalThone Pos: (0x%X, 0x%X)" % (BaalThrone[2], BaalThrone[3]), 0)

ThroneConditionAlert((char.xRange, char.yRange))

safeTpPos = GetSafeTpPos((BaalThrone[2], BaalThrone[3]))
Teleport(safeTpPos[0], safeTpPos[1])

WaitForBaalMinions((BaalThrone[2],BaalThrone[3]))
EnterChamber()

char.xRange = []
char.yRange = []
char.target = [544]

KillBaal()

ic_tick = GetTickCount()
while (GetTickCount() - ic_tick) <= 1000:
    ItemCheck()
    Sleep(0)

if len(GetPlayerIds()) > 1: SendMessage(ngMsg)
    
DLog("End of Script: %s" % ("Baal", ), 0)


 
    