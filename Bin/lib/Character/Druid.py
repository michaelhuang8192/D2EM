from D2EM import *
from ItemCheck import *
from D2EMExtended import *
import Data


def GetTotalDist(akPos):
    return akPos[1][2]

def GetAkPos(tar_pos, units, skip_range=([],[]), radius=40, in_range=([],[])):
    akPos = {}
    myPos = GetPos()
    
    staticAkPos = CirclePoints(tar_pos, radius)
    size = len(staticAkPos)
    
    for i in range(size):
        pt = GetPointInMap(staticAkPos[i][0], staticAkPos[i][1])
        if pt == None or pt & 0x1C01:
            continue
        
        akPos[i] = [staticAkPos[i][0], staticAkPos[i][1]]
    
    if len(skip_range[0]) and len(skip_range[1]):
        if len(akPos):
            posList = akPos.keys()
            
            for i in posList:
                if skip_range[0][0] <= akPos[i][0] <= skip_range[0][1]:
                    if skip_range[1][0] <= akPos[i][1] <= skip_range[1][1]:
                        akPos.pop(i)
    
    if len(in_range[0]) and len(in_range[1]):
        if len(akPos):
            posList = akPos.keys()
        
            for i in posList:
                if in_range[0][0] <= akPos[i][0] <= in_range[0][1]:
                    if in_range[1][0] <= akPos[i][1] <= in_range[1][1]:
                        continue
                    
                akPos.pop(i)
            
    if len(akPos):
        posList = akPos.keys()

        for i in posList:
            totalDist = 0
            nearMonCount = 0
            
            if not TestPath(tar_pos, akPos[i]):
                del akPos[i]
                continue
            
            for u in units:
                dist = GetDistance(akPos[i][0], akPos[i][1], units[u][4][0], units[u][4][1])
                if dist <= 8: nearMonCount += 1
                
                if nearMonCount > 3:
                    totalDist = 10000
                    break
                
                if dist <= 20: totalDist += dist
            
            akPos[i].append(int(totalDist))
        
        if not len(akPos): return None
        
        destAkPos = min(akPos.iteritems(), key=GetTotalDist)
        jPos = destAkPos[1]
            
        return (jPos[0], jPos[1])
                  
    DLog("No pos ak pos!", 2)
    return None

def TestPath(tar_pos, trapPos):
    xDist = trapPos[0] - tar_pos[0]
    yDist = trapPos[1] - tar_pos[1]
    pathCon = True
    
    if xDist == 0:
        if yDist > 0:
            deltaY = 1
            r = yDist
        
        elif yDist < 0:
            deltaY = -1
            r = yDist * -1
        
        else: return True
        
        for c in range(1, r):
            dy = c * deltaY
            ny = int(round(tar_pos[1] + dy))
            
            pt = GetPointInMap(tar_pos[0], ny)
            if pt == None or pt & 0xc2f:
                pathCon = False
                break
    
    else:
        slope = (float(yDist) / xDist)
        
        if xDist > 0:
            deltaX = 1
            r = xDist
        
        elif xDist < 0:
            deltaX = -1
            r = xDist * -1
        
        for c in range(1, r):
            dx = deltaX * c
            dy = slope * c * deltaX
            nx = int(tar_pos[0] + dx)
            ny = int(round(tar_pos[1] + dy))
            
            pt = GetPointInMap(nx, ny)
            if pt == None or pt & 0xc2f:
                pathCon = False
                break
            
    return pathCon

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

def IsSafe():
    myPos = GetPos()
    if get_npc_count((myPos[0], myPos[1]), 8) >= 4:
        return False
    
    return True
    
def _get_dist(unit):
    return unit[5]

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

def GetBestTarget(units):
    immunes = {}
    regulars = {}
    
    for u in units:
        if units[u][6] >= 100:
            immunes[u] = units[u]
        
        else: regulars[u] = units[u]
        
    if regulars: return min(regulars.itervalues(), key=_get_dist)
        
    return min(immunes.itervalues(), key=_get_dist)

def CountPets():
    pets = GetPets()
    pet_count = {419:0, 424:0, 420:0, 421:0, 428:0, 426:0, 427:0} #raven, oak sage, spirit wolf, dire wolf, grizzly, carrion vine, solar creeper
        
    for p in pets:
        pet_cid = p[0]
        if pet_count.has_key(pet_cid): pet_count[pet_cid] += 1
        
    return pet_count

def GetAkRadius(tar_cid, tar_pos):
    ak_radius = 8
    
    if tar_cid in (Data.SoulKiller1, Data.SoulKiller2, Data.SoulKiller3, Data.SoulKiller4): pass
    
    elif tar_cid == 544: ak_radius = 2
                        
    elif get_npc_count(tar_pos, 10) <= 3:
        ak_radius = 5
        #DLog("cnt <= 3, set ak_radius to 5")
    
    return ak_radius
    
