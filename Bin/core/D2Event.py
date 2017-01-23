from _D2EM import *
from D2Exception import *
import D2ServerPacket as _DSP
import D2ClientPacket as _DCP
import EM_Utils
import _struct
import _heapq

EXEC_PATH = GetExecPath()
LIB_PATH = EXEC_PATH + "\\lib"
SCRIPT_PATH = EXEC_PATH + "\\script"
LOG_PATH = EXEC_PATH + "\\log"

EVT_ONLIFECHECK = 0
EVT_ONPACKETUPDATE = 1
EVT_ONWAKE = 2
EVT_ONDRAW = 3
EVT_ONEXIT = 4
EVT_MAX = 5

__libs = {}
_game_id = 0
_config = None


def Init():
    global _cp, _players, _units, _packets, _callbacks, _wlists, _gdata, _time_limit, _game_id, _lsr, _timer
    
    _cp = None
    _players = {}
    _units = ({}, {}, {}, {}, {}, {})
    _packets = []
    _callbacks = None
    _wlists = None
    _gdata = [None, -1, None]
    _time_limit = None
    _game_id = (_game_id + 1) & 0xFFFFFFFF
    _lsr = [None, False, False, None, None, 0]
    _timer = None
    
    _call_libs(0)

def Reset():
    _call_libs(1)
    
    global _cp, _players, _units, _packets, _callbacks, _wlists, _gdata, _time_limit, _lsr, _timer
    _cp = _players = _units = _packets = _callbacks = _wlists = _gdata = _time_limit = _lsr = _timer = None
    
def SetScript(s, nidx):
    global _callbacks, _wlists, _time_limit, _config, _timer
    
    _time_limit = None
    
    if s != None:
        _config[0] = nidx
        _callbacks = [ s, 0, [], [], [], [], [], [], 0 ]
        _timer = (GetTickCount(), {}, [])
        _wlists = ([], [])
        _call_libs(2)
    else:
        _call_libs(3)
        _callbacks = _timer = _wlists = None
        _config[0] = -1

def GetScriptNzs():
    return _config[1]
    
def GetCurScriptNzIdx():
    return _config[0]

def UpdateSRate(sc_count):
    cur_gcount = GetSHM(1)
    cur_srate = GetSHM(2)
    srate = ( cur_srate * (cur_gcount - 1) + float(sc_count) / len(GetScriptNzs()) * 10000 ) / cur_gcount
    SetSHM(2, int(srate))
    
def __load_script_nz(d_val):
    snz = GetSHM(3)
    if not snz: return d_val
    
    nsnz = []
    d = snz.split(',')
    for i in d:
        n = i.strip()
        if n: nsnz.append(n)
        
    if nsnz:
        return nsnz
    else:
        return d_val

def __load_config():
    global _config
    _config = [ -1, (), {}, () ]
    
    fname = SCRIPT_PATH + "\\config.ini"
    cfd = EM_Utils.ParseConfigFile(fname)
    script_nzs = []
    if cfd:
        cf = _config[2]
        for s in cfd:
            if s[0] == 'script':
                for k in s[1]:
                    try:
                        if int(k[1]): script_nzs.append(k[0])
                    except:
                        pass
            else:
                sd = cf.get(s[0])
                if not sd: cf[ s[0] ] = sd = {}
                
                for k in s[1]:
                    sd[ k[0] ] = k[1]
    
    _config[1] = script_nzs = tuple(__load_script_nz(script_nzs))
    _config[3] = cfs = [None,] * len(script_nzs)
    snzd = {}
    nidx = 0
    for n in script_nzs:
        snzd[n] = nidx
        cf = cfs[nidx] = {}
        nidx += 1
        fname = SCRIPT_PATH + "\\" + n + "\\config.ini"
        cfd = EM_Utils.ParseConfigFile(fname)
        if not cfd: continue
        
        for s in cfd:
            sd = cf.get(s[0])
            if not sd: cf[ s[0] ] = sd = {}
            
            for k in s[1]:
                sd[ k[0] ] = k[1]
                
    sce = GetSHM(4)
    if not sce: return
    sce_v = sce.split('\x00')
    for i in range(0, len(sce_v) - 1, 2):
        k = sce_v[i]
        v = sce_v[i + 1]
        kp = k.split(".")
        kp_len = len(kp)
        if kp_len == 3:
            if not kp[0] or not kp[1] or not kp[2]: continue
            nidx = snzd.get(kp[0], -1)
            if nidx < 0: continue
            
            cf = _config[3][nidx]
            sect = kp[1]
            key = kp[2]
        else:
            cf = _config[2]
            if kp_len == 2:
                if not kp[0] or not kp[1]: continue
                sect = kp[0]
                key = kp[1]
            else:
                sect = "global"
                key = k
                
        sd = cf.get(sect)
        if not sd: cf[sect] = sd = {}
        sd[key] = v

