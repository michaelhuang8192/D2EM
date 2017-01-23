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

char.target = [Data.Mephisto]

WalkToWp(Data.DuranceOfHateLevel2)
char.Precast()
TeleToWarp(Data.DuranceOfHateLevel3)
meph = GetStaticUnits(1, Data.Mephisto)
Teleport(meph[0][2], meph[0][3], 5)

while True:
    char.Attack()
    mephisto = GetNearbyUnits(1, Data.Mephisto)
    if not mephisto or not GetLife(1, mephisto[0][1]): break
    Sleep(10)

Sleep(500)
ItemCheck()

char.target = [Data.CouncilMember1, Data.CouncilMember2, Data.CouncilMember3]
char.Attack()
Sleep(500)
ItemCheck()
TpToTown()
 
DLog("End of Script: %s" % ("Meph", ), 0)

