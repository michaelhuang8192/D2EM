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
AttachEvent(EVT_ONPACKETUPDATE, OnPacketUpdate.OnPacketUpdate)

NEAR_ROOM_DIST = 60
clearedRooms = []

def numeric_compare0(x, y):
    if x[0] > y[0]:
      return 1
    elif x[0] == y[0]:
       return 0
    else: 
       return -1

def numeric_compare1(x, y):
    if x[1] > y[1]:
      return 1
    elif x[1] == y[1]:
       return 0
    else: 
       return -1

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

    monList = []
    nbms = GetNearbyUnits(1)
    
    skip_mons = (Data.Devilkin1,)
    
    for m in nbms:
        #if m[0] in skip_mons: continue
        uex = GetUnitExtension(1, m[1])
        if uex and (uex[1] & 8):
            monList.append(m)
        
    return monList

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

def GetEdgeRooms(lvl):
    edgeRooms = []
    
    for r in GetAllRooms(1):
        for t in r[5]:
            if t[4] == lvl:
                edgeRooms.append(r)
                break
    
    return edgeRooms

def GetDirection(edgeRooms, destLvl):
    deltaX = None
    deltaY = None
    x =  None
    y = None
    roomList = []
    for er in edgeRooms:
        roomList.append(er)
        
    for er in edgeRooms:
        rt = er[5]
        for r in rt:
            if r[4] == destLvl:
                if er[0] == r[0]:
                    deltaX = 0
                    break
                
                if er[1] == r[1]:
                    deltaY = 0
                    break
        
        if deltaX == 0:
            if er[1] > r[1]: deltaY = -1
            
            else: deltaY = 1
            
            if deltaY > 0:
                y = er[1] + 38 * deltaY
            else: y = er[1] + 1
            roomList.sort(cmp=numeric_compare0)
            xr = (roomList[0][0], roomList[len(roomList)-1][0])

            for c in range(xr[0], xr[1]):
                point = (c+1, y)
                pt = GetPointInMap(point[0], point[1])
                if pt != None and not (pt & 0xc2f):
                    x = point[0]
                    break

            break
            
        elif deltaY == 0:
            if er[0] > r[0]: deltaX = -1
            
            else: deltaX = 1
            
            if deltaX > 0:
                x = er[0] + 38 * deltaX
            else: x = er[0] + 1
            roomList.sort(cmp=numeric_compare1)
            yr = (roomList[0][1], roomList[len(roomList)-1][1])

            for c in range(yr[0], yr[1]):
                point = (x, c+1)
                pt = GetPointInMap(point[0], point[1])
                if pt != None and not (pt & 0xc2f):
                    y = point[1]
                    break

            break
    
    return ((x, deltaX), (y, deltaY))
        
GetBody()
CursorCheck()

charClass = {0:Character.CsAmazon,
             3:Character.HammerPaladin,
             5:Character.Druid,
             6:Character.HybridSin}

charType = GetCharClass()

char = charClass[charType]()

TpToTown()

Shop()
HireMerc()
RepairAll()
RemoteStash()
IdentifyAll()
RemoteStash()
StashGold()

WalkToWp(Data.BlackMarsh)
char.Precast()

edgeRooms = GetEdgeRooms(7)
cdn = GetDirection(edgeRooms, 7)
Teleport(cdn[0][0], cdn[1][0])
Teleport(cdn[0][0]+10*cdn[0][1], cdn[1][0]+10*cdn[1][1])

DLog("tele to pitlevel1", 0)
TeleToWarp(Data.PitLevel1)

pitRooms1 = GetAllRooms()

AttachEvent(EVT_ONDRAW, _croom_monitor)

_mypos = None
arooms = {}
_pd = [0,]

for r in pitRooms1:
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
        
        if GetTickInterval(bo_tick) > 120000:
            char.Precast()
            bo_tick = GetTickCount()
        
        ept = nroom[1]
        if not Teleport(ept[0], ept[1], 15, 20):
            #print "Can't Teleport To Room: ", nroom[4]
            del arooms[ nroom[3] ]
            nroom = None
        
        #print nroom
        
    if not nroom: break

TeleToWarp(Data.PitLevel2)
su = GetStaticUnits(2, 397)
Teleport(su[0][2], su[0][3])
char.Attack( target_callback=(get_targets, is_target, mark_as_target), pvd=(4,) )
Sleep(200)
ItemCheck()
TpToTown()
 
DLog("End of Script: %s" % ("Pit", ), 0)

