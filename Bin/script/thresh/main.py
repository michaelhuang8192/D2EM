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

#Shop()
HireMerc()
RepairAll()
RemoteStash()
#IdentifyAll()
#RemoteStash()
StashGold()

char.tar_name = ["Thresh Socket"]

WalkToWp(Data.ArreatPlateau)
char.Precast()
warpClsId = FindWarpClsId(Data.CrystallinePassage)
warp = GetStaticUnits(5, warpClsId)
if object: Teleport(warp[0][2], warp[0][3])
char.Attack()
Sleep(500)
ItemCheck()
TpToTown()

DLog("End of Script: %s" % ("Thresh", ), 0)
