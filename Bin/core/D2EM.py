from _D2EM import *
from D2Exception import *
import D2ServerPacket as _DSP
import D2ClientPacket as _DCP
import D2Event as _D2Event
import EM_Utils as _EM_Utils
import PickIt as _PickIt

#************************************
PFG_INVENTORY = 1
PFG_CHARACTER = 2
PFG_SKILLBUTTON = 3
PFG_SKILLTREE = 4
PFG_CHAT = 5
PFG_NPCMENU = 8
PFG_GAMEMENU = 9
PFG_AUTOMAP = 10
PFG_NPCTRADE = 12
PFG_IMMUE = 13
PFG_QUEST = 15
PFG_WAYPOINT = 20
PFG_MINIPANEL = 21
PFG_PARTY = 22
PFG_PLAYERTRADE = 23
PFG_MSGLOG = 24
PFG_STASH = 25
PFG_CUBE = 26
PFG_MERCINV = 36

WP_DST= (0x01,0x03,0x04,0x05,0x06,0x1B,0x1D,0x20,0x23,0x28,0x30,0x2A,0x39,
         0x2B,0x2C,0x34,0x4A,0x2E,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,0x53,
         0x65,0x67,0x6A,0x6B,0x6D,0x6F,0x70,0x71,0x73,0x7B,0x75,0x76,0x81)
WP_CLSID = (0x77, 0x9C, 0xED, 0x18E, 0x1AD)
STASH_CLSID = 0x010B
BLUE_TP_CLSID = 0x3B
RED_TP_CLSID = 0x3C

#************************************
EXEC_PATH = _D2Event.EXEC_PATH
LIB_PATH = _D2Event.LIB_PATH
SCRIPT_PATH = _D2Event.SCRIPT_PATH
LOG_PATH = _D2Event.LOG_PATH

PrintErr = _EM_Utils.PrintErr
RGB = _EM_Utils.RGB
PrintHex = _EM_Utils.PrintHex
GetDistance = _EM_Utils.GetDistance
CirclePoints = _EM_Utils.CirclePoints
DLog = _EM_Utils.DLog
IsInList = _PickIt.IsInList

Sleep = _D2Event.Sleep
_OnRecvPacket = _D2Event.OnRecvPacket
_OnSendPacket = _D2Event.OnSendPacket
_OnDraw = _D2Event.OnDraw
_OnExit = _D2Event.OnExit
_SetScript = _D2Event.SetScript
_UpdateSRate = _D2Event.UpdateSRate
GetScriptNzs = _D2Event.GetScriptNzs
GetCurScriptNzIdx = _D2Event.GetCurScriptNzIdx
GetScriptConfig = _D2Event.GetScriptConfig

ExitGame = _D2Event.ExitGame
ExitScript = _D2Event.ExitScript
SetGameTimeLimit = _D2Event.SetGameTimeLimit
InsertAPC = _D2Event.InsertAPC
EnableAPC = _D2Event.EnableAPC
IsAPCEnable = _D2Event.IsAPCEnable
AttachLib = _D2Event.AttachLib
SetTimer = _D2Event.SetTimer
SetLagHandler = _D2Event.SetLagHandler
GetLagStatus = _D2Event.GetLagStatus

AttachEvent = _D2Event.AttachEvent
DetachEvent = _D2Event.DetachEvent
EVT_ONLIFECHECK = _D2Event.EVT_ONLIFECHECK
EVT_ONPACKETUPDATE = _D2Event.EVT_ONPACKETUPDATE
EVT_ONWAKE = _D2Event.EVT_ONWAKE
EVT_ONDRAW = _D2Event.EVT_ONDRAW
EVT_ONEXIT = _D2Event.EVT_ONEXIT

del ASleep
del GetSHM
del SetSHM
del GetItemInfo
del GetItemStat
#------------------------------------

def _Init():
    _D2Event.Init()
    _PickIt.Init()

def _Reset():
    _D2Event.Reset()
    _PickIt.Reset()

def GetPlayerIds():
    return _D2Event._players.keys()
    
def GetCurrentPlayerId():
    return _D2Event._cp[0]
    
def GetPlayer(id=0):
    if not id:
        if _D2Event._cp and _D2Event._cp[4]:
            return tuple(_D2Event._cp[4])
        else:
            return None
    
    info = _D2Event._players.get(id, None)
    if info:
        return tuple(info)
    else:
        return info

def GetPlayerPosInfo(id):
    return GetUnit(12, 0, id)
    
def GetPlayerRelation(id=0):
    return GetUnit(7, 0, id)
    
def GetPartyInfo(id=0):
    return GetUnit(6, 0, id)
    
