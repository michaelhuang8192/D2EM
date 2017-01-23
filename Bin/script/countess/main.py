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

char.target = [Data.DarkStalker]

WalkToWp(Data.BlackMarsh)
char.Precast()
TeleToWarp(Data.ForgottenTower)
TeleToWarp(Data.TowerCellarLevel1)
TeleToWarp(Data.TowerCellarLevel2)
TeleToWarp(Data.TowerCellarLevel3)
TeleToWarp(Data.TowerCellarLevel4)
TeleToWarp(Data.TowerCellarLevel5)
object = GetStaticUnits(2, 580)
if object: Teleport(object[0][2], object[0][3])
char.Attack()
Sleep(500)
ItemCheck()
TpToTown()
 
DLog("End of Script: %s" % ("Countess", ), 0)

