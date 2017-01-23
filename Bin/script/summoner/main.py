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


AttachEvent(EVT_ONWAKE, OnWake_TownAPC.OnWake_TAPC)
#AttachEvent(EVT_ONWAKE, Party.OnWake_Party)

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

char.target = [Data.Summoner]

WalkToWp(Data.ArcaneSanctuary)
char.Precast()
object = GetStaticUnits(2, 357)
if object: Teleport(object[0][2], object[0][3])
char.Attack()
Sleep(500)
ItemCheck()
Teleport(object[0][2], object[0][3])

#

def wait4_lvl_chg(ms, cur_lvl_no):
    tick = GetTickCount()
    while GetTickInterval(tick) < ms:
        lvlno = GetMapLevelNo()
        if lvlno and lvlno != cur_lvl_no: return True
        Sleep(10)

    return False


u2u5_cls_ids = {307: 44, 308: 40, 309: 43, 310: 41, 311: 42, 312: 39, 313: 38}
for u in GetNearbyUnits(2):
    if u2u5_cls_ids.has_key(u[0]):
        del u2u5_cls_ids[u[0]]

u5_cls_id = u2u5_cls_ids.values()[0]
print "Missing Tomb[%d]" % (u5_cls_id, )

tid = GetNearbyUnits(2, 357)[0][1]
redtp = None
while not redtp:
    print "Open RedTP"
    IntUnit2(tid)
    Sleep(500)
    PostKey(0x20)
    Sleep(500)
    redtp = GetNearbyUnits(2, 60)

redtp_id = redtp[0][1]
redtp_pos = GetPos(2, redtp_id)
Teleport(redtp_pos[0], redtp_pos[1])

old_lvlno = GetMapLevelNo()
for k in range(5):
    print "Int RedTP[%d]" % (redtp_id)
    IntPortal(redtp_id)
    if wait4_lvl_chg(800, old_lvlno): break

u5 = GetStaticUnits(5, u5_cls_id)[0]
Teleport(u5[2], u5[3])
Sleep(200)
IntUnit5( GetNearbyUnits(5, u5_cls_id)[0][1] )

u2 = GetStaticUnits(2, 152)[0]
if not Teleport(u2[2], u2[3], 4, 20): ExitScript(1)

u2 = None
tick = GetTickCount()
while not u2 and GetTickInterval(tick) < 5000:
    u2 = GetNearbyUnits(2, 100)
    Sleep(10)

tid = u2[0][1]
tpos = GetPos(2, tid)
if not Teleport(tpos[0], tpos[1], 4, 20): ExitScript(1)

char.target = char.target_name = char.xRange_skip = char.yRange_skip = ()
char.xRange = (tpos[0] - 10, tpos[0] + 10)
char.yRange = (tpos[1] - 10, tpos[1] + 10)
old_lvlno = GetMapLevelNo()
for k in range(5):
    print "Int Chamber[%d]" % (tid)
    IntPortal(tid)
    if wait4_lvl_chg(800, old_lvlno): break
    
    char.Attack()
    mpos = GetPos()
    if GetDistance(mpos[0], mpos[1], tpos[0], tpos[1]) > 4:
        if not Teleport(tpos[0], tpos[1], 4, 20): ExitScript(1)

if GetMapLevelNo() == old_lvlno:
    print "Failed To Int Chamber!"
    ExitScript(1)

tick = GetTickCount()
while not GetNearbyUnits(1, Data.Duriel) and GetTickInterval(tick) < 5000:
    Sleep(10)

char.target_name = char.xRange_skip = char.yRange_skip = char.xRange = char.yRange = ()
char.target = [Data.Duriel]
while True:
    char.Attack()
    duriel = GetNearbyUnits(1, Data.Duriel)
    if not duriel or not GetLife(1, duriel[0][1]): break
    Sleep(10)
    
Sleep(500)
ItemCheck()

TpToTown()

#Sleep(100000)
DLog("End of Script: %s" % ("Summoner", ), 0)
