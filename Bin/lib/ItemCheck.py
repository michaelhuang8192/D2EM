from D2EM import *
from Belt import *


def GetItemQuality(id):
    item = GetUnit(3, 4, id)
    
    if item: return item[0]
    
def HasUniqueCharm(clsId):
    invItem = GetItemsFromInv(2)
    
    if invItem:
        for i in invItem:
            if not i: continue

            itemQuality = GetItemQuality(i[1])
            
            if clsId == i[0] and itemQuality == 7:
                #DLog("Has unique charm %d" % (clsId, ), 0)
                return True
    
    return False

def IsUniqueCharm(clsId, qlt):
    if qlt == 7 and (clsId == 603 or clsId == 604 or clsId == 605): return True
    
    else: return False

def IsPotion(clsId):
    if clsId in (591, 596, 516):
        return True
    
    else: return False

def IsTsc(clsId):
    if clsId == 529: return True
    
    else: return False
    
def PickTsc(quantity = 20):
    skill = GetSkill(0xDC)
    if skill and skill[3] < quantity: return True
    
    else: return False

def GetMaxInvGold():
    char_lv = GetStat(12)
    if char_lv: return char_lv[1] * 10000
    
    else: return 0

def GetInvGold():
    invGold = GetStat(14)
    if invGold: return invGold[1]
    
    return 0

def GetStashGold():
    stashGold = GetStat(15)
    if stashGold: return stashGold[1]
    
    return 0

def IsOkToPick(item):
    #item - [priority, itemName, quality, id, clsId]
    
    itemPos = GetPos(4, item[3])
    if not itemPos: return
    
    itemClsId = item[4]
    itemSize = GetItemSize(itemClsId)
    itemQlt = item[2]
    
    if IsTownLevel(item[5]): return False
    
    if not GetPointInMap(itemPos[0], itemPos[1]):
        DLog("Item unreachable cls id [%d]" % (itemClsId, ), 2)
        return False
    
    if itemClsId == 523:
        invGold = GetInvGold()
        if (invGold == 0 or invGold) and invGold < GetMaxInvGold(): return True
        
        else:
            return False
    
    if IsPotion(itemClsId): return HasFreeNode(itemClsId)
        
    if IsTsc(itemClsId): return PickTsc()
    
    if not GetFreeGrid(3, itemSize[0], itemSize[1]):
        DLog("No free cell in inventory", 0)
        return False
    
    if IsUniqueCharm(itemClsId, itemQlt) and HasUniqueCharm(itemClsId):
        DLog("Has unique charm already", 0)
        return False
    
    return True

def FilterGroundItems():
    allItems = GetNearbyUnits(4) #[(clsId, itemId, 3L, 268435491L, level_no),]
    items = {}
    
    if allItems:
        for i in allItems:
            item = IsInList(i[1])
            
            if item:
                item[5] = i[4]
                
                if IsOkToPick(item):
                    items[item[3]] = item
                    #DLog("Add Item Id: %d, ClsId: %d, Lvl_No: %d" % (item[3], item[4], item[5]), 0)
    
    return items

            
#IsInList return - [priority, itemName, quality, id, clsId, "", "", "", iden]
#[(clsId, itemId, 3L, 268435491L, level_no),]

def GetItemDist(item):
    return item[7]
    
def GetItemPriority(item):
    return item[0]

