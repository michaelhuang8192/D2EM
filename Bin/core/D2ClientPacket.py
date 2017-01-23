from _D2EM import SendPacket
import _struct

_packet_fmt = [None] * 256
_packet_fmt[0x07] = "<BHHB" #add map
_packet_fmt[0x08] = "<BHHB" #remove map
_packet_fmt[0x09] = "<BBIBHH" #add warp
_packet_fmt[0x0A] = "<BBI" #remove unit
_packet_fmt[0x0D] = "<BxIxHHxB" #stop player
_packet_fmt[0x0F] = "<BxIBHHxHH" #move player
_packet_fmt[0x10] = "<BxIBBIHH" #move player to unit
_packet_fmt[0x15] = "<BBIHHB" #reassign player
_packet_fmt[0x23] = "<BxIBHI" #select skill
_packet_fmt[0x4C] = "<BBIHBBIxx" #cast skill on unit
_packet_fmt[0x4D] = "<BBIHxxBHHxx" #cast skill
_packet_fmt[0x2C] = "<BBIH" #sound
_packet_fmt[0x8B] = "<BIB" #party state

###
_packet_fmt[0xAC] = "<BIHHHB" #assign npc, (id, cls_id, x, y, life)
_packet_fmt[0x6D] = "<BIHHB" #stop npc, (id, x, y, life)
_packet_fmt[0x67] = "<BIxHH" #move npc, (id, x, y)
_packet_fmt[0x68] = "<BIBHHBI" #move npc to unit, (id, mode, x, y, unit_type, unit_id)
_packet_fmt[0x0C] = "<BxIxxB" #update npc life(id, life)
_packet_fmt[0x6C] = "<BIBBIxHH" #(id, skill, unit_type, unit_id, x, y)
_packet_fmt[0x6B] = "<BIBHHxxHH" #(id, v, dst_x, dst_y, src_c, src_y)
_packet_fmt[0x69] = "<BIBHH" #(id, state, x, y)

_packet_fmt[0x51] = "<BBIHHHBB" #(type, id, cls_id, x, y, state, val0)


def unpack(buf):
    if len(buf):
        packet_id = ord(buf[0])
        if packet_id >= 0 and packet_id < len(_packet_fmt):
            fmt = _packet_fmt[packet_id]
            if fmt:
                return _struct.unpack_from(fmt, buf, 0)
        
    return None


def pack(packet_id, *args):
    if packet_id >= 0 and packet_id < len(_packet_fmt):
        fmt = _packet_fmt[packet_id]
        if fmt:
            return _struct.pack(fmt, packet_id, *args)
    
    return None



def CastSkillOnUnit(unit_type, unit_id, skill_id, skill_lvl, target_type, target_id):
    buf = pack(0x4C, unit_type, unit_id, skill_id, skill_lvl, target_type, target_id)
    SendPacket(buf, 0, 1)

def CastSkill(unit_type, unit_id, skill_id, skill_lvl, x, y):
    buf = pack(0x4D, unit_type, unit_id, skill_id, skill_lvl, x, y)
    SendPacket(buf, 0, 1)
    
def AddMap(x, y, level):
    buf = pack(0x07, x, y, level)
    SendPacket(buf, 0, 1)

def RemoveMap(x, y, level):
    buf = pack(0x08, x, y, level)
    SendPacket(buf, 0, 1)

def AddWarp(type, id, v, x, y):
    buf = pack(0x09, type, id, v, x, y)
    SendPacket(buf, 0, 1)
    
def RemoveUnit(type, id):
    buf = pack(0x0A, type, id)
    SendPacket(buf, 0, 1)
    
def MovePlayer(id, mode, src_x, src_y, dst_x, dst_y):
    buf = pack(0x0F, id, mode, dst_x, dst_y, src_x, src_y)
    SendPacket(buf, 0, 1)
    
def StopPlayer(id, x, y, life):
    buf = pack(0x0D, id, x, y, life)
    SendPacket(buf, 0, 1)
    
def MovePlayerToUnit(player_id, mode, src_x, src_y, unit_type, unit_id):
    buf = pack(0x10, player_id, mode, unit_type, unit_id, src_x, src_y)
    SendPacket(buf, 0, 1)

def ReassignUnit(type, id, x, y, mode):
    buf = pack(0x15, type, id, x, y, mode)
    SendPacket(buf, 0, 1)
    
def SelectSkill(id, skill, right=False, flag=0xFFFFFFFF):
    side = 1
    if right: side = 0
    
    buf = pack(0x23, id, side, skill, flag)
    SendPacket(buf, 0, 1)

def PrintMsg(level, name, msg):
    buf = _struct.pack("<BHHIB%dsB%dsB" % (len(name), len(msg)), 0x26, 0x01, 0x02, 0x00, level, name, 0x00, msg, 0x00)
    SendPacket(buf, 0, 1)
    
    