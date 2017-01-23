from D2EM import *

def _LeechCheck():
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
            
            if pi[0] == 0 and (GetMasterPlayerId() != player_id):
                if pi[1] == -1 and not player_db[0]:
                    player_db[0] = 1
            
            elif pi[0] == 2 and (GetMasterPlayerId() == player_id):
                DLog("Accept Party: Player Id(%x), Party State(%x), Party Id(%x)" % ( player_id, pi[0], pi[1] ), 0)
                RequestParty(player_id, 8)
            
            #resume after 1sec if stime is over 200ms
            if ck < p_len and GetTickCount() - now_tick > 200: return
            
        party_info[1] = 1 & is_safe #done, check every 3sec
        #DLog("Party Done[ms:%d]." % (GetTickCount() - now_tick, ), 0)
        
        #Check if master is in game
        if not ISMCtrl(1, s_info[1]): ExitGame(0)
        
def GetSessionInfo():
    return ISMCtrl(0)

def IsMaster(s_info):
    master = True
    
    if s_info[0] != s_info[1]: master = False
    
    return master

def SetMasterId(my_sid):
    DLog("Setting master player ID %d" % GetCurrentPlayerId())
    ISMCtrl(3, my_sid, GetCurrentPlayerId())

def GetMasterPlayerId():
    #DLog("master session id %d" % s_info[1])
    mpId = ISMCtrl(2, s_info[1])[0]
    #DLog("Master player ID %d" % mpId)
    return mpId
   
def _LeechInit(reason):
    global pevt_id, s_info, party_info, master
    if reason == 0: #game start
        party_info = [ 0, 0, {} ] #[last_tick, is_done, player_data]
        pevt_id = -1
        DLog("Gather leech mode data.", 0)
        s_info = GetSessionInfo()
        master = IsMaster(s_info)
        DLog(master)
        
    elif reason == 2:
        if pevt_id < 0 and not master:
            pevt_id = AttachEvent(EVT_ONWAKE, _LeechCheck)
            DLog("LeechCheck Attached [%d]" % (pevt_id, ), 0)
        elif master:
            SetMasterId(s_info[0])
        
    elif reason == 3:
        if pevt_id >= 0 and not master:
            DetachEvent(EVT_ONWAKE, pevt_id)
            DLog("LeechCheck Detached [%d]" % (pevt_id, ), 0)
            pevt_id = -1
            

AttachLib(_LeechInit)