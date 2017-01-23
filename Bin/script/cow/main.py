from D2EM import *
from D2EMExtended import *
from Belt import *
import Data
import Character
from ItemCheck import *
from Town import *
from Stash import *
from LifeCheck import *
from Cube import *
import Party
import OnWake_TownAPC

#config
COWKING_SAFE_DIST = 60
NEAR_ROOM_DIST = 80
#

def wait4_unit_exist(type, clsid, ms):
    tick = GetTickCount()
    t = GetNearbyUnits(type, clsid)
    while not t and GetTickInterval(tick) < ms:
        Sleep(50)
        t = GetNearbyUnits(type, clsid)

    if t: return t[0]
    return None

def int_unit_with_ak(type, clsid, timeout, int_func, int_func_params):
    t = wait4_unit_exist(type, clsid, timeout)
    if not t: return False
    tid = t[1]
    tpos = GetPos(type, tid)
    if not Teleport(tpos[0], tpos[1], 4, 20): return False
    
    char.target = char.target_name = char.xRange_skip = char.yRange_skip = char.xRange = char.yRange = ()
    char.xRange = (tpos[0] - 10, tpos[0] + 10)
    char.yRange = (tpos[1] - 10, tpos[1] + 10)
    for i in range(10):
        if int_func(tid, int_func_params): return True
        
        char.Attack()
        mpos = GetPos()
        if GetDistance(mpos[0], mpos[1], tpos[0], tpos[1]) > 4:
            if not Teleport(tpos[0], tpos[1], 4, 20): break
    
    return False

def int_portal_s(tid, params):
    if params[0] != GetMapLevelNo() or IntPortal(tid):
        return True
    else:
        return False

def EnterTristram():
    old_lvlno = GetMapLevelNo()
    int_unit_with_ak(2, 60, 5000, int_portal_s, (old_lvlno,))
    if old_lvlno == GetMapLevelNo(): ExitScript(1, "Failed To Enter Tristram")

def wait4_wbsc(ms=800):
    tick = GetTickCount()
    
    while True:
        Sleep(50)
        state = GetNearbyUnits(2, 268)[0][2]
        
        if state: return True
            
        if GetTickCount() - tick >= ms: return False
    
def IntWirtsBody(id):
    print "Interact wirt's body[0x%X]" % wirtsBody[1]
    
    IntUnit2(id)
    if wait4_wbsc():
        Sleep(200)
        return True
    
    return False

def int_wirts_body_s(tid, params):
    if IntWirtsBody(tid):
        Sleep(500)
        ItemCheck()
        return True
    else:
        return False
    
def GetWirtsLeg():
    if not int_unit_with_ak(2, 268, 5000, int_wirts_body_s, ()):
        ExitScript(1, "Failed To Get Wirt's Leg")
        
def wait4_tpbook(ms=800):
    tick = GetTickCount()
    
    while True:
        Sleep(50)
        if HasItemInInv(518): return True
            
        if GetTickCount() - tick >= ms: return False
        
def BuyTpBook():
    while 1:
        units = GetNearbyUnits(1, Data.Akara)
        
        if units:
            akara = units[0]
            RunToUnit(1, akara[1])  
            IntTownNPC(akara[1])
            ClickOnMenu(GetMenuIdx("trade"))
            
            item = GetItemsFromInv(5, 518, 1, akara[1])
            if item:
                while True:
                    BuyItem(akara[1], item[0][1], 0)
                    
                    if wait4_tpbook(): break
            
            else:
                PostEsc()
                print "Akara has no tp book, exit script!"
                ExitScript()
                
            print "Cancel interaction"
            PostEsc()
            break
        
        else:
            stash = GetStaticUnits(2, STASH_CLSID)[0]
            Run(stash[2]+67, stash[3]-19)
        
        Sleep(0)

def EnterCowTp():
    t = wait4_unit_exist(2, RED_TP_CLSID, 5000)
    if not t: ExitScript(1, "No Cow TP")
    
    tid = t[1]
    tpos = GetPos(2, tid)
    Run(tpos[0], tpos[1])
    old_lvlno = GetMapLevelNo()
    for i in range(10):
        IntPortal(tid)
        if GetMapLevelNo() != old_lvlno: break
        
    if old_lvlno == GetMapLevelNo(): ExitScript(1, "Failed To Enter Cow TP")

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

def get_cow_king_pos(au=None):
    if not au: au = GetNearbyUnits(1)
    
    cow_king[2] = None
    if au:
        for t in au:
            ex = GetUnitExtension(1, t[1])
            if ex and (ex[1] & 8) and GetUnitName(1, t[1]) == "The Cow King":
                ck_pos = GetPos(1, t[1])
                if ck_pos:
                    cow_king[0] = ck_pos[0]
                    cow_king[1] = ck_pos[1]
                    cow_king[2] = t[1]
                    return ck_pos
    
    return (cow_king[0], cow_king[1])

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
    
    tu = []
    au = GetNearbyUnits(1)
    if au:
        my_rm = GetRoom()
        ck_pos = get_cow_king_pos(au)
        
        for u in au:
            if cow_king[2] == u[1]: continue
            
            tpos = GetPos(1, u[1])
            if not tpos: continue
            
            if GetDistance(ck_pos[0], ck_pos[1], tpos[0], tpos[1]) >= COWKING_SAFE_DIST + 5:
                tu.append(u)
                
    return tu

