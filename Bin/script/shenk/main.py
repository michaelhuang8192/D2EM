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
IdentifyAll()
RemoteStash()
StashGold()

char.tar_name = ["Shenk the Overseer"]

offset = (-30,6)
my_pos = GetPos()

WalkToWp(Data.FrigidHighlands)
Sleep(500)
if SelectSkill(0x36, 1) and my_pos: Teleport(0x0EDC, 0x13F0)
object = GetStaticUnits(2, 434)
Teleport(object[0][2], object[0][3])
char.Attack()
Sleep(500)
ItemCheck()
TpToTown()
    
DLog("End of Script: %s" % ("Shenk", ), 0)
