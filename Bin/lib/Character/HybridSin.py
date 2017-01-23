from D2EM import *
from ItemCheck import *
from D2EMExtended import *
import Data


def GetTotalTrapDist(traps):
    return traps[1][2]
    
def GetLayTrapPos(tar_pos, units, skip_range=([],[]), radius=18, in_range=([],[])):
    trapPos = {}
    skip = 0
    pops = []
    
    staticTrapPos = CirclePoints(tar_pos)
    size = len(staticTrapPos)
    
    for i in range(size):
        pt = GetPointInMap(staticTrapPos[i][0], staticTrapPos[i][1])
        if pt == None or pt & 0x1C01:
            pops.append(i)
            skip = 10
            continue
        
        if skip:
            skip -= 1
            continue
            
        trapPos[i] = [staticTrapPos[i][0], staticTrapPos[i][1]]
    
    for p in pops:
        s = p - 10
        e = p + 10
        
        for j in range(s, e):
            if trapPos.has_key(j): trapPos.pop(j)
    
    if len(skip_range[0]) and len(skip_range[1]):
        posList = trapPos.keys()
        
        for i in posList:
            if skip_range[0][0] <= trapPos[i][0] <= skip_range[0][1]:
                if skip_range[1][0] <= trapPos[i][1] <= skip_range[1][1]:
                    trapPos.pop(i)
                    
    if len(in_range[0]) and len(in_range[1]):
        if len(trapPos):
            posList = trapPos.keys()
        
            for i in posList:
                if in_range[0][0] <= trapPos[i][0] <= in_range[0][1]:
                    if in_range[1][0] <= trapPos[i][1] <= in_range[1][1]:
                        continue
                    
                trapPos.pop(i)
    
    if len(trapPos):
        posList = trapPos.keys()
        
        for i in posList:
            totalDist = 0
            nearMonCount = 0
            for u in units:
                dist = GetDistance(trapPos[i][0], trapPos[i][1], units[u][4][0], units[u][4][1])
                if dist <= 2: nearMonCount += 1
                
                if nearMonCount > 2:
                    totalDist = 0
                    break
                
                if dist <= 20: totalDist += dist
                
            trapPos[i].append(int(totalDist))
        
        while trapPos:
            destTrapPos = max(trapPos.iteritems(), key=GetTotalTrapDist)
            tPos = destTrapPos[1]
            tKey = destTrapPos[0]
            
            if TestPath(tar_pos, tPos):
                #print "Found best trap pos:", " - ", destTrapPos
                return (tPos[0], tPos[1])
            
            trapPos.pop(tKey)
                  
    #print "No pos for laying traps, use tar_pos!"
    return tar_pos

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

def NeedToLayTrap(targetPos):
    pets = GetPets()
    trap = []
        
    if pets:
        for p in pets:
            if p[2] == 17:
                trap.append(p)
        
    if len(trap) < 5:
        #print "Lay trap, number of traps less than 5"
        return 1
        
    for t in trap:
        trapPos = GetPos(1, t[1])
        
        if not trapPos:
            #print "Get trap pos error"
            return 2
        
        dist = GetDistance(trapPos[0], trapPos[1], targetPos[0], targetPos[1])
        if dist > 25:
            #print "Trap too far away, relay trap"
            return 3
        
        if not TestPath(targetPos, trapPos):
            #print "obstacle in way, relay trap"
            return 3
    
    #print "No Need To Lay Trap"    
    return False

def CountTrap(destTrapPos):
    pets = GetPets()
    traps = {}
    trapCount = 0
        
    if pets:
        for p in pets:
            if p[2] == 17:
                traps[p[1]] = p
    
    
    for t in traps:
        trapPos = GetPos(1, traps[t][1])
        
        if not trapPos: continue
        
        dist = GetDistance(trapPos[0], trapPos[1], destTrapPos[0], destTrapPos[1])
        if dist <= 8: trapCount += 1
        
    return trapCount


