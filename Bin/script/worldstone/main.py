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
import OnPacketUpdate


AttachEvent(EVT_ONWAKE, OnWake_TownAPC.OnWake_TAPC)
AttachEvent(EVT_ONWAKE, OnWake_TownAPC.OnWake_TAPC_Stash)
AttachEvent(EVT_ONPACKETUPDATE, OnPacketUpdate.OnPacketUpdate)

charClass = {0:Character.CsAmazon,
             3:Character.HammerPaladin,
             5:Character.Druid,
             6:Character.HybridSin}

charType = GetCharClass()

char = charClass[charType]()

def Init():
    global offset, rhs, castSide, ngMsg, kbMsg, pc_delay, pc_count, tp_offset, pt_m_offset, NEAR_ROOM_DIST
    
    NEAR_ROOM_DIST = 60
    
    ngMsgs = ("NG!", "Next Game", "ng", "Ng in 10 sec", "next game")
    kbMsgs = ("baal", "Kill Baal!", "Baal now!", "kill baal", "Baal", "baal!")
    
    ngMsg = ngMsgs[GetTickCount() % len(ngMsgs)]
    kbMsg = kbMsgs[GetTickCount() % len(kbMsgs)]
    
    tp_offset = ((27, -10), (-18, -9), (27, 7), (-18, 7))
    pt_bm_offset = (4, 33)
    pt_um_offset = (6, 20)
    offset = (0, 0)
    pt_m_offset = (offset,)
    rhs = 0
    castSide = 1
    pc_delay = 0
    pc_count = 5
    
    if charType == 0:
        #rhs = Data.LightningFury
        pt_m_offset = ((0, 0), pt_bm_offset, (-13, 25), (22, 25), (19, 15), (12, 3))
        pc_delay = 1250
        pc_count = 3
        
    elif charType == 3:
        pt_m_offset = (pt_um_offset,)
        offset = pt_um_offset
        rhs = Data.Concentration
        castSide = 0
    
    elif charType == 6:
        offset = pt_bm_offset
        pt_m_offset = (pt_bm_offset,)
        rhs = Data.LightningSentry
        
    DLog(OnPacketUpdate.baal)

def GetNearbyMonsters():
    units = []
        
    all_units = GetNearbyUnits(1)
    if not all_units: return units

    my_rm = GetRoom()
            
    for u in all_units:
        uid = u[1]
                    
        if not my_rm or my_rm[4] != u[4] or IsTownLevel(u[4]): continue
                
        life = GetLife(1, uid)
        if not life or not IsAttackable(1, uid): continue
                
        units.append(uid)
    
    return units

def GetSafeTpPos(ref):
    unitIds = GetNearbyMonsters()
    
    tpPos = (ref[0] + tp_offset[0][0], ref[1] + tp_offset[0][1])
    if not len(unitIds): return tpPos
    
    lmc = 0
    for tos in tp_offset:
        mc = 0
        for uid in unitIds:
            upos = GetPos(1, uid)
            if not upos: continue
            
            dist = GetDistance(ref[0] + tos[0], ref[1] + tos[1], upos[0], upos[1])
            if dist <= 13: mc += 1
        
        if mc == 0:
            tpPos = (ref[0] + tos[0], ref[1] + tos[1])
            DLog("break, found safe tp pos (%d, %d)" % (tos[0], tos[1]), 0)
            break
        
        if mc < lmc:
            lmc = mc
            tpPos = (ref[0] + tos[0], ref[1] + tos[1])
            
    return tpPos

def HasTpInThrone(ref_pt):
    if GetDistance(GetPos()[0], GetPos()[1], ref_pt[0], ref_pt[1]) <= 60:
        pids = GetPortalIds()
        
        if pids:
            for pid in pids:
                ownerId = GetPortalOwnerId(pid)
                
                if ownerId == GetCurrentPlayerId():
                    tpPos = GetPos(2, pid)
                    if tpPos:
                        dist = GetDistance(GetPos()[0], GetPos()[1], tpPos[0], tpPos[1])
                        #DLog("Found tp in Throne at pos (0x%X, 0x%X)" % (tpPos[0], tpPos[1]), 0)
                        #DLog("Distance from BaalThrone %d" % (dist, ), 0)
                        return True
            
            return False
        
    else: return True

def WaitForBaalMinions(ref_pt):
    global offset
    sk285 = sk286 = 0
    
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
            if sk286 >= 5: offset = (0, 0)
            DLog("BaalMonsterSpawn %d" % (sk286, ), 0)
            
            if rhs:
                Sleep(pc_delay)
                if GetSkillUIdBySide(1)[0] != rhs: SelectSkill(rhs, 1)
                
                for i in range(0, pc_count):
                    CastSkill(my_pos[0], my_pos[1], castSide)
                    Sleep(500)
        
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
        
def get_room_dist(rm):
    if rm[0]: return 0xFFFFFFFF
    
    cpt = rm[2]
    return GetDistance(cpt[0], cpt[1], _mypos[0], _mypos[1])