def GetPets(id=0):
    return GetUnit(4, 0, id)
    
def GetPetOwnerId(id):
    return GetUnit(4, 1, id)
    
def GetPortalIds(id=0):
    return GetUnit(5, 0, id)
    
def GetPortalOwnerId(id):
    return GetUnit(5, 2, id)

def GetUnitState(type=0, id=0):
    return GetUnit(0, type, id)

def GetName(type, id):
    return GetUnitName(type, id)

def GetUnitName(type, id):
    return GetUnit(1, type, id) 

def GetUnitPath(type, id):
    return GetUnit(2, type, id)
    
def GetUnitExtension(type, id):
    return GetUnit(3, type, id)
    
def GetAura(type, id):
    return GetUnit(11, type, id)
    
def HasAura(type, player_id, aura_id):
    m = GetAura(type, player_id)
    if m:
        ml = len(m)
        dm = divmod(aura_id, 8)
        if dm[0] < ml:
            val = ord( m[ dm[0] ] )
            return bool( val & (1 << dm[1]) )
            
    return False
    
def GetPlayerLife(id=0):
    if id == 0 or id == GetCurrentPlayerId():
        s = GetStat(0x06)
        if not s: return None
        return s[1]
        
    else:
        return GetUnit(8, 0, id)

#demon=2, undead=1, otherwise 0

def GetMerc(player_id=0):
    pets = GetPets(player_id)
    if not pets: return None
    
    for i in pets:
        if i[2] == 7:
            return i

def GetMercId(player_id=0):
    merc = GetMerc(player_id)
    if merc:
        return merc[1]
    else:
        return None
    
def GetNPCLife(id):
    if GetPetOwnerId(id) == GetCurrentPlayerId():
        return GetUnit(8, 1, id)
        
    else:
        s = GetStat(0x06, 0, 1, id)
        if s:
            return s[1]
        else:
            return None
    
def IsDead(type, id):
    s = GetStat( 0x06, 0, type, id )
    
    if not s or not s[1]: return True
    
    else: return False

def GetLife(type=0, id=0):
    if type == 0:
        return GetPlayerLife(id)
        
    elif type == 1:
        return GetNPCLife(id)

    else:
        return None
    
def IsTownLevel(lvl_no):
    if lvl_no == 0x01 or lvl_no == 0x28 or lvl_no == 0x4B or lvl_no == 0x67 or lvl_no == 0x6D:
        return True
    else:
        return False
    
def GetMapLevelNo(type=0, id=0):
    rm = GetRoom(type, id)
    if rm:
        return rm[4]
    else:
        return 0

def IsInTown(type=0, id=0):
    return IsTownLevel( GetMapLevelNo(type, id) )
    
def GetScreenCenter():
    w, h, c = GetWindowSize()[:3]
    return (w / 2 + c, h /2)
    
def GetScreenOffset(type=0, id=0):
    c, rcx, rcy = GetWindowSize()[2:]
    rp = GetUnit(10, type, id)
    if not rp: return None
    
    return ( rp[0] - (rcx - c), rp[1] - rcy )
    
def GetPing():
    return GetGameInfo(1)

def GetSkillUIdBySide(side):
    return _D2Event._cp[ 2 + (side & 1) ]

def HasWayPoint(idx):
    f = _D2Event._gdata[0]
    if not f or idx < 0 or idx >= 40: return False
    
    r = divmod(idx, 8)
    if ( ord( f[ r[0] ] ) & (1 << r[1]) ):
        return True
    else:
        return False

def GetQuestState(quest_idx, offset):
    q = _D2Event._gdata[2]
    b = (quest_idx << 4) + offset
    r = divmod(b, 8)
    if not q or len(q) <= r[0]: return None

    return bool( ord(q[r[0]]) & (1 << r[1]) )

def GetActNo():
    return _D2Event._gdata[1] + 1
    
def GetCurPlayerFlag(idx, val=0):
    return GetFlag(1, idx, val)
    
def GetPanelFlag(idx):
    return GetFlag(0, idx)

def IsAttackable(type, id):
    return IsAttackableEx(type, id)