def GetScriptConfig(key, sect=None, default_val=None):
    sidx = GetCurScriptNzIdx()
    if sidx < 0 or sidx >= len( GetScriptNzs() ): return default_val
    
    if not sect: sect = "global"
    if key:
        cf = _config[3][sidx].get(sect)
        if cf and cf.has_key(key):
            return cf[key]
        else:
            cf = _config[2].get(sect)
            if cf:
                return cf.get(key, default_val)
            else:
                return default_val
    else:
        cf = _config[2].get(sect)
        if cf:
            ccf = cf.copy()
        else:
            ccf = {}
        
        cf = _config[3][sidx].get(sect)
        if cf: ccf.update(cf)
        
        if ccf:
            return ccf
        else:
            return default_val
    
#
__load_config()
#

def OnDraw():
    if not _callbacks: return
    
    f = get_func(_callbacks[0], "OnDraw")
    if f:
        fz = _callbacks[EVT_ONDRAW + 2] + [(None, f)]
    else:
        fz = _callbacks[EVT_ONDRAW + 2]
    
    try:
        for f in fz:
            try:
                f[1]()
            except Exception, e:
                EM_Utils.DLog("OnDraw() Callback Error:", 3)
                EM_Utils.DLog(e, 3)
                EM_Utils.PrintErr()
                
    finally:
        f = None
        fz = None
        
def OnExit():
    if not _callbacks: return
    
    f = get_func(_callbacks[0], "OnExit")
    if f:
        fz = _callbacks[EVT_ONEXIT + 2] + [(None, f)]
    else:
        fz = _callbacks[EVT_ONEXIT + 2]
    
    try:
        for f in fz:
            try:
                f[1]()
            except Exception, e:
                EM_Utils.DLog("OnExit() Callback Error:", 3)
                EM_Utils.DLog(e, 3)
                EM_Utils.PrintErr()
                
    finally:
        f = None
        fz = None
        
def OnRecvPacket(size, type, buf):
    #print "ToC:",
    #EM_Utils.PrintHex(buf)
    ret = parse_client_packet(size, type, buf)
    relay_packet(buf, 1)
    return ret
    
    
