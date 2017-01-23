import Data
from D2EM import *
import Command
import D2EMExtended

myBo = 1
compliment = False
myBoFromConfig = GetScriptConfig("myBo")
if myBoFromConfig:
    myBo = int(myBoFromConfig)
    compliment = True


def OnPacketUpdate(packet_id, dest, args):
    if baal["id"] and dest == 1: TrackBaalSkills(args)
    
    if dest == 1: TrackPlayerSkills(args)
    
    if dest == 0 and packet_id == 0x15:
        Command.CommandParser(args)
        
def TrackBaalSkills(args):
    if len(args) >= 6 and args[1] == baal["id"]:
        if baal.has_key(args[2]):
            DLog(args)
            baal[args[2]] += 1

def BoCompliment(boLvl, boer):
    global curBoLvl
    if boLvl > curBoLvl:
        curBoLvl = boLvl
        if boLvl == 56:
            SendMessage("Thanks, %s. #1 Bo! [Level: %d]" % (boer, boLvl))
        elif boLvl >= 50:
            SendMessage("GFG bo niggar! Thanks, %s. [Level: %d]" % (boer, boLvl))
        elif boLvl >= 40:
            SendMessage("Damn, nice bo niggar! Thanks %s. [Level: %d]" % (boer, boLvl))
        elif boLvl >= 31:
            SendMessage("Thanks for bo, %s! [Bo Level: %d]" % (boer, boLvl))
    
def TrackPlayerSkills(args):
    global barbBo, lowBo
    if len(args) >= 6 and args[0] == 0:
        if args[2] == 149:
            pids = GetPlayerIds()
            if args[1] in pids and GetPartyInfo(args[1])[0] == 1:
                myPos = GetPos()
                bPos = GetPos(0, args[1])
                boer = GetName(0, args[1])
                if bPos and boer and GetDistance(myPos[0], myPos[1], bPos[0], bPos[1]) <= 20:
                    if args[3] > 20:
                        DLog("Barb bo, lvl: %d!" % (args[3], ), 0)
                        if compliment: BoCompliment(args[3], boer)
                        barbBo = True
                        
                    else:
                        if args[3] < lowBo:
                            charType = D2EMExtended.GetCharClass(args[1])
                            if charType and charType == 4:
                                if compliment: SendMessage("Please max battle order first, %s! [Bo Level: %d]" % (boer, args[3]))
                            else:
                                if args[3] < myBo and (GetName(0, args[1]) != "evilmfD"):
                                    if compliment: SendMessage("Thanks for your pussy fart bo, %s! [Bo Level: %d]" % (boer, args[3]))
                            
                            lowBo = args[3]
                            
                        DLog("Cta bo, lvl: %d!" % (args[3], ), 0)
                        barbBo = False
                
def _ResetBaalSDB(reason):
    if reason == 0: #game start
        global baal, barbBo, curBoLvl, lowBo
        DLog("Monitor Baal in Throne.", 0)
        
        baal = {"id": False,
                Data.BaalCorpseExplode: 0,
                Data.BaalMonsterSpawn: 0}
        
        DLog("My bo lv %d" % (myBo, ))
        barbBo = False
        curBoLvl = 30
        lowBo = 20
        
        
AttachLib(_ResetBaalSDB)

#dest 1 = packet for client; 0 = packet for server