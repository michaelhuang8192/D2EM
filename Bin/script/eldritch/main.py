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

def numeric_compare0(x, y):
    if x[0] > y[0]:
      return 1
    elif x[0] == y[0]:
       return 0
    else: 
       return -1

def numeric_compare1(x, y):
    if x[1] > y[1]:
      return 1
    elif x[1] == y[1]:
       return 0
    else: 
       return -1
    
def GetEdgeRooms(lvl):
    edgeRooms = []
    
    for r in GetAllRooms(1):
        for t in r[5]:
            if t[4] == lvl:
                edgeRooms.append(r)
                break
    
    return edgeRooms

def GetDirection(edgeRooms, destLvl):
    deltaX = None
    deltaY = None
    x =  None
    y = None
    roomList = []
    
    for er in edgeRooms:
        roomList.append(er)
        
    for er in edgeRooms:
        rt = er[5]
        for r in rt:
            if r[4] == destLvl:
                if er[0] == r[0]:
                    deltaX = 0
                    break
                
                if er[1] == r[1]:
                    deltaY = 0
                    break
        
        c = 0
        if deltaX == 0:
            if er[1] > r[1]: deltaY = -1
            
            else: deltaY = 1
            
            if deltaY > 0:
                y = er[1] + 38 * deltaY
            else: y = er[1] + 1
            roomList.sort(cmp=numeric_compare0)
            xr = (roomList[0][0], roomList[len(roomList)-1][0])
            
            xrange = range(xr[0], xr[1])
            while c < len(xrange):
                point = (xrange[c], y)
                pt = GetPointInMap(point[0], point[1])
                if pt != None and not (pt & 0xc2f):
                    x = point[0]
                    cpt = GetPointInMap(point[0] - 13, point[1])
                    if cpt != None and not (cpt & 0xc2f):
                        break
                
                c += 1
                
            break
            
        elif deltaY == 0:
            if er[0] > r[0]: deltaX = -1
            
            else: deltaX = 1
            
            if deltaX > 0:
                x = er[0] + 38 * deltaX
            else: x = er[0] + 1
            roomList.sort(cmp=numeric_compare1)
            yr = (roomList[0][1], roomList[len(roomList)-1][1])
            
            yrange = range(yr[0], yr[1])
            while c < len(yrange):
                point = (x, yrange[c])
                pt = GetPointInMap(point[0], point[1])
                if pt != None and not (pt & 0xc2f):
                    y = point[1]
                    cpt = GetPointInMap(point[0], point[1] - 13)
                    if cpt != None and not (cpt & 0xc2f):
                        break
                
                c += 1
                
            break
    
    return ((x, deltaX), (y, deltaY))

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

char.tar_name = ["Eldritch the Rectifier"]

WalkToWp(Data.FrigidHighlands)
Sleep(500)
char.Attack()
Sleep(500)
ItemCheck()

char.tar_name = ["Shenk the Overseer"]

edgeRooms = GetEdgeRooms(Data.BloodyFoothills)
cdn = GetDirection(edgeRooms, Data.BloodyFoothills)
Teleport(cdn[0][0], cdn[1][0])
Teleport(cdn[0][0]+5*cdn[0][1], cdn[1][0]+5*cdn[1][1])

object = GetStaticUnits(2, 434)
Teleport(object[0][2], object[0][3])
char.Attack()
Sleep(500)
ItemCheck()

TpToTown()

DLog("End of Script: %s" % ("Eldritch", ), 0)
