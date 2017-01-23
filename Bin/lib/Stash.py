from D2EM import *
from Town import *
from ItemCheck import *


invMap = (1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
          1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
          1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
          1, 1, 1, 1, 1, 1, 1, 0, 0, 0)

invMapFromConfig = GetScriptConfig("invMap")
if invMapFromConfig:
    _invMap = eval( invMapFromConfig )
    if len(_invMap) == 40: invMap = _invMap


def GetInvItemIds():
    itemList = []
    
    inv = GetMapFromInv(2)[1]
    if not inv: return itemList

    for i in range(40):
        if not invMap[i]: continue
        
        itemId = inv[i]
        if itemId and not itemList.count(itemId): itemList.append(itemId)
    
    return itemList

def IsStashEmpty():
    stash = GetMapFromInv(6)
    if stash == None: return (0L, 0L)
    
def HasItemOnCursor():
    cid = GetInvInfo()[2]
    if cid: return cid
    
def GetStashItemIds():
    itemList = []
    
    if IsStashEmpty(): return itemList
    
    stash = GetMapFromInv(6)[1]

    for i in range(48):
        itemId = stash[i]
        if itemId and not itemList.count(itemId): itemList.append(itemId)
    
    return itemList

def HasItemInInv(clsId):
    items = GetInvItemIds()
    if items:
        for i in items:
            unit = GetUnit(0, 4, i)
            if unit and unit[0] == clsId: return True
        
    return False

def RemoteStash():
    itemList = GetInvItemIds()
    
    if itemList:
        for item in itemList:
            ret = IsInList(item) # - [priority, itemName, quality, id, clsId, "", "", "", iden]
            #DLog(ret)
            
            if ret:
                for c in (88, 596, 591, 516, 518, 529):
                    if c == ret[4]:
                        ret = False
                        break
                
            if (ret and not ret[8]) or (ret and IsIdentified(ret[9])):
                itemSize = GetItemSize(ret[4])
                location = GetFreeGrid(7, itemSize[0], itemSize[1])
                
                if IsStashEmpty(): location = (0L, 0L)
                
                if location:
                    #Sleep(250)
                    RemoveInvItem(item)
                    #Sleep(250)
                    AddItemToStash(item, location[0], location[1])
            
            elif not ret:
                #Sleep(250)
                RemoveInvItem(item)
                DropItem()
                #Sleep(250)

    else: pass #DLog("Has no items to stash", 0)
    
    
def wait4_itemRemove(ms, id):
    done = False
    while True:
        invItems = GetMapFromInv(2)[1]
        ms = Sleep(ms, (True,))
        if not (id in invItems):
            done = True
            break
        
        if ms <= 0: break
    
    return done
    
def RemoveInvItem(id):
    while True:
        RemoveItemFromInv(id)
        
        if not wait4_cursor_clear(500):
            Sleep(0)
            break

def AddItemToStash(id, x, y, dst=0x04):
    while True:
        cid = GetInvInfo()[2]
        if not cid: return
        
        AddItemToInv(id, x, y, dst)
        if wait4_cursor_clear(500):
            Sleep(200)
            return
        
def CheckGold():
    return (GetStashGold() < GetGoldMax()) and (GetInvGold() > (GetMaxInvGold() - 60000))
    
def CalGold(gold):
    return ( (gold >> 16) | ((gold & 0xffff) << 16) )
    
def DepositGold():
    gold = GetInvGold()
    DLog("Stashing %d" % (gold, ), 0)
    gold = CalGold(GetInvGold())
    ClickButton(0x14, gold)
    
def RunToStash():
    while True:
        stash = GetNearbyUnits(2, STASH_CLSID)
        if stash:
            stash = stash[0]
            break
        
        staticStash = GetStaticUnits(2, STASH_CLSID)[0]
        Run(staticStash[2],staticStash[3],10)
        Sleep(0)
    
    RunToUnit(2, stash[1])

def CloseStash():
    while GetPanelFlag(PFG_STASH):
        PostEsc()
        
        tick = GetTickCount()
        while GetPanelFlag(PFG_STASH):
            Sleep(50)
            if GetTickInterval(tick) >= 800:
                break
            
def StashGold():
    if not CheckGold(): return
    
    DLog("Going to Stash Gold!", 0)
    RunToStash()
    IntStash()
    Sleep(500)
    DepositGold()
    Sleep(500)
    DLog("Close Stash!", 0)
    CloseStash()

def IsRuneWord(id):
    flag = GetUnitExtension(4, id)[1]
    
    return bool(flag & 0x4000000)
    
def CursorCheck():
    cid = HasItemOnCursor()
    if not cid: return
    
    if not MercHasWp() and IsRuneWord(cid): EquipMercWp()
    
    else:
        itemClsId = GetUnit(0, 4, cid)[0]
        itemSize = GetItemSize(itemClsId)
        location = GetFreeGrid(3, itemSize[0], itemSize[1])
        
        if location:
            AddItemToStash(cid, location[0], location[1], 0)
        
        else: DropItem()
       