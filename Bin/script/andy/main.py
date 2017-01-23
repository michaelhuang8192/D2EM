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

char.target = [Data.Andariel]

WalkToWp(Data.CatacombsLevel2)
char.Precast()
TeleToWarp(Data.CatacombsLevel3)
TeleToWarp(Data.CatacombsLevel4)
andy = GetStaticUnits(1, Data.Andariel)
Teleport(andy[0][2], andy[0][3], 5)

while True:
    char.Attack()
    andy = GetNearbyUnits(1, Data.Andariel)
    if not andy or not GetLife(1, andy[0][1]): break
    Sleep(10)
    
Sleep(500)
ItemCheck()
TpToTown()

DLog("End of Script: %s" % ("Andy", ), 0)