def Run(x, y, max_dist=3, max_retry=0, sync_req=False):
    tick = 0
    pos = GetPos()
    if pos[0] == x and pos[1] == y and not sync_req: return True
    
    last_pos = None
    while True:
        while True:
            p = GetWalkPath(pos[0], pos[1], x, y)
            if p: break
            if sync_req:
                DLog("Run::1::No Path", 2)
                DLog("Run::2::Sync Position", 2)
                sync_req = False
                SyncPosition()
                npos = GetPos()
                if npos[0] != pos[0] or npos[1] != pos[1]:
                    pos = npos
                    continue
                
            else:
                DLog("Run::2::No Path src(%04X,%04X) dst(%04X,%04X)" % (pos[0], pos[1], x, y), 1)
                #if IsInTown():
                #    DumpCurrentMap("c:\\m%02x_%08x_%08x_%08x_%s.txt" % (GetMapLevelNo(), GetCurrentGameId(), x | y << 16, pos[0] | pos[1] << 16, p==None), 0x1C01)
            
            return False

        lpos = p[-1]
        if (x != lpos[0] or y != lpos[1]) and GetDistance(lpos[0], lpos[1], x, y) <= 20:
            p += ((x,y),)
        
        for i in p:
            if i[0] == pos[0] and i[1] == pos[1]: continue
            
            tval = GetTickInterval(tick)
            while tval < 50 or tval < 200 and last_pos == i:
                Sleep(10)
                tval = GetTickInterval(tick)
            
            sync_req = True
            last_pos = i
            _DSP.Move(i[0], i[1])
            tick = GetTickCount()
            
            Sleep(50)
            while GetCurPlayerFlag(0) or GetCurPlayerFlag(3) or GetCurPlayerFlag(1):
                if GetTickInterval(tick) < 20000:
                    Sleep(10)
                else:
                    DLog("Run::Error::TimeOut!", 1)
                    return False
    
            pos = GetPos()
            if GetDistance(pos[0], pos[1], i[0], i[1]) > 2:
                break
        
        if GetDistance(pos[0], pos[1], x, y) <= max_dist:
            if sync_req:
                sync_req = False
                SyncPosition()
                pos = GetPos()
                if GetDistance(pos[0], pos[1], x, y) <= max_dist:
                    break
                else:
                    DLog("Run::1::Sync Position", 2)
            else:
                break
        
        max_retry -= 1
        if not max_retry:
            if sync_req:
                DLog("Run::3::Sync Position", 2)
                SyncPosition()
            return False
        
    return True
    
def Teleport(x, y, max_dist=3, max_retry=0, timeout=800):
    pos = GetPos()
    if pos[0] == x and pos[1] == y: return True
    
    tick = GetTickCount()
    while True:
        if GetDistance(pos[0], pos[1], x, y) <= 35:
            p = ((x, y),)
        else:
            p = GetTeleportPath(pos[0], pos[1], x, y)
            if not p:
                DLog("Teleport::Fail to find path!", 1)
                #DumpCurrentMap("c:\\m%02x_%08x_%08x_%08x_%s.txt" % (GetMapLevelNo(), GetCurrentGameId(), x | y << 16, pos[0] | pos[1] << 16, p==None), 0x1C01)
                return False
            
            lpos = p[-1]
            if (x != lpos[0] or y != lpos[1]) and GetDistance(lpos[0], lpos[1], x, y) <= 35:
                p += ((x,y),)

        for i in p:
            if i[0] == pos[0] and i[1] == pos[1]: continue
            
            if not SelectSkill(0x36, 1): return False
            
            _DSP.CastSkill(i[0], i[1], 1)
            #print "Tele To <%x, %x> from <%x, %x>, DST<%x, %x>, MS[%d]" % (i[0], i[1], pos[0], pos[1], x, y, GetTickCount() - tick)
            tick = GetTickCount()
            
            _D2Event.wait4_reassignment(timeout, i[0], i[1])
            while GetTickInterval(tick) < 180: Sleep(10)
            
            pos = GetPos()
            if GetDistance(pos[0], pos[1], i[0], i[1]) > 2:
                break
            
        if GetDistance(pos[0], pos[1], x, y) <= max_dist:
            break
        
        max_retry -= 1
        if not max_retry:
            return False
        
    return True

def RunToUnit(type, id, max_dist=3):
    tick = 0
    sync_req = False
    while True:
        tpos = GetPos(type, id)
        if not tpos: return False
        
        mpos = GetPos()
        dist = GetDistance(mpos[0], mpos[1], tpos[0], tpos[1])
        
        if dist <= max_dist:
            if sync_req:
                sync_req = False
                SyncPosition()
                continue
            
            break
        
        elif dist > 10:
            if not Run(tpos[0], tpos[1], 10, 0, sync_req):
                return False
            
            sync_req = False
            continue
        
        while GetTickInterval(tick) < 100:
            Sleep(10)
        
        sync_req = True
        _DSP.MoveToUnit(type, id)
        tick = GetTickCount()
        
        Sleep(50)
        while GetCurPlayerFlag(0) or GetCurPlayerFlag(3) or GetCurPlayerFlag(1):
            Sleep(10)
            
    return True