def parse_client_packet(size, type, buf):
    global _cp
    
    pid = ord(buf[0])
    
    if pid == 0x0A: #remove unit
        type, id = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (type, id)) )
        
        
    elif pid == 0x59: #assign player
        id, cls_id = _struct.unpack_from("<IH", buf, 1)
        x, y = _struct.unpack_from("<HH", buf, 22)

        if not _cp: _cp = [id, cls_id, None, None, None] #l_skill, r_skill, ps
        
        _packets.append( (pid, 1, (0, id, cls_id, x, y)) )


    elif pid == 0x5B: #join game
        id, cls_id, level = _struct.unpack_from("<IB16xH", buf, 3)
        
        idx = buf.find('\x00', 8, 24)
        name = None
        if idx != -1:
            name = buf[8:idx]
        
        player = [id, cls_id, level, name, None]
        if not _players.has_key(id):
            _players[id] = player
            
        if _cp and _cp[0] == id: _cp[4] = player
        
        _packets.append( (pid, 1, (id, cls_id, level, name)) )
        
        
    elif pid == 0x5C: #leave game
        id = _struct.unpack_from("<I", buf, 1)[0]
        
        if _players.has_key(id):
            if _cp and _cp[0] == id: _cp = None
            
            del _players[id]
            
            _packets.append( (pid, 1, (id,)) )
            
            
    elif pid == 0x5A:
        if ord(buf[1]) == 0x02:
            char_name = acc_name = None
            idx = buf.find('\x00', 8, 24)
            if idx != -1:
                char_name = buf[8:idx]
                
            idx = buf.find('\x00', 24, 40)
            if idx != -1:
                acc_name = buf[24:idx]
            
            for p in _players.itervalues():
                if p[3] and p[3] == char_name:
                    p[4] = acc_name
                    break
                

    elif pid == 0x23:
        id, side, skill, flag = _DCP.unpack(buf)[1:]
        if _cp[0] == id:
            if side == 0x01:
                _cp[2] = (skill, flag)
            else:
                _cp[3] = (skill, flag)
                
    
    elif pid == 0x0D: #player stop
        id, x, y, life = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, x, y)) )
            
            
    elif pid == 0x0F: #player move
        id, mode, d_x, d_y, s_x, s_y = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, mode, d_x, d_y, s_x, s_y)) )


    elif pid == 0x10: #player move to unit
        id, mode, u_type, u_id, s_x, s_y = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, mode, u_type, u_id, s_x, s_y)) )
            
    
    elif pid == 0xAC: #assign npc
        id, cls_id, x, y, life = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, cls_id, x, y, life * 0.78125)) )
        
        
    elif pid == 0x6D: #npc stop
        id, x, y, life = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, x, y, life * 0.78125)) )
            
    
    elif pid == 0x67: #npc move
        id, d_x, d_y = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, d_x, d_y)) )
            
    
    elif pid == 0x68: #npc move to unit
        id, mode, s_x, s_y, u_type, u_id = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, mode, u_type, u_id, s_x, s_y)) )
            
            
    elif pid == 0x6c: #npc attack
        id, skill, t_type, t_id, s_x, s_y = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, skill, t_type, t_id, s_x, s_y)) )
        
        
    elif pid == 0x4D:
        type, id, skill, skill_lvl, d_x, d_y = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (type, id, skill, skill_lvl, d_x, d_y)) )
        
        
    elif pid == 0x4C:
        type, id, skill, skill_lvl, t_type, t_id = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (type, id, skill, skill_lvl, t_type, t_id)) )
        
        
    elif pid == 0x69:
        id, state, x, y = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, state, x, y)) )
        
        
    elif pid == 0x51:
        type, id, cls_id, x, y, state, val0 = _DCP.unpack(buf)[1:]
        _packets.append( (pid, 1, (id, cls_id, x, y, state, val0)) )
    
    
    elif pid == 0x0E:
        type, id, state = _struct.unpack_from("<BIxxB", buf, 1)
        if type == 2:
            _packets.append( (pid, 1, (id, state)) )
    
    
    elif pid == 0x63: #waypoint
        _gdata[0] = buf[7:12]
        
        
    elif pid == 0x9D or pid == 0x9C:
        action = ord(buf[1])
        if action == 0x00 or action == 0x02 or action == 0x03:
            id = _struct.unpack_from("<I", buf, 4)[0]  
            _packets.append( (pid, 1, (action, id)) )


    elif pid == 0x03:
        _gdata[1] = ord(buf[1])
        
        
    elif pid == 0x28:
        _gdata[2] = buf[7:]
        
        
    elif pid == 0x8F:
        if _lsr[0] != None:
            tv = GetTickInterval(_lsr[0])
            if tv >= 500:
                _lsr[1] = True
                _lsr[2] = False
                EM_Utils.DLog("<Lag Ended>")
                
        _lsr[0] = None
        
    
    return True
    
def OnSendPacket(size, type, buf):
    #print "ToS:",
    #EM_Utils.PrintHex(buf)
    ret = parse_server_packet(size, type, buf)
    relay_packet(buf, 0)
    return ret

def parse_server_packet(size, type, buf):
    pid = ord(buf[0])
    
    if pid == 0x15:
        if ord(buf[3]) == 0x2E:
            _packets.append( (pid, 0, (buf[4:-3],)) )
            return False
    
    elif pid == 0x6D:
        global _lsr
        if _lsr[0] == None:
            _lsr[0] = GetTickCount()
    
    #elif pid == 0x13:
    #    EM_Utils.PrintHex(buf)
    
    return True

def relay_packet(buf, dst):
    #relay_packet(buf, 1)
    if not _wlists: return
    
    i = 0
    wl = _wlists[dst]
    while i < len(wl):
        wid, f, v = wl[i]
        ret = f(v, buf)
        if ret:
            del wl[i]
            continue
        
        i += 1

__nid = 0
def get_new_id():
    global __nid
    
    __nid = (__nid + 1) & 0xFFFFFFFF
    return __nid

def call_lag_handler():
    if not _callbacks or _callbacks[1] >= 32: return
    
    _callbacks[1] += 32
    try:
        try:
            _lsr[3](_lsr[5], _lsr[4])
        except Exception, e:
            EM_Utils.DLog("Lag Callback Error:", 3)
            EM_Utils.DLog(e, 3)
            EM_Utils.PrintErr()
            
    finally:
        _callbacks[1] -= 32
        
