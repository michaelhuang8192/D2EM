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
import D2Memory


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

WalkToWp(Data.CanyonOfTheMagi)
char.Precast()

tomb_clsid = D2Memory.GetMissingTomb()
if not tomb_clsid:
    print "Can't Find The Missing Tomb!"
    ExitScript(1)

print "Missing Tomb %s" % (tomb_clsid, )
tomb = GetStaticUnits(5, tomb_clsid[1])[0]
Teleport(tomb[2], tomb[3])
Sleep(200)
IntUnit5( GetNearbyUnits(5, tomb_clsid[1])[0][1] )

nc = GetStaticUnits(2, 152)[0]
if not Teleport(nc[2], nc[3], 4, 20):
    print "Teleport Error!"
    ExitScript(1)

chamber = None
tick = GetTickCount()
while not chamber and GetTickInterval(tick) < 5000:
    chamber = GetNearbyUnits(2, 100)
    Sleep(10)

chamber_id = chamber[0][1]
chamber_pos = GetPos(2, chamber_id)
if not Teleport(chamber_pos[0], chamber_pos[1], 4, 20): ExitScript(1)

char.target = char.target_name = char.xRange_skip = char.yRange_skip = ()
char.xRange = (chamber_pos[0] - 10, chamber_pos[0] + 10)
char.yRange = (chamber_pos[1] - 10, chamber_pos[1] + 10)
old_lvlno = GetMapLevelNo()
for k in range(10):
    print "Int Chamber[%d]" % (chamber_id)
    IntPortal(chamber_id)
    if GetMapLevelNo() != old_lvlno: break
    
    char.Attack()
    mpos = GetPos()
    if GetDistance(mpos[0], mpos[1], chamber_pos[0], chamber_pos[1]) > 4:
        if not Teleport(chamber_pos[0], chamber_pos[1], 4, 20): ExitScript(1)

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

DLog("End of Script: %s" % ("Duriel", ), 0)
