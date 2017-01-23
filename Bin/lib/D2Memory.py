from D2EM import *
import _struct


def ReadMemoryAsUInt(ptr):
    m = ReadMemory(ptr, 4)
    if not m: return None
    
    return _struct.unpack("<I", m)[0]

def GetSkillTreeString(stid):
    ccls = stid >> 3
    skid = stid & 7
    
    dtptr = GetD2DataTables()
    chr_ptr = ReadMemoryAsUInt(dtptr + 0x0BC4) + ccls * 0xC4
    
    sid = ReadMemoryAsUInt(chr_ptr + skid * 2 + 0x54) & 0xFFFF
    return GetD2StaticString(sid)

def DumpSkillTree():
    dtptr = GetD2DataTables()
    cls_ms = ReadMemoryAsUInt(dtptr + 0x0BC8)
    
    for i in range(cls_ms):
        for k in range(3):
            stid = (i << 3) | k
            DLog("0x%02X, %s" % (stid, GetSkillTreeString(stid)))

def GetCurrentPlayerPtr():
    d2client = GetModuleHandle("D2Client")
    if not d2client: return None
    
    return ReadMemoryAsUInt( d2client + 0x6FBCC3D0 - 0x6FAB0000 )

def GetActPtr():
    p = GetCurrentPlayerPtr()
    if not p: return None
    
    return ReadMemoryAsUInt( p + 0x1C )

__tomb_cls_ids = (
    (0x139, 0x26),
    (0x138, 0x27),
    (0x134, 0x28),
    (0x136, 0x29),
    (0x137, 0x2A),
    (0x135, 0x2B),
    (0x133, 0x2C)
    )

def GetMissingTomb():
    t = GetActPtr()
    if not t: return None
    
    idx = 6
    
    t = ReadMemoryAsUInt(t + 0x38)
    if t:
        t = ReadMemoryAsUInt(t + 0x3C0)
        if t:
            idx = t - 0x42
    
    if idx >= len(__tomb_cls_ids): return None
    
    return __tomb_cls_ids[idx]
    
    