def SetLagHandler(func, func_data):
    if func:
        _lsr[3] = func
        _lsr[4] = func_data
    else:
        _lsr[3] = _lsr[4] = None

def GetLagStatus():
    return (_lsr[2], _lsr[5] )

def AttachEvent(evt_id, func):
    idx = evt_id + 2
    if evt_id >= EVT_MAX or idx >= len(_callbacks):
        return -1
    
    ef_id = get_new_id()
    _callbacks[idx].append( (ef_id, func) )
    return ef_id

def DetachEvent(evt_id, ef_id):
    idx = evt_id + 2
    if evt_id >= EVT_MAX or idx >= len(_callbacks):
        return
    
    cbs = _callbacks[idx]
    i = 0
    l = len(cbs)
    while i < l:
        if cbs[i][0] == ef_id:
            del cbs[i]
            break
        
        i += 1

def InsertAPC(func, arg):
    if IsAPCEnable():
        _callbacks[5 + 2].append( (arg, func) )
        return True
    else:
        return False
    
def EnableAPC(enable=True):
    _callbacks[6 + 2] = enable
    
def IsAPCEnable():
    return _callbacks[6 + 2]

def SetTimer(tmr_id, elapse, func, func_data):
    if tmr_id != None:
        tmr = _timer[1].get(tmr_id)
        if not tmr: return None
        
        if elapse:
            tmr[0] = GetTickInterval(_timer[0]) + elapse
            tmr[1] = elapse
            if func: tmr[2] = func
            if func_data: tmr[3] = func_data
        else:
            del _timer[1][tmr_id]
            if tmr[5]:
                tmr_ix = 0
                for k in _timer[2]:
                    if k[4] == tmr[4]: break
                    tmr_ix += 1
                
                if tmr_ix < len(_timer[2]): del _timer[2][tmr_ix]
                
        if tmr[5]: _heapq.heapify(_timer[2])
        
    else:
        tmr_id = get_new_id()
        tmr = [ GetTickInterval(_timer[0]) + elapse, elapse, func, func_data, tmr_id, True ]
        _timer[1][tmr_id] = tmr
        _heapq.heappush(_timer[2], tmr)
        
    return tmr_id

def AttachLib(entry_func):
    key = id( entry_func )
    if not __libs.has_key(key):
        __libs[key] = entry_func
        #print ":0:Call Lib Entry(0) - %s" % (entry_func,)
        entry_func(0)
        entry_func(2)
        
    else:
        pass
        #print ":0:Lib Entry Exists - %s" % (entry_func,)

def _call_libs(reason):
    for f in __libs.itervalues():
        #print ":1:Call Lib Entry(%d) - %s" % (reason, f)
        f(reason)

def add_to_wlist(f, v, type):
    wid = get_new_id()
    _wlists[type].append( (wid, f, v) )
    return wid

def del_from_wlist(wid, type):
    wl = _wlists[type]
    for i in range( len(wl) ):
        if wl[i][0] == wid:
            del wl[i]
            break

def ExitGame(ec=0xFF, estr=None):
    raise GameExitedException(ec, estr)

def ExitScript(ec=0xFF, estr=None):
    raise ScriptExitedException(ec, estr)

def Sleep(ms, val=None):
    cts = GetTickCount()
    cms = ms
    while True:
        wdms = __get_next_tmr_wdms()
        if wdms != None and wdms < cms: cms = wdms
        cms = ASleep(cms)
        
        code = GetErrorCode()
        if code != 0:
            ExitGame(code)
        
        if _time_limit and GetTickInterval(_time_limit[1]) >= _time_limit[0]:
            if _time_limit[2]:
                ExitGame(9)
            else:
                ExitScript(9)
        
        cp = GetUnit()
        if cp and (cp[1] == 0x00 or cp[1] == 0x11 or (cp[2] & 0x10000)):
            ExitGame(2)
        
        #lag handler
        if not _lsr[2] and _lsr[0] != None and GetTickInterval(_lsr[0]) >= 500:
            _lsr[1] = True
            _lsr[2] = True
            EM_Utils.DLog("<Lag Detected>")
            
        if _lsr[1]:
            if _lsr[3]:
                call_lag_handler()
            
            if _lsr[2]:
                continue
            else:
                _lsr[1] = False
        #
        
        check_life()
        dispatch_packets()
        timer_pool()
        
        cms = ms - GetTickInterval(cts)
        if val and val[0]: break
        if cms <= 0: break
        
    on_wake()
    dispatch_apc()
    
    cms = ms - GetTickInterval(cts)
    if cms <= 0:
        return 0
    else:
        return cms

