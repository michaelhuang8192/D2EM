from _D2EM import SendPacket
import _struct

_packet_fmt = [None] * 256
_packet_fmt[0x01] = "<BHH" #walk
_packet_fmt[0x02] = "<BII" #walk to unit
_packet_fmt[0x03] = "<BHH" #run
_packet_fmt[0x04] = "<BII" #run to unit
_packet_fmt[0x05] = "<BHH" #left hand cast
_packet_fmt[0x06] = "<BII" #left hand cast on object
_packet_fmt[0x07] = "<BII" #stop left hand cast on object
_packet_fmt[0x08] = "<BHH" #left hand recast
_packet_fmt[0x09] = "<BII" #left hand recast on object
_packet_fmt[0x0A] = "<BII" #stop left hand recast on object
_packet_fmt[0x0C] = "<BHH" #right hand cast
_packet_fmt[0x0D] = "<BII" #right hand cast on object
_packet_fmt[0x0E] = "<BII" #stop right hand cast on object
_packet_fmt[0x0F] = "<BHH" #right hand recast
_packet_fmt[0x10] = "<BII" #right hand recast on object
_packet_fmt[0x11] = "<BII" #stop right hand recast on object
_packet_fmt[0x13] = "<BII" #interact
_packet_fmt[0x16] = "<BIII" #pick item
_packet_fmt[0x17] = "<BI" #drop item
_packet_fmt[0x18] = "<BIIII" #add item to inv
_packet_fmt[0x19] = "<BI" #remove item from inv
_packet_fmt[0x20] = "<BIII" #use item from inv
_packet_fmt[0x23] = "<BII" #add item to belt
_packet_fmt[0x24] = "<BI" #remove item from belt
_packet_fmt[0x26] = "<BIII" #use item from belt
_packet_fmt[0x27] = "<BII" #identify item
_packet_fmt[0x2A] = "<BII" #add item to cube
_packet_fmt[0x2F] = "<BII" #Int town npc
_packet_fmt[0x30] = "<BII" #cancel town npc int
_packet_fmt[0x32] = "<BIIII" #buy item
_packet_fmt[0x33] = "<BIIII" #sell item
_packet_fmt[0x34] = "<BI" #identify all items
_packet_fmt[0x35] = "<BIIII" #repair all items
_packet_fmt[0x38] = "<BIII" #select menu entry
_packet_fmt[0x3C] = "<BHHI" #select skill
_packet_fmt[0x49] = "<BII" #Int waypoint
_packet_fmt[0x4B] = "<BII" #Sync position
_packet_fmt[0x4F] = "<BHI" #click button
_packet_fmt[0x50] = "<BII" #drop gold
_packet_fmt[0x59] = "<BIIII" #drop gold
_packet_fmt[0x5D] = "<BBBI" #player relation
_packet_fmt[0x5E] = "<BBI" #player relation
_packet_fmt[0x5F] = "<BHH" #Update Position
_packet_fmt[0x60] = "<B" #switch weapon
_packet_fmt[0x61] = "<BBB" #remove/equip merc items
_packet_fmt[0x62] = "<BI" #resurrect merc
_packet_fmt[0x63] = "<BI" #mov inv item to belt


def unpack(buf):
    if len(buf):
        packet_id = ord(buf[0])
        if packet_id >= 0 and packet_id < len(_packet_fmt):
            fmt = _packet_fmt[packet_id]
            if fmt:
                return _struct.unpack(fmt, buf)
        
    return None


def pack(packet_id, *args):
    if packet_id >= 0 and packet_id < len(_packet_fmt):
        fmt = _packet_fmt[packet_id]
        if fmt:
            return _struct.pack(fmt, packet_id, *args)
    
    return None


def SelectSkill(skill_id, right=False, flag=0xFFFFFFFF):
    sval = 0x8000
    if right: sval = 0x0000
    
    buf = pack(0x3C, skill_id, sval, flag)
    SendPacket(buf, 1, 0)
    

def UseItemFromInv(item_id, x, y):
    buf = pack(0x20, item_id, x, y)
    SendPacket(buf, 1, 0)
    
    
def CastSkill(x, y, right=False, recast=False):
    if recast:
        packet_id = 0x08
        if right: packet_id = 0x0F
    else:
        packet_id = 0x05
        if right: packet_id = 0x0C
        
    buf = pack(packet_id, x, y)
    SendPacket(buf, 1, 0)


def CastSkillOnUnit(type, id, right=False, recast=False, castop=False):
    if right:
        packet_id = 0x0D
    else:
        packet_id = 0x06
        
    if recast: packet_id += 0x03
    if castop: packet_id += 0x01
    
    buf = pack(packet_id, type, id)
    SendPacket(buf, 1, 0)


