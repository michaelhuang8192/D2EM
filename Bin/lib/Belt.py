from D2EM import *


belts = {"Sash":8, "Light Belt":8, "Belt":12, "Heavy Belt":12,}
potionLimit = {591: 1, 596: 2, 516: 4} #hp5:# of col, mp5, rvl
    
def GetBeltSize():
    equipPos = GetMapFromInv(0)[1]
    b = GetUnit(1, 4, equipPos[8])
    if b:
        return belts.get(b, 16)
        
    else:
        DLog("Your char doesn't have a belt", 2)
        return 4

def HasFreeNode(ClsId):
    pl = potionLimit.get(ClsId, 4)
    if not pl: return False
        
    tick = GetTickCount()
    while 1:
        equipPos = GetMapFromInv(1)[1]
        if GetTickCount() - tick > 5000 or len(equipPos): break
        Sleep(250)
        
    numOfRow = GetBeltSize() / 4
        
    res = False
    emptyCol = 0
    l = 0
        
    for n in range(0, 4):
        
        if not equipPos[n]:
            emptyCol += 1
            continue
        
        itemClsId = GetUnit(0, 4, equipPos[n])[0]
            
        if itemClsId == ClsId:
            l += 1

            lr = n + (numOfRow - 1)*4
            if not equipPos[lr]:
                res = True
                break
        
    if not res and emptyCol > 0 and l < pl: res = True
        
    if res: pass #DLog("There free node in belt", 0)
        
    else: pass #DLog("Belt is full", 0)
        
    return res
            
        
        
        