def is_target(type, target):
    ck_pos = get_cow_king_pos()
    tpos = target[4]
    if GetDistance(ck_pos[0], ck_pos[1], tpos[0], tpos[1]) >= COWKING_SAFE_DIST:
        return True
    
    return False

def mark_as_target(type, target):
    r = GetRoom(1, target[1])
    rid = r[0] | (r[1] << 16)
    rm = arooms.get(rid)
    if rm:
        rm[0] = False

def _cow_king_detector_lcd():
    ck_pos = get_cow_king_pos()
    mpos = GetPos()
    
    dist = GetDistance(mpos[0], mpos[1], ck_pos[0], ck_pos[1])
    str = "CowKing: <\377c20x%04X, 0x%04X\377c8>, Dist[\377c90x%04X\377c8]" % (ck_pos[0], ck_pos[1], dist)
    str = "CRoom: (\377c9%d/%d\377c8)\n" % (_pd[0], len(arooms)) + str
    
    w, h = GetWindowSize()[:2]
    of_id = D2SelectFont(0x05)
    D2DrawText(str, w - 380, h - 150, 8, 0)
    D2SelectFont(of_id)
    
#-------------------------------------------------
AttachEvent(EVT_ONWAKE, OnWake_TownAPC.OnWake_TAPC)

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

charClass = {0:Character.CsAmazon,
             3:Character.HammerPaladin,
             5:Character.Druid,
             6:Character.HybridSin}

charType = GetCharClass()
char = charClass[charType]()

if GetQuestState(4, 10): ExitScript(0, "No Cow Quest!")

if not HasItemInInv(88) and not HasItemInCube(88):
    WalkToWp(Data.StonyField)
    char.Precast()
    cairnStone22 = GetStaticUnits(2, 22)[0]
    Teleport(cairnStone22[2], cairnStone22[3])
    EnterTristram()
    wirtsBody = GetStaticUnits(2, 268)[0]
    Teleport(wirtsBody[2], wirtsBody[3])
    GetWirtsLeg()
    TpToTown()

if GetMapLevelNo() != Data.RogueEncampment: WalkToWp(Data.RogueEncampment)

if not HasItemInInv(518) and not HasItemInCube(518):
    BuyTpBook()
    
if HasItemInInv(518) or HasItemInInv(88):
    RunToStash()
    IntStash()
    Sleep(500)
    
    invItems = GetInvItemIds()
    items = {}
    
    for i in invItems:
        unit = GetUnit(0, 4, i)
        
        if unit[0] in (518, 88):
            items[unit[0]] = i
    
    if len(items):
        for i in items:
            items[i]
            RemoveInvItem(items[i])
            
            if HasItemInCube(i): DropItem()
            
            else: AddItemToCube_S(items[i], HasCubeInStash())
    
    OpenCube()
    Sleep(250)
    Transmute()
    Sleep(500)
    CloseCube()
    Sleep(250)
    CloseStash()

if HasItemInCube(518) and HasItemInCube(88):
    RunToStash()
    IntStash()
    Sleep(500)
    OpenCube()
    Sleep(250)
    Transmute()
    Sleep(500)
    CloseCube()
    Sleep(250)
    CloseStash()

EnterCowTp()

AttachEvent(EVT_ONDRAW, _cow_king_detector_lcd)

_ck = GetStaticUnits(1, 773)[0]
cow_king = [ _ck[2], _ck[3], None ]
arooms = {}
_mypos = None
_pd = [0,]

for r in GetAllRooms():
    ept = get_room_enter_point(r)
    if ept:
        rid = r[0] | (r[1] << 16)
        cpt = (r[0] + r[2] / 2, r[1] + r[3] / 2)
        arooms[ rid ] = [ None, ept, cpt, rid, r ]
    else:
        print "Unreachable Room: ", r

bo_tick = GetTickCount()
char.target = char.target_name = char.xRange_skip = char.yRange_skip = char.xRange = char.yRange = ()
while True:
    char.Attack( target_callback=(get_targets, is_target, mark_as_target), pvd=(4,) )
    Sleep(200)
    ItemCheck()
    
    nroom = None
    while arooms and nroom == None:
        _mypos = GetPos()
        nroom = min(arooms.itervalues(), key=get_room_dist)
        print nroom
        if nroom[0]:
            nroom = None
            break
        
        if GetTickInterval(bo_tick) > 120000:
            char.Precast()
            bo_tick = GetTickCount()
        
        ept = nroom[1]
        if not Teleport(ept[0], ept[1], 15, 20):
            print "Can't Teleport To Room: ", nroom[4]
            del arooms[ nroom[3] ]
            nroom = None
        
        print nroom, "-"
    if not nroom: break
    
TpToTown()

DLog("End of Script: %s" % ("Cow", ), 0)