AK_TYPE = 1
class HybridSin:
    def __init__(self):
        self.target_id = -1
        self.fid = AttachEvent(EVT_ONDRAW, self.mark_target)
        self.target = []
        self.tar_name = []
        self.xRange = []
        self.yRange = []
        self.xRange_skip = []
        self.yRange_skip = []
    
    def Precast(self):
        myPos = GetPos()
        if HasCta() and GetSkillBySide(0)[0]:
            SwitchWeapon()
        
            SelectSkill(Data.BattleCommand, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(600)
            SelectSkill(Data.BattleOrders, 1)
            CastSkill(myPos[0], myPos[1], 1)
            Sleep(600)
    
        if not GetSkillBySide(0)[0]: SwitchWeapon()
    
        SelectSkill(Data.Fade, 1)
        CastSkill(myPos[0], myPos[1], 1)
        Sleep(500)
        SelectSkill(Data.Venom, 1)
        CastSkill(myPos[0], myPos[1], 1)
        Sleep(500)
        SelectSkill(Data.ShadowMaster, 1)
        CastSkill(myPos[0], myPos[1], 1)
        Sleep(500)
    
    def InTownPrecast(self): return
    
    def mark_target(self):
        if self.target_id < 0: return
        sp = GetScreenOffset(AK_TYPE, self.target_id)
        if not sp: return
        
        left = sp[0] - 50
        top = sp[1] - 20
        right = sp[0] + 50
        bottom = sp[1] + 20
        
        DrawText("ID: %d" % self.target_id, 0x0033FF, left, top, right, bottom, 1)
    
    def get_dist(self, unit):
        return unit[5]
    
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
        
    def Attack(self):
        ItemCheck()
        units_d = {}
        units = {}
        tick = 0
        
        while True:
            all_units = GetNearbyUnits(AK_TYPE)
            if not all_units: break
            
            my_pos = GetPos()
            my_rm = GetRoom()
            
            units.clear()
            for u in all_units:
                uid = u[1]
                
                if len(self.target) and not (u[0] in self.target): continue
                    
                if len(self.tar_name) and not (GetName(1, u[1]) in self.tar_name): continue
                
                if not my_rm or my_rm[4] != u[4] or IsTownLevel(u[4]): continue
                
                life = GetLife(AK_TYPE, uid)
                if not life or not IsAttackable(AK_TYPE, uid): continue
                
                tar_pos = GetPos(AK_TYPE, uid)
                
                if len(self.xRange) and not (self.xRange[0] <= tar_pos[0] <= self.xRange[1]): continue
                
                if len(self.yRange) and not (self.yRange[0] <= tar_pos[1] <= self.yRange[1]): continue
                
                if len(self.xRange_skip) and (self.xRange_skip[0] <= tar_pos[0] <= self.xRange_skip[1]):
                    if len(self.yRange_skip) and (self.yRange_skip[0] <= tar_pos[1] <= self.yRange_skip[1]): continue
                
                ud = units_d.get(uid, None)
                if not ud:
                    dist = GetDistance(my_pos[0], my_pos[1], tar_pos[0], tar_pos[1])
                elif ud[0] > 0:
                    dist = 0xFFFF - ud[0]
                else:
                    continue
                
                units[uid] = [u[0], u[1], u[2], u[3], tar_pos, dist] #clsid, id, state, flag, pos, dist
                
            if not units: break
            
            ru = False
            while units:   
                target = min(units.itervalues(), key=self.get_dist)
                tar_pos = target[4]
                tar_id = target[1]
                
                ft_count = 2
                curTrapPos = []
                self.target_id = tar_id
                ts = 0
                mb_tick = 0
                
                #print ">>>>target %d<<<<" % tar_id
                
                while True:
                    while True:
                        layTrapStatus = NeedToLayTrap(tar_pos)
                        if layTrapStatus:                               
                            if ft_count:
                                if ft_count == 2:
                                    ft_count -= 1
                                    more_units = self.GetNearbyMonsters()
                                    if not more_units: break
                                    
                                    safeTrapPos1 = GetLayTrapPos(tar_pos, more_units, (self.xRange_skip, self.yRange_skip), 18, (self.xRange,self.yRange))
                                    
                                    if not safeTrapPos1:
                                        #print "Find no trap position monster clsId: %d, dist: %d " % (target[0], target[5])
                                        target = None
                                        break
                                    
                                    else:
                                        curTrapPos = safeTrapPos1
                                        #print "Tele to first trap pos, myPos: (%d, %d); trapPos: (%d, %d)" % (GetPos()[0], GetPos()[1], curTrapPos[0], curTrapPos[1])
                                        Teleport(curTrapPos[0], curTrapPos[1], 4, 1, 600)
                                        ru = True
                                        break
                                    
                                elif ft_count == 1:
                                    ft_count -= 1
                                    more_units = self.GetNearbyMonsters()
                                    if not more_units: break
                                    
                                    safeTrapPos2 = GetLayTrapPos(tar_pos, more_units, (self.xRange_skip, self.yRange_skip), 18, (self.xRange,self.yRange))
                                    
                                    if not safeTrapPos2 or safeTrapPos2 == tar_pos:
                                        pass
                                        #print "Find only one trap position, use first trap position"
                                    
                                    elif GetDistance(safeTrapPos2[0], safeTrapPos2[1], curTrapPos[0], curTrapPos[1]) <= 3:
                                        pass
                                        #print "2nd trap pos = 1st trap pos, no tele req'ed"
                                        
                                    else:
                                        curTrapPos = safeTrapPos2
                                        #print "Tele to 2nd trap pos, myPos: (%d, %d); trapPos: (%d, %d)" % (GetPos()[0], GetPos()[1], curTrapPos[0], curTrapPos[1])
                                        Teleport(curTrapPos[0], curTrapPos[1], 4, 1, 600)
                                        ts = GetTickCount()
                                        break
                                    
                                    ts = GetTickCount()
                            
                            ru = True
                            
                            if layTrapStatus == 3 and ft_count == 0 and ((GetTickCount() - ts) > 4000):
                                #print "Resetting ft_count, going to find new trap pos!"
                                ft_count = 2
                            
                            if CountTrap(curTrapPos) < 5:
                                if GetTickCount() - tick < 400:
                                    Sleep(0)
                                    break
                                 
                                if GetSkillUIdBySide(1)[0] != Data.LightningSentry:
                                    SelectSkill(Data.LightningSentry, 1)
                                    
                                CastSkill(my_pos[0], my_pos[1], 1)
                                #print "Lay trap tar(%d), - %s" % (tar_id, GetName(1, tar_id))
                                tick = GetTickCount()
                                Sleep(250)
                                break
                        
                        #if GetTickCount() - mb_tick < (400, 500, 600, 700)[mb_tick % 4]:
                        if GetTickCount() - mb_tick < 400:
                            Sleep(0)
                            break
                        
                        mb_targets = self.GetNearbyMonsters()
                        if mb_targets:
                            if len(units) <= 2:
                                for suid in units:
                                    if mb_targets.has_key(suid):
                                        #print "remove %s from mb_targets" % GetName(1, suid)
                                        mb_targets.pop(suid)
                            
                            if len(mb_targets):    
                                mb_target = min(mb_targets.itervalues(), key=self.get_dist)
                            
                                if GetSkillUIdBySide(1)[0] != Data.MindBlast:
                                    SelectSkill(Data.MindBlast, 1)
                                            
                                CastSkill(mb_target[4][0], mb_target[4][1], 1)
                                #print "Mindblast (%d), - %s - Pos: (%d, %d)" % (mb_target[1], GetName(1, mb_target[1]), tar_pos[0], tar_pos[1])
                                mb_tick = GetTickCount()
                                Sleep(250)
                                break       
                        
                        Sleep(0)
                        break
                    
                    ItemCheck(tar_id)
                    
                    if target: #update npc
                        my_rm = GetRoom()
                        tar_rm = GetRoom(AK_TYPE, tar_id)
                        if not my_rm or not tar_rm or my_rm[4] != tar_rm[4] or IsTownLevel(tar_rm[4]): break
                        
                        life = GetLife(AK_TYPE, tar_id)
                        if not life or not IsAttackable(AK_TYPE, tar_id): break
                        
                        tar0 = GetUnit(0, AK_TYPE, tar_id)
                        target[2] = tar0[1]
                        target[3] = tar0[2]
                        
                        target[4] = tar_pos = GetPos(AK_TYPE, tar_id)
                        my_pos = GetPos()
                    
                    else:
                        ud = units_d.get(tar_id, None)
                        if ud:
                            units_d[tar_id] = (ud[0] - 1, 0)
                        else:
                            units_d[tar_id] = (1, 0)
                            
                        break
                
                if ru:
                    #print "Break out while units"
                    break
                else:
                    #print "Del target %d" % tar_id 
                    del units[tar_id]
            
            Sleep(0)
                    
                    