AK_TYPE = 1
class Druid:
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
        
        self.hur_tick = 0
        self.ca_tick = 0
        self.oak_tick = 0
        self.raven_tick = 0
        self.grizzly_tick = 0
        self.carrionvine_tick = 0
        
        self.qt_raven_cast = 5
    
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
        
        pet_count = CountPets()
        for i in range(5-pet_count[419]):
            if GetSkillUIdBySide(1)[0] != Data.Raven: SelectSkill(Data.Raven, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
            
        if not pet_count[428]:
            if GetSkillUIdBySide(1)[0] != Data.SummonGrizzly: SelectSkill(Data.SummonGrizzly, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
        
        if not pet_count[427]:
            if GetSkillUIdBySide(1)[0] != Data.Vines: SelectSkill(Data.Vines, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
            
        if not HasAura(0, GetCurrentPlayerId(), 149):
            if GetSkillUIdBySide(1)[0] != Data.OakSage: SelectSkill(Data.OakSage, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
            
        if not HasAura(0, GetCurrentPlayerId(), 151):
            if GetSkillUIdBySide(1)[0] != Data.CycloneArmor: SelectSkill(Data.CycloneArmor, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
            
        if not HasAura(0, GetCurrentPlayerId(), 144):
            if GetSkillUIdBySide(1)[0] != Data.Hurricane: SelectSkill(Data.Hurricane, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
    
    def InTownPrecast(self):
        myPos = GetPos()
        
        pet_count = CountPets()
        for i in range(5-pet_count[419]):
            if GetSkillUIdBySide(1)[0] != Data.Raven: SelectSkill(Data.Raven, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
            
        if not pet_count[428]:
            if GetSkillUIdBySide(1)[0] != Data.SummonGrizzly: SelectSkill(Data.SummonGrizzly, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
        
        if not HasAura(0, GetCurrentPlayerId(), 151):
            if GetSkillUIdBySide(1)[0] != Data.CycloneArmor: SelectSkill(Data.CycloneArmor, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
            
        if not pet_count[424]:
            if GetSkillUIdBySide(1)[0] != Data.OakSage: SelectSkill(Data.OakSage, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
            
        if not pet_count[427]:
            if GetSkillUIdBySide(1)[0] != Data.Vines: SelectSkill(Data.Vines, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(500)
        
    def __del__(self):
        pass
    
    def GetNearbyMonsters(self):
        units = {}
        
        all_units = GetNearbyUnits(AK_TYPE)
        if not all_units: return units
            
        my_pos = GetPos()
        my_rm = GetRoom()
            
        for u in all_units:
            uid = u[1]
                    
            if not my_rm or my_rm[4] != u[4] or IsTownLevel(u[4]): continue
                
            life = GetLife(AK_TYPE, uid)
            if not life or not IsAttackable(AK_TYPE, uid): continue
                
            tar_pos = GetPos(AK_TYPE, uid)
                
            dist = GetDistance(my_pos[0], my_pos[1], tar_pos[0], tar_pos[1])
                
            units[uid] = [u[0], u[1], u[2], u[3], tar_pos, dist]
        
        return units
    
    def ak_lcd(self):
        if not self.__ak_info: return
        
        target_id = self.__ak_info[0]
        target = GetUnitState(AK_TYPE, target_id)
        if target:
            tstr = "Target[\377c1%s\377c8]: cls(\377c2%x\377c8), id(\377c2%x\377c8), state(\377c2%x\377c8), dr(\377c2%d\377c8)" % (
                str(GetUnitName(AK_TYPE, target_id)), target[0], target_id, target[1], GetNpcResist(36, target_id) )
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
                
                tu = [u[0], u[1], u[2], u[3], tar_pos, dist, GetNpcResist(36, u[1])]
                if not _is_target(tu, self.target, self.tar_name, _in_rngs, _out_rngs): continue
                
                if target_callback: target_callback[2](AK_TYPE, tu)
                
                units[uid] = tu
                
            if not units: break
            
            while units:
                target = GetBestTarget(units)
                tar_id = target[1]
                tar_pos = target[4]
                tar_lr = target[6]
                
                qt_tel = 4
                qt_max_tel = 5
                qt_max_cas = 15
                qt_rcc = 0
                qt_rcf = 0
                last_tar_pos = None
                safe = False
                re_tele = False
                
                while True: #update
                    while True: #ak
                        if not HasAura(0, GetCurrentPlayerId(), 144) and (GetTickCount() - self.hur_tick) >= 6000:
                            if GetSkillUIdBySide(1)[0] != Data.Hurricane: SelectSkill(Data.Hurricane, 1)
                            CastSkill(my_pos[0], my_pos[1], 1)
                            DLog("Cast Hurricane!")
                            self.hur_tick = GetTickCount()
                            Sleep(350)
                            break
                            
                        if not HasAura(0, GetCurrentPlayerId(), 151) and (GetTickCount() - self.ca_tick) >= 5000:
                            if GetSkillUIdBySide(1)[0] != Data.CycloneArmor: SelectSkill(Data.CycloneArmor, 1)
                            CastSkill(my_pos[0], my_pos[1], 1)
                            DLog("Cast CycloneArmor!")
                            self.ca_tick = GetTickCount()
                            Sleep(350)
                            break
                        
                        if not HasAura(0, GetCurrentPlayerId(), 149) and (GetTickCount() - self.oak_tick) >= 5000:
                            if GetSkillUIdBySide(1)[0] != Data.OakSage: SelectSkill(Data.OakSage, 1)
                            CastSkill(my_pos[0], my_pos[1], 1)
                            DLog("Summon OakSage!")
                            self.oak_tick = GetTickCount()
                            Sleep(350)
                            break
                        
                        pet_count = CountPets()
                        
                        #cast raven
                        if pet_count[419] < 5 and ((GetTickCount() - self.raven_tick) >= 8000 or self.qt_raven_cast):
                            if GetSkillUIdBySide(1)[0] != Data.Raven: SelectSkill(Data.Raven, 1)
                            CastSkill(my_pos[0], my_pos[1], 1)
                            DLog("Summon Raven!")
                            self.qt_raven_cast -= 1
                            if self.qt_raven_cast == 0: self.raven_tick = GetTickCount()
                            
                            elif (GetTickCount() - self.raven_tick) >= 8000: self.qt_raven_cast = 5
                            
                            Sleep(350)
                            break
                        
                        #cast grizzly
                        if pet_count[428] < 1 and (GetTickCount() - self.grizzly_tick) >= 5000:
                            if GetSkillUIdBySide(1)[0] != Data.SummonGrizzly: SelectSkill(Data.SummonGrizzly, 1)
                            CastSkill(my_pos[0], my_pos[1], 1)
                            DLog("Summon Grizzly!")
                            self.grizzly_tick = GetTickCount()
                            Sleep(350)
                            break    
                        
                        #cast solar creeper
                        if pet_count[427] < 1 and (GetTickCount() - self.carrionvine_tick) >= 8000:
                            if GetSkillUIdBySide(1)[0] != Data.Vines: SelectSkill(Data.Vines, 1)
                            CastSkill(my_pos[0], my_pos[1], 1)
                            DLog("Summon Solar Creeper!")
                            self.carrionvine_tick = GetTickCount()
                            Sleep(350)
                            break
                        
                        ak_radius = GetAkRadius(target[0], tar_pos)
                        
                        if target[5] <= (ak_radius + 2): re_tele = False
                        
                        else:
                            DLog("Re-tele needed", 2)
                            re_tele = True
                        
                        if True:
                            more_units = self.GetNearbyMonsters()
                            if not more_units: break
                            
                            ir = False
                            
                            ak_pos = GetAkPos(tar_pos, more_units, (self.xRange_skip, self.yRange_skip), ak_radius, (self.xRange,self.yRange))
                            if not ak_pos:
                                DLog("No AK Pos <%x,%x>" % (tar_pos[0], tar_pos[1]), 1)
                                #print "%s, %d" % ( GetUnit(1, AK_TYPE, tar_id),  target[0])
                                target = None
                                break
                            
                            self.__ak_info = (tar_id, len(units))
                            ru = True
                            
                            if ak_pos != my_pos and not safe and re_tele:
                                if last_tar_pos:
                                    dist = GetDistance(last_tar_pos[0], last_tar_pos[1], tar_pos[0], tar_pos[1])
                                else:
                                    dist = 0xFFFF
                                
                                if qt_tel >= 2 or dist > 3:
                                    if qt_max_tel > 0:
                                        qt_tel -= 2
                                        qt_max_tel -= 1
                                        
                                        #print "Teleport %s, %s- %d" % (ak_pos, my_pos, dst_pos == tar_pos)
                                        if Teleport(ak_pos[0], ak_pos[1], 3, 1, 600):
                                            last_tar_pos = tar_pos
                                        elif last_tar_pos:
                                            npos = GetPos()
                                            if npos[0] != my_pos[0] or npos[1] != my_pos[1]:
                                                last_tar_pos = None
                                            
                                    else:
                                        target = None
                                        
                                    break
                        
                        self.__ak_info = (tar_id, len(units))
                        
                        if GetTickInterval(self.tick) < 400:
                            Sleep(10)
                            break
                        
                        qt_max_cas -= 1
                        
                        if qt_tel < 0:
                            qt_tel = 1
                        elif qt_tel < 4:
                            qt_tel += 1
                        
                        if GetSkillUIdBySide(1)[0] != Data.Tornado: SelectSkill(Data.Tornado, 1)
                        
                        CastSkillOnUnit(1, tar_id, True, False, True)
                        
                        DLog(">>>>>>AK[%d] %d" % (GetTickInterval(self.tick), tar_id))
                        self.tick = GetTickCount()
                        
                        Sleep(260)
                        
                        break
                    
                    if ru: ItemCheck(tar_id)
                    
                    if target and qt_max_cas > 0: #update npc
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
                        
                        target[5] = GetDistance(my_pos[0], my_pos[1], tar_pos[0], tar_pos[1])
                    
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
        