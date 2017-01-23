from D2EM import *
from D2EMExtended import *
from Belt import *
from ItemCheck import *
import Data


#(Potion, Cain, Hire, Repair)
townNpcs = ((Data.Akara, Data.DeckardCain5, Data.Kashya, Data.Charsi),
    (Data.Drognan,       Data.DeckardCain2, Data.Greiz, Data.Fara),
    (Data.Ormus,         Data.DeckardCain3, Data.Asheara, 0),
    (Data.Jamella,       Data.DeckardCain4, Data.Tyrael2, Data.Halbu),
    (Data.Malah,         Data.DeckardCain6, Data.QualKehk, Data.Larzuk))


def GetNpcClsId(npc):
    actNo = GetActNo()
    
    if not (actNo in (4, 5)):
        #DLog("NPC ClsId: %d" % (townNpcs[4][npc], ), 0)
        return townNpcs[4][npc]
    
    #DLog("NPC ClsId: %d" % (townNpcs[actNo-1][npc], ), 0)
    return townNpcs[actNo-1][npc]
    

def RunToNpc(npcClsId):
    npc = GetStaticUnits(1, npcClsId)
    if npcClsId == Data.DeckardCain6: npc = False
    
    while not npc:
        DLog("NPC %d not found " % (npcClsId, ), 0)
            
        my_rm = GetRoom()
            
        if (my_rm[4] == Data.Harrogath) and (npcClsId in (Data.Larzuk, Data.DeckardCain6)):
            wpClsId = WP_CLSID[GetActNo()-1]
            wp = GetStaticUnits(2, wpClsId)
            wpPos = [wp[0][2], wp[0][3]]
            Run(wpPos[0], wpPos[1])
            return
            
        WalkToWp(Data.Harrogath)
        Sleep(500)
            
        npc = GetStaticUnits(1, npcClsId)
        if npcClsId == Data.DeckardCain6: npc = False
            
        continue
    
    npcPos = (npc[0][2],npc[0][3])
    Run(npcPos[0], npcPos[1])
        

def GetItemDura(id):
    dura = GetStat(72, 0, 4, id)
    if dura: return dura
    
    return False

def IsEthereal(flag):
    if flag & 0x400000: return 1
    
    else: return 0
    
def CheckEquip():
    equipmentIds = GetMapFromInv(0)[1][1:]

    if equipmentIds:
        for i in equipmentIds:
            iflag = GetUnitExtension(4, i)[1]
            if IsEthereal(iflag): continue
            
            dura = GetItemDura(i)
            if dura and dura[1] <= 4:
                DLog("Need to repair!", 0)
                return True
    
    #DLog("No need to repair", 0)
    return False


def CheckMerc(hire=True):
    mercId = GetMercId()
    if not mercId: return True
    
    return IsDead(1, mercId)
    

def IsIdentified(flag):
    if flag & 0x10: return 1
    
    else: return 0
    
    
def HasItemsToIden():
    invItems = GetItemsFromInv(2)
    
    if invItems:
        for i in invItems:
            u = GetUnit(3, 4, i[1]) #(quality,flag,level,dst0,dst1)
            
            if not u: continue
            
            if not IsIdentified(u[1]):
                DLog("Cain Identify", 0)
                return True
    
    #DLog("No items needed to be identified", 0)
    return False


def IsIdentified(flag):
    if flag & 0x10: return 1
    
    else: return 0
    
    
