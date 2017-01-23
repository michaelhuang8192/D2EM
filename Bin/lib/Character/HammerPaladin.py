from D2EM import *
from ItemCheck import *
from D2EMExtended import *
import Data

HPOS = (
    (0,0), (0,1), (-1,1), (-2,1), (-2,0), (-2,-1),
    (-2,-2), (-2,-3), (-1,-3), (0,-4), (1,-4), (2,-3),
    (3,-3), (4,-2), (4,-1), (5,0), (5,1), (5,2),
    (4,3), (4,4), (3,5), (2,5), (1,6), (0,6),
    (-1,6), (-2,6), (-3,5), (-4,5), (-5,5), (-5,4),
    (-6,3), (-7,2), (-7,1), (-7,0), (-7,-1), (-7,-2),
    (-7,-3), (-6,-4), (-5,-5), (-4,-6), (-3,-7), (-2,-8),
    (0,-8), (2,-8), (3,-8), (4,-7), (5,-7), (6,-6),
    (7,-5), (8,-4), (8,-3), (9,-2), (9,0), (9,2),
    (9,4), (8,5), (7,7), (6,8), (4,9), (2,10),
    (0,11), (-2,10), (-4,10), (-6,9), (-8,8), (-9,6),
    (-10,5), (-11,4), (-11,2), (-12,0), (-12,-2), (-11,-4),
    (-11,-5), (-10,-7), (-9,-9), (-7,-10), (-6,-11), (-5,-12),
    (-3,-12), (-2,-12), (0,-13), (2,-13), (3,-13)
    )

APOS = (
    (-1, 1), (-1, 0), (-1, -1), (0, 1), (0, 0), (0, -1), (1, 1), (1, 0), (1, -1)
)

def get_ak_pos(tx, ty, px, py, ak_sidx=None):
    l = len(HPOS)
    if ak_sidx and ak_sidx >= 2:
        a = ak_sidx
    else:
        a = 2
        
    while a < l:
        x = tx - HPOS[a][0]
        y = ty - HPOS[a][1]
        e = a
        a += 1
        
        if px != x or py != y:
            pt = GetPointInMap(x, y)
            if pt == None or (pt & 0x1C01): continue
            
            pt = GetPointInMap(x - 1, y)
            if pt == None or (pt & 0x1C01): continue
            
            pt = GetPointInMap(x + 1, y)
            if pt == None or (pt & 0x1C01): continue
            
            pt = GetPointInMap(x, y - 1)
            if pt == None or (pt & 0x1C01): continue
            
            pt = GetPointInMap(x, y + 1)
            if pt == None or (pt & 0x1C01): continue
            
        ak = True
        b = 2
        while b < e:
            x0 = x + HPOS[b][0]
            y0 = y + HPOS[b][1]
            pt = GetPointInMap(x0, y0)
            if pt == None or (pt & 0x06):
                ak = False
                break
            
            b += 1
            
        if ak: return (x, y)
    
    return None

def is_overlapping(s_x, s_y, d_x, d_y, radius):
    dx = d_x - s_x
    if dx == 0:
        x0 = x1 = float(s_x)
        D = radius ** 2 - x0 ** 2
        if D < 0:
            return False
        
        y0 = (D ** 0.5)
        y1 = -y0
        
        s_r = s_y
        d_r = d_y
        r_0 = y0
        r_1 = y1
        
    else:
        s = (d_y - s_y) / float(dx)
        m = s_y - s * s_x
        
        a = (1 + s ** 2)
        b = (2 * m * s)
        c = (m ** 2 - radius ** 2)
        
        D = b ** 2 - 4 * a * c
        
        if D < 0:
            return False
                
        x0 = (-b + D ** 0.5) / float(2 * a)
        y0 = s * x0 + m
        
        x1 = (-b - D ** 0.5) / float(2 * a)
        y1 = s * x1 + m
    
        s_r = s_x
        d_r = d_x
        r_0 = x0
        r_1 = x1
    
    if s_r <= d_r:
        min_a = s_r
        max_a = d_r
    else:
        min_a = d_r
        max_a = s_r
            
    if r_0 <= r_1:
        min_b = r_0
        max_b = r_1
    else:
        min_b = r_1
        max_b = r_0
            
    if min_a <= max_b and min_b <= max_a:
        return True
    
    else:
        return False