def ItemCheck(tar_id=0):
    groundItems = FilterGroundItems() #{id:[priority, itemName, quality, id, clsId, level_no],}
    
    if not groundItems: return
    
    filteredItems = {}
    
    s_key = GetItemDist
    se = True
    
    while True:
        my_pos = GetPos()
        my_rm = GetRoom()
    
        if not my_pos or not my_rm:
            DLog("Invalid player position or room, in ItemCheck()", 3)
            return
    
        if IsTownLevel(my_rm[4]):
            DLog("In town now, stop pick!", 0)
            return 
        
        if s_key == GetItemDist:
            for i in groundItems:
                item = groundItems[i] #[priority, itemName, quality, id, clsId, level_no]
                
                if my_rm[4] != item[5] or IsTownLevel(item[5]):
                    DLog("Item in towns", 0)
                    continue
                
                itemPos = GetPos(4, i)
                if not itemPos:
                    DLog("No item pos, id: %d, x: %d, y: %d" % (i, targetItem[6][0], targetItem[6][1]), 0)
                    continue
                
                dist = GetDistance(my_pos[0], my_pos[1], itemPos[0], itemPos[1])
                
                if len(item) >= 7:
                    item[6] = itemPos
                    item[7] = dist
                    
                else:
                    item.append(itemPos)
                    item.append(dist)
                
                filteredItems[i] = item #[priority, itemName, quality, id, clsId, level_no, [itemPosX, itemPosY], dist]
    
        if not filteredItems:
            return
    
        targetItem = min(filteredItems.itervalues(), key=s_key)
        
        if se:
            if s_key == GetItemDist:
                if targetItem[7] <= 4:
                    del filteredItems[targetItem[3]]
                    del groundItems[targetItem[3]]
                    
                    if IsOkToPick(targetItem):
                        DLog("Pick item - priority: %d, code: %s, id: %d, x: %d, y: %d" % (targetItem[0], targetItem[4], targetItem[3], targetItem[6][0], targetItem[6][1]), 0)
                        PickItem(targetItem[3])
                        Sleep(0)
                
                else:
                    Sleep(0)
                    s_key = GetItemPriority
                    
            
            elif s_key == GetItemPriority:
                if targetItem[0] == 0:
                    if IsOkToPick(targetItem) and Teleport(targetItem[6][0], targetItem[6][1]):
                        Sleep(0)
                        DLog("Tele to item - priority: %d, code: %s, id: %d, x: %d, y: %d, dist: %d" % (targetItem[0], targetItem[4], targetItem[3], targetItem[6][0], targetItem[6][1], targetItem[7]), 0)
                    
                    else:
                        DLog("Pick item fail - priority: %d, code: %s, id: %d" % (targetItem[0], targetItem[4], targetItem[3]), 2)
                        del filteredItems[targetItem[3]]
                        del groundItems[targetItem[3]]
                        Sleep(0)
                
                else:
                    se = False
                    Sleep(0)
                
                s_key = GetItemDist
                    
        else:
            tar_rm = GetRoom(1, tar_id)
            life = GetLife(1, tar_id)
            tar_pos = GetPos(1, tar_id)
            
            if not tar_pos or not tar_rm or my_rm[4] != tar_rm[4] or IsTownLevel(tar_rm[4]) or not life or not IsAttackable(1, tar_id) or targetItem[7] < GetDistance(my_pos[0], my_pos[1], tar_pos[0], tar_pos[1]):
                
                if IsOkToPick(targetItem) and Teleport(targetItem[6][0], targetItem[6][1]):
                    DLog("Pick first, Tele to item - priority: 1, code: %s, id: %d" % (targetItem[4], targetItem[3]), 0)
                    se = True
                    Sleep(0)
                    pass
            
                else:
                    DLog("Fail tele to item - priority: 1, code: %s, id: %d" % (targetItem[4], targetItem[3]), 0)
                    del filteredItems[targetItem[3]]
                    del groundItems[targetItem[3]]
                    Sleep(0)
            
            elif targetItem[7] >= GetDistance(my_pos[0], my_pos[1], tar_pos[0], tar_pos[1]):
                DLog("Fight first!", 0)
                break
            
            else: DLog("Last else in ItemCheck(), should never got here!", 3)

"""
def _InitGold(reason):
    if reason == 0: #game start
        global maxInvGold, maxStashGold
        
        maxInvGold = GetMaxInvGold()
        maxStashGold = GetGoldMax()
        
        DLog("Resetting max gold. MaxSG: %d; MaxIG: %d" % (maxStashGold, maxInvGold), 0)
        

AttachLib(_InitGold)
"""