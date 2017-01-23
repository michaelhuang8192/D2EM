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

char.target = [Data.DefiledWarrior]

WalkToWp(Data.HallsOfPain)
char.Precast()
TeleToWarp(Data.HallsOfAnguish)
TeleToWarp(Data.NihlathaksTemple)
char.Attack()
Sleep(500)
ItemCheck()
TpToTown()
 
DLog("End of Script: %s" % ("Pindle", ), 0)