def Move(x, y, walk=False):
    packet_id = 0x03
    if walk: packet_id = 0x01
    
    buf = pack(packet_id, x, y)
    SendPacket(buf, 1, 0)

    
def MoveToUnit(type, id, walk=False):
    packet_id = 0x04
    if walk: packet_id = 0x02
    
    buf = pack(packet_id, type, id)
    SendPacket(buf, 1, 0)


def IntUnit(type, id):
    buf = pack(0x13, type, id)
    SendPacket(buf, 1, 0)

def SendMessage(msg):
    buf = _struct.pack("BBB%dsBBB" % (len(msg),), 0x15, 0x01, 0x00, msg, 0x00, 0x00, 0x00)
    SendPacket(buf, 1, 0)

def SendOverheadMessage(msg):
    buf = _struct.pack("BBB%dsBBB" % (len(msg),), 0x14, 0x00, 0x00, msg, 0x00, 0x00, 0x00)
    SendPacket(buf, 1, 0)

def PickItem(id, cursor):
    buf = pack(0x16, 0x04, id, cursor)
    SendPacket(buf, 1, 0)

def DropItem(id):
    buf = pack(0x17, id)
    SendPacket(buf, 1, 0)

def AddItemToInv(id, x, y, dst):
    buf = pack(0x18, id, x, y, dst)
    SendPacket(buf, 1, 0)

def RemoveItemFromInv(id):
    buf = pack(0x19, id)
    SendPacket(buf, 1, 0)
    
def AddItemToBelt(id, pos):
    buf = pack(0x23, id, pos)
    SendPacket(buf, 1, 0)
    
def RemoveItemFromBelt(id):
    buf = pack(0x24, id)
    SendPacket(buf, 1, 0)
    
def UseItemFromBelt(id, type):
    buf = pack(0x26, id, type, 0)
    SendPacket(buf, 1, 0)
    
def IdentifyItem(item_id, scroll_id):
    buf = pack(0x27, item_id, scroll_id)
    SendPacket(buf, 1, 0)

def AddItemToCube(item_id, cube_id):
    buf = pack(0x2A, item_id, cube_id)
    SendPacket(buf, 1, 0)
    
def IntTownNPC(type, id):
    buf = pack(0x2F, type, id)
    SendPacket(buf, 1, 0)

def CancelTownNPCInt(type, id):
    buf = pack(0x30, type, id)
    SendPacket(buf, 1, 0)

def BuyItem(npc_id, item_id, flag, cost=0):
    buf = pack(0x32, npc_id, item_id, cost, flag)
    SendPacket(buf, 1, 0)

def SellItem(npc_id, item_id, cost, flag):
    buf = pack(0x33, npc_id, item_id, flag, cost)
    SendPacket(buf, 1, 0)

def IdentifyAllItems(cain_id):
    buf = pack(0x34, cain_id)
    SendPacket(buf, 1, 0)

def RepairAllItems(npc_id):
    buf = pack(0x35, npc_id, 0, 0, 0x80000000)
    SendPacket(buf, 1, 0)
    
def SelectMenuEntry(menu_id, npc_id, player_id):
    buf = pack(0x38, menu_id, npc_id, player_id)
    SendPacket(buf, 1, 0)
    
def IntWaypoint(id, level_no):
    buf = pack(0x49, id, level_no)
    SendPacket(buf, 1, 0)

def ClickButton(id, val):
    buf = pack(0x4F, id, val)
    SendPacket(buf, 1, 0)

def DropGold(id, amount):
    buf = pack(0x50, id, amount)
    SendPacket(buf, 1, 0)

def MoveToTownNPC(type, id, x, y):
    buf = pack(0x59, type, id, x, y)
    SendPacket(buf, 1, 0)

def ChangeRelationShip(id, k, v):
    buf = pack(0x5D, k, v, id)
    SendPacket(buf, 1, 0)

def RequestParty(id, v):
    buf = pack(0x5E, v, id)
    SendPacket(buf, 1, 0)

def SwitchWeapon():
    buf = pack(0x60)
    SendPacket(buf, 1, 0)

def ResurrectMerc(id):
    buf = pack(0x62, id)
    SendPacket(buf, 1, 0)
    
def MoveInvItemToBelt(id):
    buf = pack(0x63, id)
    SendPacket(buf, 1, 0)

def EquipMerc(idx):
    buf = pack(0x61, idx, 0)
    SendPacket(buf, 1, 0)
    
def SyncPosition(type, id):
    buf = pack(0x4B, type, id)
    SendPacket(buf, 1, 0)
    
def UpdatePosition(x, y):
    buf = pack(0x5F, x, y)
    SendPacket(buf, 1, 0)
    
