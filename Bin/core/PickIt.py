from _D2EM import GetExecPath, ItemCodeToClsId, GetItemInfo, GetItemStat, GetTickCount
import EM_Utils

_items = {}
CONFIG_FILE_DIR = GetExecPath()

def Init():
    if _items: return
    
    fp = open(CONFIG_FILE_DIR + "\\pickit.ini", "r")
    if not fp:
        print "No PickIt File..."
        return
    
    cur = None
    for l in fp:
        idx = l.find(';')
        if idx != -1:
            l = l[:idx]
        
        if not l.strip(): continue
        
        if l[0] == '[':
            idx = l.find(']')
            if idx == -1:
                print "Error[sect]", l
                break
            
            code = l[1:idx].strip()
            if len(code) != 3:
                print "Error[size]", c
                break
            
            cls_id = ItemCodeToClsId(code)
            if not cls_id:
                print "Error[code]", code
                break
            
            cur_code = (code, cls_id)
            if not _items.has_key(cls_id):
                _items[cls_id] = (code, [])
        
        else:
            s = l.split(':')
            if len(s) != 4:
                print "Error[fmt]", l
                break
            
            name = s[1].strip()
            expr = s[3].strip()
            if not expr: continue
            
            try:
                c = compile(expr, "PickIt", "eval")
            except Exception, e:
                print expr, e
                c = None
                
            if not c: break
            
            if not cur_code:
                print "Error[nosect]", name, expr
                break
            
            try:
                prio = int( s[0].strip() )
            except Exception, e:
                prio = 0
                
            try:
                iden = int( s[2].strip() )
            except Exception, e:
                iden = 0
                
            _items[ cur_code[1] ][1].append( (prio, name, c, iden) )
            #_items{cls_id:(code,[(prio, name, c, iden),]),}
    
    fp.close()
    
def Reset():
    pass


class _ItemProp:
    def __init__(self, tid, ptr):
        self.tid = tid
        self.ptr = ptr
        self.mod = None
        
    def get_stat(self, idx, mul=0):
        sts = GetItemStat(self.ptr, idx, mul) #(idx >> 16, stat)
        
        if not mul:
            if not sts: return 0
            #print sts[1]
            return sts[1]
            
        else:
            ret = []
            if not sts: return ret
            for i in sts:
                ret.append( i[1] )
            
            return ret
    
    def get_mod(self, idx, mul=0):
        if not self.mod:
            info = GetItemInfo(self.tid, 1)
            if info:
                self.mod = info[0]
                
        if self.mod:
            sts = GetItemStat(self.mod, idx, mul) #(idx >> 16, stat)
        else:
            sts = None
        
        if not mul:
            if not sts: return 0
            #print sts[1]
            return sts[1]
            
        else:
            ret = []
            if not sts: return ret
            for i in sts:
                ret.append( i[1] )
            
            return ret
        
    def get_skill(self, skill_id):
        sts = GetItemStat(self.ptr, 0x6B, 1)
        if sts:
            for i in sts:
                if i[0] & 0xFFFF == skill_id:
                    return i[1]
        
        return -1
    
    def get_skill_tree(self, st_id):
        sts = GetItemStat(self.ptr, 0xbc, 1)
        if sts:
            for i in sts:
                if i[0] & 0xFFFF == st_id:
                    return i[1]
        
        return -1
        
        
def IsEthereal(flag):
    if flag & 0x400000: return 1
    
    else: return 0
    
def IsIdentified(flag):
    if flag & 0x10: return 1
    
    else: return 0

def IsInList(id):
    info = GetItemInfo(id) #(c_ptr, cls_id, flg, qlt, lvl)
    #print info[3], " ", IsIdentified(info[2])

    if not info: return False
    
    item = _items.get(info[1]) #(code,[(prio, name, c, iden),]) code = hp5 - [hp5]
    if not item: return False
    
    e = None
    tp = _ItemProp(id, info[0])
    
    d = {'s':       tp.get_stat,
         'm':       tp.get_mod,
         'flg':     info[2],
         'qlt':     info[3],
         'lvl':     info[4],
         'eth':     IsEthereal(info[2]),
         'ide':     IsIdentified(info[2]),
         'stat':    tp.get_stat,
         'mod':     tp.get_mod,
         'skill':   tp.get_skill,
         'sktr':        tp.get_skill_tree
         }
    
    try:
        for i in item[1]: #item[1] = [(prio, name, c, iden),]; i = (prio, name, c, iden)
            try:
                if eval(i[2], d):
                    e = [i[0], i[1], info[3], id, info[1], "", "", "", i[3], info[2]]
                    break
                
            except Exception, err:
                EM_Utils.DLog("ItemExpr::Error::%s:%s:%s" % (item[0], i[1], err), 1)
            
    finally:
        d.clear()
        d = None
        tp = None
    
    # - [priority, itemName, quality, id, clsId, "", "", "", iden]
    return e