def IntTownNPC(id):
    while not GetPanelFlag(PFG_NPCMENU):
        if not RunToUnit(1, id, 4): return False
        
        #print "Int NPC[0x%X]" % id
        _DSP.IntUnit(1, id)
        if _D2Event.wait4_npc_int(2000, id): Sleep(500)
        
        if GetPanelFlag(PFG_NPCMENU):
            Sleep(300)
            break
        
        DLog("Retry::Int NPC[0x%X]" % id, 2)
        
    return True

def ClickOnMenu(menu_idx):
    dlg = GetNPCDialog()
    if not dlg: return
    
    x = dlg[2] + int(dlg[4] / 2)
    y = dlg[3]
    es = dlg[6]
    l = len(es)
    
    #print "Get NPC[0x%X] Dialog[%d]" % (id, l)
    
    if menu_idx >= l:
        DLog("Menu IDX[%d] Out Of Range" % menu_idx, 1)
        return
        
    k = 0
    while k < l:
        y += es[k][1]
        if k == menu_idx: break
        k += 1
            
    y -= 4
        
    #print "Click On Menu(%d, %d)" % (x, y)
    PostMessage(0x0201, 1, x | (y << 16) )
    PostMessage(0x0202, 0, x | (y << 16) )
    Sleep(300)
    

def IntStash(id=0):
    if not id:
        units = GetNearbyUnits(2, STASH_CLSID)
        if not units:
            DLog("No Stash Id", 1)
            return
        id = units[0][1]
    
    while not GetPanelFlag(PFG_STASH):
        #print "Open Stash[0x%X]" % id
        _DSP.IntUnit(2, id)
        
        tick = GetTickCount()
        while not GetPanelFlag(PFG_STASH):
            Sleep(50)
            if GetTickInterval(tick) >= 800:
                break
            
        DLog("Retry::Open Stash[0x%X]" % id, 2)
    
def IntPortal(id):
    old_lvl_no = GetMapLevelNo()
    
    DLog("Int Portal[0x%X]" % id)
    
    _DSP.IntUnit(2, id)
    if _D2Event.wait4_lvlno(800, old_lvl_no):
        Sleep(200)
        return True
    
    return False

def IntUnit5(id):
    old_lvl_no = GetMapLevelNo()
    
    while True:
        _DSP.IntUnit(5, id)
        #print "Int Unit5[0x%X]" % id
        if _D2Event.wait4_lvlno(800, old_lvl_no):
            Sleep(200)
            break
        DLog("Retry::Int Unit5[0x%X]" % id, 2)
        
        pos = GetPos()
        kpos = GetPos(5, id)
        if not kpos or GetDistance(pos[0], pos[1], kpos[0], kpos[1]) > 8:
            return False
        
    return True

def IntWayPoint(id, wp_idx):
    while not GetPanelFlag(PFG_WAYPOINT):
        _DSP.IntUnit(2, id)
        #print "Int WayPoint[0x%X]" % id
        if _D2Event.wait4_wp_up(800):
            break
        DLog("Retry::Int WayPoint[0x%X]" % id, 2)
        
    if GetPanelFlag(PFG_WAYPOINT):
        PostEsc()
    
    Sleep(200)
    if not HasWayPoint(wp_idx):
        DLog("No WayPoint[0x%X] Index[%d]" % (id, wp_idx), 1)
        return False
    
    old_lvl_no = GetMapLevelNo()
    while True:
        _DSP.IntWaypoint(id, WP_DST[wp_idx])
        #print "Open WayPoint[0x%X] Dst[%d]" % (id, WP_DST[wp_idx])
        if _D2Event.wait4_lvlno(800, old_lvl_no):
            Sleep(200)
            break
        DLog("Retry::Open WayPoint[0x%X] Dst[%d]" % (id, WP_DST[wp_idx]), 2)
    
    return True


def SellItem(npc_id, item_id, flag):
    _DSP.SellItem(npc_id, item_id, 0, flag)
    
    if _D2Event.wait4_ts_done(800):
        return True

    return False

def BuyItem(npc_id, item_id, flag):
    _DSP.BuyItem(npc_id, item_id, 0, flag)
    
    if _D2Event.wait4_ts_done(800):
        return True

    return False


def PickItem(id, cursor=0):
    _DSP.PickItem(id, cursor & 1)
    if _D2Event.wait4_sd_up(500, 1):
        return True

    return False

def DropItem():
    while True:
        id = GetInvInfo()[2]
        if not id: return
        
        _DSP.DropItem( id )
        if _D2Event.wait4_cursor_clear(500):
            Sleep(200)
            return
        