#ms: (0)NoLimit, exit_type: (0)exit script, (1)exit game
def SetGameTimeLimit(ms, exit_type=1, stackable=True):
    global _time_limit
    
    if ms > 0:
        if not stackable or not _time_limit:
            _time_limit = (ms, GetTickCount(), exit_type, [])
        else:
            _time_limit[3].append( (_time_limit[0], _time_limit[1], _time_limit[2]) )
            _time_limit = (ms, GetTickCount(), exit_type, _time_limit[3])
    
    else:
        if stackable and _time_limit and _time_limit[3]:
            tl = _time_limit[3].pop()
            _time_limit = (tl[0], tl[1], tl[2], _time_limit[3])
        else:
            _time_limit = None

def __get_next_tmr_wdms():
    if not _timer or not _timer[2] or not _callbacks or _callbacks[1] >= 4: return None
    
    wdms = _timer[2][0][0] - GetTickInterval(_timer[0])
    if wdms > 0:
        return wdms
    else:
        return 0

def timer_pool():
    if not _callbacks or _callbacks[1] >= 4 or not _timer[2]: return
    
    _callbacks[1] += 4
    try:
        fz = _timer[2]
        while fz:
            f = fz[0]
            if GetTickInterval(_timer[0]) <= f[0]: break
            
            _heapq.heappop(fz)
            f[5] = False
            
            try:
                f[2](f[3])
            except Exception, e:
                EM_Utils.DLog("Timer Callback Error:", 3)
                EM_Utils.DLog(e, 3)
                EM_Utils.PrintErr()
                
            if _timer[1].has_key(f[4]):
                f[0] = GetTickInterval(_timer[0]) + f[1]
                _heapq.heappush(fz, f)
                f[5] = True
                
    finally:
        _callbacks[1] -= 4
        f = None
        fz = None
    
def dispatch_apc():
    if not _callbacks or _callbacks[1] >= 1: return

    _callbacks[1] += 1
    try:
        while _callbacks[5 + 2]:
            fz = _callbacks[5 + 2] #apc = 5
            _callbacks[5 + 2] = []
            
            for f in fz:
                try:
                    f[1](f[0])
                except Exception, e:
                    EM_Utils.DLog("Dispatch APC Callback Error:", 3)
                    EM_Utils.DLog(e, 3)
                    EM_Utils.PrintErr()
                
    finally:
        _callbacks[1] -= 1
        f = None
        fz = None


def check_life():
    if not _callbacks or _callbacks[1] >= 16: return
    
    _callbacks[1] += 16
    f = get_func(_callbacks[0], "OnLifeCheck")
    if f:
        fz = _callbacks[EVT_ONLIFECHECK + 2] + [(None,f)]
    else:
        fz = _callbacks[EVT_ONLIFECHECK + 2]
        
    try:
        for f in fz:
            try:
                f[1]()
            except Exception, e:
                EM_Utils.DLog("OnLifeCheck() Callback Error:", 3)
                EM_Utils.DLog(e, 3)
                EM_Utils.PrintErr()
                
            
    finally:
        _callbacks[1] -= 16
        f = None
        fz = None
    
    
def dispatch_packets():
    global _packets
    
    if not _callbacks or _callbacks[1] >= 8: return
    
    _callbacks[1] += 8
    f = get_func(_callbacks[0], "OnPacketUpdate")
    if f:
        fz = _callbacks[EVT_ONPACKETUPDATE + 2] + [(None,f)]
    else:
        fz = _callbacks[EVT_ONPACKETUPDATE + 2]
        
    try:
        while _packets:
            tmp = _packets
            _packets = []
            
            for et in tmp:
                for f in fz:
                    try:
                        f[1](et[0], et[1], et[2])
                    except Exception, e:
                        EM_Utils.DLog("OnPacketUpdate() Callback Error:", 3)
                        EM_Utils.DLog(e, 3)
                        EM_Utils.PrintErr()
                        
    finally:
        _callbacks[1] -= 8
        f = None
        fz = None


