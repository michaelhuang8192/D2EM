from D2EM import *


def DrinkPotion(code, type=0):
    codeId = ItemCodeToClsId(code)
    equipPos = GetMapFromInv(1)[1]

    for n in range(0, 4):
        if not equipPos[n]: continue
        
        if GetUnit(0, 4, equipPos[n])[0] == codeId:
            UseItemFromBelt(equipPos[n], type)
            DLog("Drink Potion %d" % (codeId, ), 0)
            return True
    
    #DLog("Out of potion %d" % (codeId, ), 2)
    return False


#(l_rvl, l_hp5, m_rvl, m_mp5, mc_l_rvl, mc_l_hp5, l_exit)
cLifePercent = (45, 80, 1, 20, 25, 55, 20)

#[rvl, hp5, mp5, mc_rvl, mc_hp5]
potionTs = [0, 0, 0, 0, 0]

def OnLifeCheck():
    my_rm = GetRoom()
        
    if not my_rm: return
        
    level_no = my_rm[4]
    
    if IsTownLevel(level_no): return
    
    life = GetStat(0x06)
    max_life = GetStat(0x07)
    mana = GetStat(0x08)
    max_mana = GetStat(0x09)
        
    if not (life and max_life and mana and max_mana): return
    
    lp = float(life[1] >> 8) / (max_life[1] >> 8) * 100
    mp = float(mana[1] >> 8) / (max_mana[1] >> 8) * 100
    
    if lp < cLifePercent[6]:
        DLog("Low Life. Exit Game!", 3)
        ExitGame()
        
    ts = GetTickCount()
    if lp < cLifePercent[0]:
        if ts - potionTs[0] > 2000 and DrinkPotion("rvl"):
            potionTs[0] = ts
            DLog("Low Life, Drink rvl %.2f" % (lp, ), 0)
            return
    
    if lp < cLifePercent[1]:
        if ts - potionTs[1] > 7000 and DrinkPotion("hp5"):
            potionTs[1] = ts
            DLog("Medium Life, Drink hp5 %.2f" % (lp, ), 0)
            return
    
    if mp < cLifePercent[3]:
        if ts - potionTs[2] > 7000 and DrinkPotion("mp5"):
            potionTs[2] = ts
            DLog("Medium Mana, Drink mp5 %.2f" % (mp, ), 0)
            return
    
    if mp < cLifePercent[2]:
        if ts - potionTs[0] > 2000 and DrinkPotion("rvl"):
            potionTs[0] = ts
            DLog("Low Mana, Drink rvl %.2f" % (mp, ), 0)
            return
    
    mercId = GetMercId()
    
    if not mercId: return
    
    if IsDead(1, mercId): return
    
    merc_lp = GetNPCLife(mercId)
    
    if merc_lp < cLifePercent[4]:
        if ts - potionTs[3] > 3000 and DrinkPotion("rvl", 1):
            potionTs[3] = ts
            DLog("Merc Low Life, Drink rvl %.2f" % (merc_lp, ), 0)
            return
    
    if merc_lp < cLifePercent[5]:
        if ts - potionTs[4] > 10000 and DrinkPotion("hp5", 1):
            potionTs[4] = ts
            DLog("Merc Medium Life, Drink hp5 %.2f" % (merc_lp, ), 0)
            return      
        
        
        
        
        
        
        