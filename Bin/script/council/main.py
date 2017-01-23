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

char.target = [Data.CouncilMember1, Data.CouncilMember2, Data.CouncilMember3]

WalkToWp(Data.Travincal)
char.Precast()
object = GetStaticUnits(2, 404)
if object: Teleport(object[0][2], object[0][3])
char.Attack()
Sleep(500)
ItemCheck()
TpToTown()

DLog("End of Script: %s" % ("Council", ), 0)