def on_wake():
    if not _callbacks or _callbacks[1] >= 2: return
    
    _callbacks[1] += 2
    f = get_func(_callbacks[0], "OnWake")
    if f:
        fz = _callbacks[EVT_ONWAKE + 2] + [(None,f)]
    else:
        fz = _callbacks[EVT_ONWAKE + 2]
        
    try:
        for f in fz:
            try:
                f[1]()
            except Exception, e:
                EM_Utils.DLog("OnWake() Callback Error:", 3)
                EM_Utils.DLog(e, 3)
                EM_Utils.PrintErr()
            
    finally:
        _callbacks[1] -= 2
        f = None
        fz = None
        

def get_func(s, nz):
    if s and s.has_key(nz):
        f = s[nz]
        if callable(f):
            return f
        
    return None

#######

def _wait4(ms, func, val, type):
    wid = add_to_wlist(func, val, type)
    Sleep(ms, val)
    
    if not val[0]:
        del_from_wlist(wid, type)
        
    return val[0]
    

def chk4_npc_int(val, buf):
    pid = ord(buf[0])
    if pid == 0x2F:
        type, id = _struct.unpack_from("<II", buf, 1)
        if id == val[1]:
            val[0] = True
            return True
        

def wait4_npc_int(ms, npc_id):
    val = [False, npc_id]
    return _wait4(ms, chk4_npc_int, val, 0)


def wait4_lvlno(ms, old_lvl):
    done = False
    while True:
        ms = Sleep(ms, (True,))
        if GetRoom()[4] != old_lvl:
            done = True
            break
        
        if ms <= 0: break
        
    return done


def wait4_skill(ms, skill_uid, side):
    done = False
    idx = 2 + (side & 1)
    while True:
        ms = Sleep(ms, (True,))
        if _cp[idx] == skill_uid:
            done = True
            break
        
        if ms <= 0: break
        
    return done


def chk4_reassignment(val, buf):
    pid = ord(buf[0])
    if pid == 0x15:
        type, id, x, y, m = _DCP.unpack(buf)[1:]
        if type == 0 and id == _cp[0]:
            if not val[5] or x != val[1] or y != val[2]:
                val[0] = True
                return True

def wait4_reassignment(ms, x, y, s=1):
    pos = GetPos()
    val = [False, pos[0], pos[1], x, y, s]
    
    return _wait4(ms, chk4_reassignment, val, 1)


def chk4_weapon_chg(val, buf):
    pid = ord(buf[0])
    if pid == 0x97:
        val[0] = True
        return True
        
def wait4_weapon_chg(ms, old_val = -1):
    val = [False, old_val]
    
    return _wait4(ms, chk4_weapon_chg, val, 1)
 

def chk4_partystate_chg(val, buf):
    if val[1] not in _players.keys():
        val[0] = True
        return True
    
    pid = ord(buf[0])
    if pid == 0x8B:
        player_id, state = _DCP.unpack(buf)[1:]
        if player_id == val[1]:
            val[0] = True
            return True

def wait4_partystate_chg(ms, player_id):
    val = [False, player_id]
    return _wait4(ms, chk4_partystate_chg, val, 1)
    
def chk4_ts_done(val, buf):
    pid = ord(buf[0])
    if pid == 0x2A:
        val[0] = True
        return True

def wait4_ts_done(ms):
    val = [False]
    return _wait4(ms, chk4_ts_done, val, 1)

def chk4_wp_up(val, buf):
    pid = ord(buf[0])
    if pid == 0x63:
        val[0] = True
        return True
    
def wait4_wp_up(ms):
    val = [False]
    return _wait4(ms, chk4_wp_up, val, 1)

def chk4_item_chg(val, buf):
    pid = ord(buf[0])
    if pid == 0x9C or pid == 0x9D:
        state, id = _struct.unpack_from("<BxxI", buf, 1)
        if id == val[1] and state == val[2]:
            val[0] = True
            return True

def wait4_item_chg(ms, id, state):
    val = [False, id, state]
    return _wait4(ms, chk4_item_chg, val, 1)

def chk4_sd_up(val, buf):
    pid = ord(buf[0])
    if pid == 0x2C:
        type, id, sd_id = _DCP.unpack(buf)[1:]
        if type == 0 and _cp[0] == id and sd_id == val[1]:
            val[0] = True
            return True

def wait4_sd_up(ms, sd_id):
    val = [False, sd_id]
    return _wait4(ms, chk4_sd_up, val, 1)

def wait4_cursor_clear(ms):
    done = False
    while True:
        ms = Sleep(ms, (True,))
        if not GetInvInfo()[2]:
            done = True
            break
        
        if ms <= 0: break
        
    return done

