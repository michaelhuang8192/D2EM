import D2EM

__cur_script_nz = None

def OnDraw():
    if __cur_script_nz:
        str = "Script: " + __cur_script_nz
        x = D2EM.GetScreenCenter()[0] - D2EM.D2GetTextLength(str) / 2
        D2EM.D2DrawText(str, x, 15, 4)
        
    D2EM._OnDraw()

def OnExit():
    D2EM._OnExit()
    
def OnSendPacket(size, type, buf):
    return D2EM._OnSendPacket(size, type, buf)

def OnRecvPacket(size, type, buf):
    return D2EM._OnRecvPacket(size, type, buf)
    
def Main():
    global __cur_script_nz
    
    D2EM._Init()
    sc_count = 0
    try:
        D2EM.Sleep(0)
        nidx = 0
        for n in D2EM.GetScriptNzs():
            D2EM.DLog("+Start Script: %s" % n)
            __cur_script_nz = n
            s = {}
            D2EM._SetScript(s, nidx)
            nidx += 1
            try:
                run_script(n, s)
                sc_count += 1
                
            except Exception, e:
                print "Script Error:", n, ",", e
                D2EM.PrintErr()
            
            except D2EM.ScriptExitedException, se:
                print "Script Exited:", se
                if se.args and se.args[0] == 0: sc_count += 1
            
            finally:
                try:
                    OnExit()
                    
                finally:
                    s.clear()
                    s = None
                    D2EM._SetScript(None, None)
        
    except D2EM.GameExitedException, ge:
        print "Game Exited:", ge
        if ge.args and ge.args[0] == 0: sc_count += 1
    
    finally:
        D2EM._Reset()
        
    D2EM._UpdateSRate(sc_count)

def run_script(n, s):
    execfile(D2EM.SCRIPT_PATH + "\\" + n + "\\main.py", s)