def get_npc_count(center, radius):
    units = GetNearbyUnits(AK_TYPE)
    if not units: return 0

    cnt = 0
    for u in units:
        uid = u[1]
        if not GetLife(AK_TYPE, uid) or not IsAttackable(AK_TYPE, uid): continue
        
        pos = GetPos(AK_TYPE, uid)
        if not pos: continue
        
        if GetDistance(center[0], center[1], pos[0], pos[1]) <= radius:
            cnt += 1
            
    return cnt

def _get_dist(unit):
    return unit[5]
    
def _mcast(my_pos):
    if get_npc_count(my_pos, 8) >= 5:
        ret = True
        for i in range(2, 24):
            x = my_pos[0] + HPOS[i][0]
            y = my_pos[1] + HPOS[i][1]
            pt = GetPointInMap(x, y)
            if pt == None or (pt & 0x06):
                ret = False
                break
                
        return ret
            
    return False

def _is_in_range(mpos, rng):
    xr = rng[0]
    yr = rng[1]
    
    if xr:
        xres = False
        if mpos[0] >= xr[0] and mpos[0] <= xr[1]:
            xres = True
    else:
        xres = True
        
    if yr:
        yres = False
        if mpos[1] >= yr[0] and mpos[1] <= yr[1]:
            yres = True
    else:
        yres = True

    return xres and yres

def _is_target(u, cls_ids, names, in_rngs, out_rngs):
    if cls_ids or names or in_rngs:
        res = False
        
        if not res and cls_ids:
            if u[0] in cls_ids: res = True
            
        if not res and names:
            nz = GetUnitName(AK_TYPE, u[1])
            if nz and (nz in names): res = True
            
        if not res and in_rngs:
            kpos = u[4]
            for p in in_rngs:
                if _is_in_range(kpos, p):
                    res = True
                    break
        
    else:
        res = True
        
    if res and out_rngs:
        kpos = u[4]
        for p in out_rngs:
            if _is_in_range(kpos, p):
                res = False
                break
            
    return res
    
def _is_target_by_rng(u, in_rngs, out_rngs):
    if in_rngs:
        res = False
        kpos = u[4]
        for p in in_rngs:
            if _is_in_range(kpos, p):
                res = True
                break
    else:
        res = True
            
    if res and out_rngs:
        kpos = u[4]
        for p in out_rngs:
            if _is_in_range(kpos, p):
                res = False
                break
            
    return res

