from D2EM import *

def _party_hdler():
    now_tick = GetTickCount()
    tick_intval = now_tick - party_info[0]
    if tick_intval >= 3000 or tick_intval >= 1000 and party_info[1]:
        party_info[0] = now_tick
        
        me_id = GetCurrentPlayerId()
        me_rel = GetPlayerRelation(me_id)
        if me_rel:
            rel_dict = dict(me_rel)
        else:
            rel_dict = {}
        
        ids = GetPlayerIds()
        pds = party_info[2]
        party_info[2] = new_pds = {}
        
        #refresh the database
        is_safe = 1
        for i in ids:
            if i == me_id: continue
            
            pr = rel_dict.get(i)
            if pr and pr & 8:
                if not IsInTown() and GetPos(0, i) and GetMapLevelNo() == GetMapLevelNo(0, i):
                    ExitGame(911)
                    
                is_safe = 0
                party_info[1] = 0 #set check intval to 1sec
                continue
            
            pd = pds.get(i)
            if not pd:
                pd = [0, ]
                party_info[1] = 0
                
            new_pds[i] = pd
        
        p_len = len(new_pds)
        ck = 0
        for i in new_pds.iteritems():
            player_id, player_db = i
            ck += 1
            pi = GetPartyInfo(player_id)
            
            #join a party already
            if not pi: continue
            
            cplayer = GetPlayer(player_id)
            if not cplayer: pass
            
            elif not player_db[0] and cplayer[3] in black_list :
                DLog("%s is blacklisted!" % (cplayer[3], ))
                SendMessage("%s is blacklisted, DO NOT invite him. Selfish people are not welcome." % (cplayer[3], ))
                player_db[0] = 1
                
            elif pi[0] == 0:
                if pi[1] == -1 and not player_db[0]:
                    DLog("Request Party: Player Id(%x), Party State(%x), Party Id(%x)" % ( player_id, pi[0], pi[1] ), 0)
                    RequestParty(player_id, 6)
                    player_db[0] = 1
            
            elif pi[0] == 2:
                DLog("Accept Party: Player Id(%x), Party State(%x), Party Id(%x)" % ( player_id, pi[0], pi[1] ), 0)
                RequestParty(player_id, 8)
            
            #resume after 1sec if stime is over 200ms
            if ck < p_len and GetTickCount() - now_tick > 200: return
            
        party_info[1] = 1 & is_safe #done, check every 3sec
        #DLog("Party Done[ms:%d]." % (GetTickCount() - now_tick, ), 0)
        
def GetBlackList():
    black_list = []
    try:
        bl = open(LIB_PATH + "/black_list.txt")
        for name in bl:
            black_list.append(name.strip())
            
        bl.close()
    
    except Exception, e: DLog(e, 2)
    
    return black_list
    
def _LibEntry(reason):
    global pevt_id, party_info, black_list
    if reason == 0: #game start
        DLog("Create Party Database.", 0)
        party_info = [ 0, 0, {} ] #[last_tick, is_done, player_data]
        pevt_id = -1
        black_list = GetBlackList()
        DLog(black_list)
        
    elif reason == 2:
        if pevt_id < 0:
            pevt_id = AttachEvent(EVT_ONWAKE, _party_hdler)
            DLog("Party Attached [%d]" % (pevt_id, ), 0)
        
    elif reason == 3:
        if pevt_id >= 0:
            DetachEvent(EVT_ONWAKE, pevt_id)
            DLog("Party Detached [%d]" % (pevt_id, ), 0)
            pevt_id = -1
        
#####
AttachLib(_LibEntry)