def _is_reachable(x, y):
    pt = GetPointInMap(x, y)
    if pt == None or (pt & 0x0C01): return False
    
    pt = GetPointInMap(x + 1, y)
    if pt == None or (pt & 0x0C01): return False
    
    pt = GetPointInMap(x - 1, y)
    if pt == None or (pt & 0x0C01): return False
    
    pt = GetPointInMap(x, y + 1)
    if pt == None or (pt & 0x0C01): return False
    
    pt = GetPointInMap(x, y - 1)
    if pt == None or (pt & 0x0C01): return False
    
    return True

def get_room_enter_point(room):
    x = room[0] + 1
    sx = room[2] - 1
    y = room[1] + 1
    sy = room[3] - 1
    
    dx = sx / 2
    xinc = 1
    a = 0
    while a < sx:
        nx = x + dx
        dx += xinc
        a += 1
        if dx >= sx:
            dx = sx / 2 - 1
            xinc = -1
        
        dy = sy / 2
        yinc = 1
        b = 0
        while b < sy:
            ny = y + dy
            dy += yinc
            b += 1
            if dy >= sy:
                dy = sy / 2 - 1
                yinc = -1
            
            if _is_reachable(nx, ny): return (nx, ny)
        
    return None

def mark_near_rooms():
    _pd[0] = 0
    pos = GetPos()
    for r in arooms.itervalues():
        cpt = r[2]
        if GetDistance(cpt[0], cpt[1], pos[0], pos[1]) <= NEAR_ROOM_DIST:
            r[0] = True
        elif r[0]:
            _pd[0] += 1
        
def get_targets(type):
    mark_near_rooms()
    
    return GetNearbyUnits(1)
    
def is_target(type, target):
    return True

def mark_as_target(type, target):
    r = GetRoom(1, target[1])
    rid = r[0] | (r[1] << 16)
    rm = arooms.get(rid)
    if rm:
        rm[0] = False
    
def _croom_monitor():
    str = "CRoom: (\377c9%d/%d\377c8)\n" % (_pd[0], len(arooms))
    
    w, h = GetWindowSize()[:2]
    of_id = D2SelectFont(0x05)
    D2DrawText(str, w - 380, h - 150, 8, 0)
    D2SelectFont(of_id)
    
Init()

GetBody()
CursorCheck()

TpToTown()

Shop()
HireMerc()
RepairAll()
RemoteStash()
IdentifyAll()
RemoteStash()
StashGold()

WalkToWp(Data.TheAncientsWay)
char.Precast()

TeleToWarp(Data.ArreatSummit)
destLvls = (Data.TheWorldStoneKeepLevel1, Data.TheWorldStoneKeepLevel2, Data.TheWorldStoneKeepLevel3, Data.ThroneOfDestruction)

for dlvl in destLvls:
    TeleToWarp(dlvl)
    
    roomz = GetAllRooms()

    AttachEvent(EVT_ONDRAW, _croom_monitor)
    
    _mypos = None
    arooms = {}
    _pd = [0,]
    
    for r in roomz:
        ept = get_room_enter_point(r)
        if ept:
            rid = r[0] | (r[1] << 16)
            cpt = (r[0] + r[2] / 2, r[1] + r[3] / 2)
            arooms[ rid ] = [ None, ept, cpt, rid, r ]
        else:
            DLog("Unreachable Room: ", 2)
            DLog(r, 2)
    
    bo_tick = GetTickCount()
    while True:
        char.Attack( target_callback=(get_targets, is_target, mark_as_target), pvd=(4,) )
        Sleep(200)
        ItemCheck()

        nroom = None
        while arooms and nroom == None:
            _mypos = GetPos()
            nroom = min(arooms.itervalues(), key=get_room_dist)
            if nroom[0]:
                nroom = None
                break
            
            if GetTickInterval(bo_tick) > 90000:
                char.Precast()
                bo_tick = GetTickCount()
            
            ept = nroom[1]
            if not Teleport(ept[0], ept[1], 15, 20):
                #DLog("Can't Teleport To Room: ")
                #DLog(nroom[4])
                del arooms[ nroom[3] ]
                nroom = None
            
            #DLog(nroom)
            
        if not nroom: break
    
    if not HasAura(0, 0, 32): char.Precast()
        
    #DLog("Detach event!")
    DetachEvent(EVT_ONDRAW, _croom_monitor)

char.xRange = [0x3ae0, 0x3b0e]
char.yRange = [0x138a, 0x13e4]

BaalThrone = GetStaticUnits(1, 543)[0]
Teleport(BaalThrone[2], BaalThrone[3])
DLog("BaalThone Pos: (0x%X, 0x%X)" % (BaalThrone[2], BaalThrone[3]), 0)

safeTpPos = GetSafeTpPos((BaalThrone[2], BaalThrone[3]))
Teleport(safeTpPos[0], safeTpPos[1])

WaitForBaalMinions((BaalThrone[2],BaalThrone[3]))
EnterChamber()

DetachEvent(EVT_ONWAKE, OnWake_TownAPC.OnWake_TAPC_Stash)

char.xRange = []
char.yRange = []
char.target = [544]

KillBaal()
ItemCheck()
Sleep(500)
ItemCheck()
 
DLog("End of Script: %s" % ("WorldStone", ), 0)