def BuyPots(npcId, menu_idx, npcClsId):
    items = {}
    ClickOnMenu(menu_idx)
    Sleep(500)
        
    for itemClsId in (529, 591, 596): #tsc hp5 mp5
        item = GetItemsFromInv(5, itemClsId, 1, npcId)
            
        if item:
            items[itemClsId] = item[0]
            DLog("%d price: %d" % (itemClsId, GetItemPrice(items[itemClsId][1], npcClsId, 0)), 0)
                
    if not len(items): return PostEsc()
        
    while True and items.has_key(591):
        if HasFreeNode(591) and ((GetInvGold() + GetStashGold()) >= GetItemPrice(items[591][1], npcClsId, 0)):
            BuyItem(npcId, items[591][1], 0)
            Sleep(400)
            continue
        break
    
    while True and items.has_key(596):
        if HasFreeNode(596) and ((GetInvGold() + GetStashGold()) >= GetItemPrice(items[596][1], npcClsId, 0)):
            BuyItem(npcId, items[596][1], 0)
            Sleep(400)
            continue
        break
    
    while True and items.has_key(529):
        if PickTsc() and ((GetInvGold() + GetStashGold()) >= GetItemPrice(items[529][1], npcClsId, 0)):
            BuyItem(npcId, items[529][1], 0x80000000)
            Sleep(200)
            continue
        break
    
    DLog("Post ESC", 0)
    PostEsc()        
    
            
def GetMenuIdx(action):
    dlg = GetNPCDialog()
    
    if dlg:
        menu = dlg[6]
        
        for d in range(len(menu)):
            if menu[d][0] == action: return d
    
    return None


def CancelNpnInt():
    while True:
        menuIdx = GetMenuIdx("cancel")
        if not (menuIdx == None):
            ClickOnMenu(menuIdx)
            Sleep(500)
            continue
                
        break
    
def RemoveMercWp():
    while True:
        id = GetInvInfo()[2]
        if id: return
        
        EquipMerc(4)
        if not wait4_cursor_clear(500):
            Sleep(200)
            return

def EquipMercWp():
    while True:
        id = GetInvInfo()[2]
        if not id: return
        
        EquipMerc(0)
        if wait4_cursor_clear(500):
            Sleep(200)
            return
        
def MercHasWp():
    mercWp = False
    mercId = GetMercId()
    
    if mercId and not IsDead(1, mercId):
        mercInv = GetMapFromInv(0, 1, mercId)
        if mercInv:
            mercWp = mercInv[1][4]
            
    else: DLog("No merc", 0)
    
    return mercWp

def NpcAction(action):
    while True:
        npcClsId = GetNpcClsId(action)
        npc = GetNearbyUnits(1, npcClsId)
        if npc: break
            
        RunToNpc(npcClsId)

    if npc:
        RunToUnit(1, npc[0][1])  
        IntTownNPC(npc[0][1])
            
        Sleep(500)
            
        if action == 0: return BuyPots(npc[0][1], GetMenuIdx("trade"), npcClsId)
            
        elif action == 1: IdentifyAllItems(npc[0][1])
            
        elif action == 2: ResurrectMerc(npc[0][1])
            
        elif action == 3: RepairAllItems(npc[0][1])

        Sleep(500)
        DLog("Cancel NPC interaction!", 0)
            
        CancelNpnInt()
        
def IsRuneWord(id):
    flag = GetUnitExtension(4, id)[1]
    
    return bool(flag & 0x4000000)

def HireMerc():
    refresh = False
    
    while True:
        if not CheckMerc() or ((GetInvGold() + GetStashGold()) < 50000):
            #DLog("No need to hire merc!", 0)
            break
        
        DLog("Going to hire merc", 0)
        NpcAction(2)
        
        mercWp = MercHasWp()
        if mercWp and IsRuneWord(mercWp): refresh = True
    
    if refresh and MercHasWp():
        DLog("Refresh merc weapon!", 0)
        RemoveMercWp()
        EquipMercWp()
        
def RepairAll():
    if not CheckEquip(): return
    
    NpcAction(3)
    

def IdentifyAll():
    if HasItemsToIden(): NpcAction(1)
    
    
def Shop():
    life = GetStat(0x06)
    max_life = GetStat(0x07)
    
    if not (life and max_life): lp = 0
    
    else: lp = float(life[1] >> 8) / (max_life[1] >> 8) * 100
    
    if lp < 80 or HasFreeNode(591) or HasFreeNode(596) or PickTsc(12):
        DLog("Need to shop or heal!", 0)
        NpcAction(0)
        return
    
    #DLog("No need to shop or heal!", 0)