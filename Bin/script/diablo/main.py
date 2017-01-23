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


#AttachEvent(EVT_ONWAKE, OnWake_TownAPC.OnWake_TAPC)
    
seals = ((396,"Grand Vizier of Chaos"), (395,0), (394,"Lord De Seis"), (393,0), (392,"Infector of Souls"))
forge = 376

def CountNearbyMonsters():
    mCount = 0
    units = GetNearbyUnits(1)
    myPos = GetPos()
    for u in units:
        monPos = GetPos(1, u[1])
        if GetDistance(myPos[0], myPos[1], monPos[0], monPos[1]) <= 10:
            mCount += 1
            
    return mCount

def wait4_seal_open(id):
    tick = GetTickCount()
    hm_tick = 0
    
    while True:
        Sleep(50)
        unit = GetUnitState(2, id)
        state = unit[1]
        clsId = unit[0]
        #DLog( "seal state [%d]" % (state, ), 0)
        
        if state == 2: return True
        
        if clsId in (395, 393) and state == 1: return True
        
        if CountNearbyMonsters() >= 3 and GetTickCount() - hm_tick > 400:
            DLog("Hammer while waiting for seal open", 0)
            hm_tick = GetTickCount()
            if GetSkillUIdBySide(1)[0] != 113: SelectSkill_NB(113, 1)
            myPos = GetPos()
            CastSkill(myPos[0], myPos[1], 0)
            
        if GetTickCount() - tick >= 3000: return False
    
def IntSeal(id):
    DLog("Interact seal[0x%X]" % (id, ), 0)
    
    IntUnit2(id)
    if wait4_seal_open(id):
        Sleep(200)
        return True
    
    return False
        
def GoToSeal(sealCID):
    tick = GetTickCount()
    while 1:
        unit = GetNearbyUnits(2, sealCID)
        if unit:
            id = unit[0][1]
            pos = GetPos(2, id)
            
            Teleport(pos[0], pos[1], 5)
            if IntSeal(id):
                Sleep(500)
                ItemCheck()    
                break
            
            if GetTickCount() - tick >= 25000: break
        
        else:
            seal = GetStaticUnits(2, sealCID)[0]
            Teleport(seal[2], seal[3], 5)
        
        Sleep(0)

def GetUniqueBossByName(boss):
    units = GetNearbyUnits(1)
    for u in units:
        name = GetName(1, u[1])
        if name == boss: return u
        
    return False
    
def ClearSeals():
    char.target = []
    char.tar_name = ["Grand Vizier of Chaos", "Lord De Seis", "Infector of Souls"]
    
    for s in seals:
        GoToSeal(s[0])
        
        if s[0] == 394: Teleport(0x1e80, 0x1449, 5)
        
        if s[0] == 396: Teleport(0x1dfa, 0x14c0, 5)
        
        if s[1]:
            tick = GetTickCount()
            hm_tick = 0
            while not GetUniqueBossByName(s[1]) and GetTickInterval(tick) < 5000:
                DLog("waiting %s" % (s[1], ), 0)
                if GetTickCount() - hm_tick > 400:
                    DLog("Hammer while waiting for %s" % (s[1], ), 0)
                    hm_tick = GetTickCount()
                    if GetSkillUIdBySide(1)[0] != 113: SelectSkill_NB(113, 1)
                    myPos = GetPos()
                    CastSkill(myPos[0], myPos[1], 0)
                    
                Sleep(10)
            
            k_tick = GetTickCount()
            while True:
                char.Attack()
                boss = GetUniqueBossByName(s[1])
                if not boss or not GetLife(1, boss[1]) or GetTickCount() - k_tick > 25000: break
                Sleep(10)

            Sleep(250)
            ItemCheck()
            continue
        
        Sleep(0)
        
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

char.target = [Data.Hephasto]

WalkToWp(Data.RiverOfFlame)
char.Precast()
object = GetStaticUnits(2, forge)
Teleport(object[0][2], object[0][3])

while True:
    char.Attack()
    Sleep(250)
    ItemCheck()
    hephasto = GetNearbyUnits(1, Data.Hephasto)
    if not hephasto or not GetLife(1, hephasto[0][1]): break
    Sleep(10)
    
Sleep(250)
ItemCheck()

Teleport(7798, 5602)
Teleport(7796, 5586)

ClearSeals()

star = GetStaticUnits(2, 255)[0]
Teleport(star[2], star[3])

char.tar_name = []
char.target = [Data.Diablo]

tick = GetTickCount()
dcount = 0
while not GetNearbyUnits(1, Data.Diablo):
    if GetTickInterval(tick) > 20000:
        tick = GetTickCount()
        if dcount >= 1: break
        ClearSeals()
        char.tar_name = []
        char.target = [Data.Diablo]
        Teleport(star[2], star[3])
        dcount += 1
        
    Sleep(100)

while True:
    Teleport(star[2], star[3])
    char.Attack()
    diablo = GetNearbyUnits(1, Data.Diablo)
    if not diablo or not GetLife(1, diablo[0][1]): break
    Sleep(10)
    
Teleport(star[2], star[3])
Sleep(500)
ItemCheck()
TpToTown()

DLog("End of Script: %s" % ("Diablo", ), 0)
