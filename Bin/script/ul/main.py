from D2EM import *
from D2EMExtended import *
from Belt import *
import Data
import Character
from ItemCheck import *
from Town import *
from Stash import *
from LifeCheck import *
#import Party
#import OnWake_TownAPC
import Leech
import OnPacketUpdate
import Command
import time

#AttachEvent(EVT_ONWAKE, OnWake_TownAPC.OnWake_TAPC)
AttachEvent(EVT_ONPACKETUPDATE, OnPacketUpdate.OnPacketUpdate)


GetBody()
CursorCheck()

charClass = {0:Character.CsAmazon,
             3:Character.HammerPaladin,
             5:Character.Druid,
             6:Character.HybridSin}

charType = GetCharClass()

if charClass.has_key(charType): char = charClass[charType]()

dx = 0
dy = 1
bwCount = 0
   
    
drawPos = OnDraw
def OnDraw():
    drawPos()
    type = 2
    u = GetNearbyUnits(type)
    if not u: return
    mpos = GetPos()
    for k in u:
        kpos = GetPos(type, k[1])
        if kpos and GetDistance(kpos[0], kpos[1], mpos[0], mpos[1]) < 10:
            sp = GetScreenOffset(type, k[1])
            if not sp: return
            
            left = sp[0] - 50
            top = sp[1] - 20
            right = sp[0] + 50
            bottom = sp[1] + 20
            
            DrawText("<%d,%d>" % (k[0], k[2]), 0x0033FF, left, top, right, bottom, 1)
    
while 1:
    if Command.spin and not IsInTown() and charType == 3:
        my_pos = GetPos()
        
        dx = dx % 8 + 1
        dy = dy % 6 + 1
        
        if Command.pos == 0:
            if GetSkillUIdBySide(1)[0] != 113: SelectSkill_NB(113, 1)
        
        else:
            if GetSkillUIdBySide(1)[0] != 0x006d: SelectSkill_NB(0x006d, 1)
        
        CastSkill(my_pos[0] + dx , my_pos[1] + dy, 0)
        
        Sleep(350)
        continue
    
    elif Command.tele and not IsInTown():
        Command.tele = False
        TeleToWarp(Command.destLvl)
        
    elif Command.up and not IsInTown():
        pids = GetPlayerIds()
        if Command.pos == 0: spinPos = (25046, 5138)
        else: spinPos = (25046, 5135)
        
        Teleport(spinPos[0], spinPos[1]) 
        Command.spin = True
        Command.up = False
    
    elif Command.down and not IsInTown():
        pids = GetPlayerIds()
        if Command.pos == 0: spinPos = (25143, 5095)
        else: spinPos = (25143, 5092)
        
        Teleport(spinPos[0], spinPos[1]) 
        Command.spin = True
        Command.down = False
        
    elif Command.rb and not IsInTown():
        Teleport(0x625d, 0x13e2, 4)
        Command.rb = False
    
    elif Command.clear and not IsInTown():
        char.Attack()
    
    elif Command.drop and IsInTown():
        items = GetInvItemIds()
        if items:
            for i in items:
                RemoveInvItem(i)
                DropItem()
            
            items = []
        
        Command.drop = False
        
    elif Command.toInv and IsInTown():
        items = GetStashItemIds()
        if items:
            for i in items:
                itemClsId = GetUnit(0, 4, i)[0]
                if itemClsId == 549: continue
                itemSize = GetItemSize(itemClsId)
                location = GetFreeGrid(3, itemSize[0], itemSize[1])
                
                if location:
                    RemoveInvItem(i)
                    AddItemToStash(i, location[0], location[1], 0)
            
            items = []
        
        Command.toInv = False
    
    elif Command.toStash and IsInTown():
        """
        items = GetInvItemIds()
        if items:
            for i in items:
                itemClsId = GetUnit(0, 4, i)[0]
                itemSize = GetItemSize(itemClsId)
                location = GetFreeGrid(7, itemSize[0], itemSize[1])
                
                if location:
                    RemoveInvItem(i)
                    AddItemToStash(i, location[0], location[1])
            
            items = []
        """
        RemoteStash()
        Command.toStash = False
    
    elif Command.bw and not IsInTown():
        units = GetNearbyUnits(1, 344)
        cast = True
        bwPos = (Command.bwPosX, Command.bwPosY)
        if units:
            for u in units:
                if GetLife(1, u[1]):
                    cast = False
        
        if cast:
            if GetSkillUIdBySide(1)[0] != 0x004e: SelectSkill_NB(0x004e, 1)
        
            CastSkill(bwPos[0], bwPos[1], 1)
            Sleep(2000)
            bwCount += 1
            DLog("Bonewall Count [# %d]" % (bwCount, ), 0)
        
        if bwCount >= 3:
            Command.bw = False
            bwCount = 0
        
    ItemCheck()
    
    Sleep(0)

DLog("End of Script: %s" % ("ul", ), 0)