AK_TYPE = 1
class HammerPaladin:
    def __init__(self):
        self.__evt_id = None
        self.__ak_info = None
        self.target = []
        self.tar_name = []
        self.xRange = []
        self.yRange = []
        self.xRange_skip = []
        self.yRange_skip = []
        self.tick = 0
        self.apos_idx = 0
        self.units_d = {}
    
    def Precast(self):
        myPos = GetPos()
        if HasCta() and GetSkillBySide(0)[0]: SwitchWeapon()
        
        SelectSkill(Data.BattleCommand, 1)
        CastSkill(myPos[0], myPos[1], 1)
        Sleep(600)
        SelectSkill(Data.BattleOrders, 1)
        CastSkill(myPos[0], myPos[1], 1)
        Sleep(600)
        
        if not GetSkillBySide(0)[0]: SwitchWeapon()
        
        SelectSkill(Data.HolyShield, 1)
        CastSkill(myPos[0], myPos[1], 1)
        Sleep(500)
    
    def InTownPrecast(self): return
        
    def __del__(self):
        pass
        #print "Del..."
        #DetachEvent(EVT_ONDRAW, self.fid)
    
    def ak_lcd(self):
        if not self.__ak_info: return
        
        target_id = self.__ak_info[0]
        target = GetUnitState(AK_TYPE, target_id)
        if target:
            tstr = "Target[\377c1%s\377c8]: cls(\377c2%x\377c8), id(\377c2%x\377c8), state(\377c2%x\377c8)" % (
                str(GetUnitName(AK_TYPE, target_id)), target[0], target_id, target[1] )
        else:
            tstr = "Target[\377c9None\377c8]"
            
        tstr = "Total Monsters: \377c;%d\377c8" % (self.__ak_info[1]) + "\n" + tstr
        
        w, h = GetWindowSize()[:2]
        of_id = D2SelectFont(0x0d)
        D2DrawText(tstr, 0, h - 200, 8, 0)
        D2SelectFont(of_id)
        
    def Attack(self, clear_black_list=True, target_callback=None, pvd=None):
        if self.__evt_id == None: self.__evt_id = AttachEvent(EVT_ONDRAW, self.ak_lcd)
        
        if clear_black_list: self.units_d.clear()
        units_d = self.units_d
        units = {}
        
        ak_sidx = None
        if pvd: ak_sidx = pvd[0]
        
        _in_rngs = _out_rngs = None
        if self.xRange or self.yRange:
            _in_rngs = ((self.xRange, self.yRange), )
        if self.xRange_skip or self.yRange_skip:
            _out_rngs = ((self.xRange_skip, self.yRange_skip), )
        
        ru = False
        while True:
            if ru:
                ru = False
            else:
                ItemCheck()
            
            if target_callback:
                all_units = target_callback[0](AK_TYPE)
            else:
                all_units = GetNearbyUnits(AK_TYPE)
                
            if not all_units: break
            
            my_pos = GetPos()
            my_rm = GetRoom()
            
            units.clear()
            for u in all_units:
                uid = u[1]
                if not my_rm or my_rm[4] != u[4] or IsTownLevel(u[4]): continue
                
                life = GetLife(AK_TYPE, uid)
                if not life or not IsAttackable(AK_TYPE, uid): continue
                
                tar_pos = GetPos(AK_TYPE, uid)
                if not tar_pos: continue
                
                ud = units_d.get(uid, None)
                if not ud:
                    dist = GetDistance(my_pos[0], my_pos[1], tar_pos[0], tar_pos[1])
                elif ud[0] > 0:
                    dist = 0xFFFF - ud[0]
                else:
                    continue
                
                tu = [u[0], u[1], u[2], u[3], tar_pos, dist]
                if not _is_target(tu, self.target, self.tar_name, _in_rngs, _out_rngs): continue
                
                if target_callback: target_callback[2](AK_TYPE, tu)
                
                units[uid] = tu
                
            if not units: break
            
            while units:
                target = min(units.itervalues(), key=_get_dist)
                tar_id = target[1]
                tar_pos = target[4]
                qt_tel = 4
                qt_max_tel = 5
                qt_max_cas = 15
                qt_rcc = 0
                qt_rcf = 0
                last_tar_pos = None
                
                while True: #update
                    while True: #ak
                        if not qt_rcc:
                            ir = False
                            dst_pos = tar_pos
                            pth = GetUnitPath(AK_TYPE, tar_id)
                            if pth and pth[4] and (target[2] == 2 or target[2] == 15):
                                dlt = pth[4]
                                dd = (dlt[0] ** 2 + dlt[1] ** 2) ** 0.5 / 40 * 650
                                
                                dx = ((pth[2] << 16) | 0x8000) - pth[0]
                                dy = ((pth[3] << 16) | 0x8000) - pth[1]
                                dr = (dx ** 2 + dy ** 2) ** 0.5
                                
                                if dr:
                                    x = dx / dr * dd + pth[0]
                                    y = dy / dr * dd + pth[1]
                                    
                                    src_pos = ( pth[0] >> 16, pth[1] >> 16 )
                                    dst_pos = ( int(x) >> 16, int(y) >> 16 )
                                    
                                    src_x = src_pos[0] - my_pos[0]
                                    src_y = src_pos[1] - my_pos[1]
                                    dst_x = dst_pos[0] - my_pos[0]
                                    dst_y = dst_pos[1] - my_pos[1]
                                    if is_overlapping(src_x, src_y, dst_x, dst_y, 3):
                                        #print "*****IN*****"
                                        ir = True
                                    elif last_tar_pos:
                                        src_x = src_pos[0] - last_tar_pos[0]
                                        src_y = src_pos[1] - last_tar_pos[1]
                                        dst_x = dst_pos[0] - last_tar_pos[0]
                                        dst_y = dst_pos[1] - last_tar_pos[1]
                                        if is_overlapping(src_x, src_y, dst_x, dst_y, 3):
                                            #print "*****ININ*****"
                                            ir = True
                            
                            ak_pos = get_ak_pos(dst_pos[0], dst_pos[1], my_pos[0], my_pos[1], ak_sidx)
                            if not ak_pos:
                                DLog("No AK Pos <%x,%x>" % (dst_pos[0], dst_pos[1]), 1)
                                #print "%s, %d" % ( GetUnit(1, AK_TYPE, tar_id),  target[0])
                                target = None
                                break
                            
                            self.__ak_info = (tar_id, len(units))
                            ru = True
                            
                            oak_pos = get_ak_pos(tar_pos[0], tar_pos[1], my_pos[0], my_pos[1], ak_sidx)
                            if ak_pos != my_pos and oak_pos != my_pos:
                                if last_tar_pos:
                                    dist = GetDistance(last_tar_pos[0], last_tar_pos[1], tar_pos[0], tar_pos[1])
                                else:
                                    dist = 0xFFFF
                                
                                if (dst_pos == tar_pos or not ir) and (qt_tel >= 2 or dist > 3 and get_npc_count(my_pos, 5) < 3):
                                    if qt_max_tel > 0:
                                        qt_tel -= 2
                                        qt_max_tel -= 1
                                        
                                        #print "Teleport %s, %s- %d" % (ak_pos, my_pos, dst_pos == tar_pos)
                                        if Teleport(ak_pos[0], ak_pos[1], 3, 1, 600):
                                            last_tar_pos = dst_pos
                                        elif last_tar_pos:
                                            npos = GetPos()
                                            if npos[0] != my_pos[0] or npos[1] != my_pos[1]:
                                                last_tar_pos = None
                                            
                                    else:
                                        target = None
                                        
                                    break
                        
                        if GetTickInterval(self.tick) < 400:
                            Sleep(10)
                            break
                        
                        if qt_rcc:
                            qt_rcc -= 1
                        else:
                            qt_max_cas -= 1
                            
                            if not qt_rcf and _mcast(my_pos):
                                #print "...............Recast..............."
                                qt_rcf = 1
                                qt_rcc = 2
                        
                        if qt_tel < 0:
                            qt_tel = 1
                        elif qt_tel < 4:
                            qt_tel += 1
                        
                        if GetSkillUIdBySide(1)[0] != 113:
                            SelectSkill_NB(113, 1)
                            #SelectSkill(113, 1)
                        
                        cs_x = my_pos[0] + APOS[ self.apos_idx ][0]
                        cs_y = my_pos[1] + APOS[ self.apos_idx ][1]
                        self.apos_idx %= len(APOS)
                        CastSkill(cs_x, cs_y, 0)
                        
                        #print ">>>>>>AK[%d] %d" % (GetTickInterval(self.tick), tar_id)
                        self.tick = GetTickCount()
                        
                        Sleep(260)
                        if GetSkillUIdBySide(1)[0] != 113:
                            #print "Error................................."
                            SelectSkill(113, 1)
                        
                        break
                    
                    if ru: ItemCheck(tar_id)
                    
                    if qt_rcc: #recast
                        my_pos = GetPos()
                        
                    elif target and qt_max_cas > 0: #update npc
                        my_rm = GetRoom()
                        tar_rm = GetRoom(AK_TYPE, tar_id)
                        if not my_rm or not tar_rm or my_rm[4] != tar_rm[4] or IsTownLevel(tar_rm[4]): break
                        
                        life = GetLife(AK_TYPE, tar_id)
                        if not life or not IsAttackable(AK_TYPE, tar_id): break
                        
                        tar0 = GetUnit(0, AK_TYPE, tar_id)
                        target[2] = tar0[1]
                        target[3] = tar0[2]                        
                        target[4] = tar_pos = GetPos(AK_TYPE, tar_id)

                        if not tar_pos: break
                        if target_callback and not target_callback[1](AK_TYPE, target): break
                        if not _is_target_by_rng(target, _in_rngs, _out_rngs): break
                        
                        my_pos = GetPos()
                    
                    else:
                        #print "------"
                        ud = units_d.get(tar_id, None)
                        if ud:
                            units_d[tar_id] = (ud[0] - 1, 0)
                        else:
                            units_d[tar_id] = (1, 0)
                            
                        break
                    
                if ru:
                    break
                else:
                    del units[tar_id]
            
            Sleep(0)
        
        DetachEvent(EVT_ONDRAW, self.__evt_id)
        self.__evt_id = None
        