def SelectSkill_NB(skill_id, side, flag=0xFFFFFFFF):
    if GetSkillUIdBySide(side) == (skill_id, flag):
        return True
    
    skill = GetSkill(skill_id, flag)
    if not skill:
        DLog("No Such Skill[0x%X][0x%X]" % (skill_id, flag), 1)
        return False
    
    #print "Select Skill Non-blocking[%d][0x%X][0x%X]" % (side, skill_id, flag)
    _DSP.SelectSkill(skill_id, side, flag)

def SelectSkill(skill_id, side, flag=0xFFFFFFFF):
    if GetSkillUIdBySide(side) == (skill_id, flag):
        return True
    
    skill = GetSkill(skill_id, flag)
    if not skill:
        DLog("No Such Skill[0x%X][0x%X]" % (skill_id, flag), 1)
        return False
    
    while True:
        #print "Select Skill[%d][0x%X][0x%X]" % (side, skill_id, flag)
        _DSP.SelectSkill(skill_id, side, flag)
        if _D2Event.wait4_skill(800, (skill_id, flag), side):
            Sleep(0)
            break
    
        DLog("Retry::Select Skill[%d][0x%X][0x%X]" % (side, skill_id, flag), 2)
    
    return True

def PostKey(key):
    PostMessage(0x0100, key, 0)
    PostMessage(0x0101, key, 0)
    
def PostEsc():
    PostKey(0x1B)

def CastSkill(x, y, side, recast=False):
    _DSP.CastSkill(x, y, side, recast)

def CastSkillOnUnit(type, id, right=False, recast=False, castop=False):
    _DSP.CastSkillOnUnit(type, id, right, recast, castop)
    
def SwitchWeapon():
    while True:
        _DSP.SwitchWeapon()
        if _D2Event.wait4_weapon_chg(800):
            break
        
        DLog("Retry::Switch Weapon[%d]" % ( GetWeaponWzNo(), ), 2)
    
def UseItemFromBelt(id, type=0, sync=False):
    _DSP.UseItemFromBelt(id, type)
    if not sync or _D2Event.wait4_item_chg(800, id, 0x0F):
        return True
    
    return False
    
def RepairAllItems(npcId):
    _DSP.RepairAllItems(npcId)
    return _D2Event.wait4_ts_done(800)
    
def IdentifyAllItems(cain_id):
    _DSP.IdentifyAllItems(cain_id)
    return _D2Event.wait4_ts_done(800)
    
def ResurrectMerc(id):
    _DSP.ResurrectMerc(id)
    return _D2Event.wait4_ts_done(800)
    
def CancelTownNPCInt(type, id):
    _DSP.CancelTownNPCInt(type, id)

def RemoveItemFromInv(id):
    _DSP.RemoveItemFromInv(id)
    return _D2Event.wait4_item_chg(800, id, 0x05)
    
def AddItemToInv(id, x, y, dst):
    _DSP.AddItemToInv(id, x, y, dst)
    return _D2Event.wait4_item_chg(800, id, 0x04)

def wait4_cursor_clear(ms):
    return _D2Event.wait4_cursor_clear(ms)

def ClickButton(id, val):
    _DSP.ClickButton(id, val)

def EquipMerc(idx):
    _DSP.EquipMerc(idx)
    
def SyncPosition(timeout=800, type=0 , id=0):
    if not type and not id:
        id = GetCurrentPlayerId()
        
    while True:
        _DSP.SyncPosition(type, id)
        if _D2Event.wait4_reassignment(timeout, 0, 0, 0):
            break
        
        DLog("Retry::SyncPosition()", 2)
    
#06: invite, 07: cancel, 08: accept
def RequestParty(player_id, val):
    while True:
        if player_id not in GetPlayerIds(): return False
        
        _DSP.RequestParty(player_id, val)
        
        if _D2Event.wait4_partystate_chg(800, player_id):
            break
        
    return True
        
def GetCurrentGameId():
    return _D2Event._game_id

def SendMessage(msg, overhead=0):
    if msg:
        if overhead:
            _DSP.SendOverheadMessage(msg)
        else:
            _DSP.SendMessage(msg)

def Loot(id):
    _DSP.IntUnit(0, id)
    
def UseItemFromInv(item_id, x, y):
    _DSP.UseItemFromInv(item_id, x, y)
    
def AddItemToCube(item_id, cube_id):
    _DSP.AddItemToCube(item_id, cube_id)
    
def IntUnit2(id):
    _DSP.IntUnit(2, id)
    
    