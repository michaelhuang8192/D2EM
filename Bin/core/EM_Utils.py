import sys, math, time
from _D2EM import GetPrivateProfileString

__dlvl_str = ('INFO', 'ERROR', 'WARNING', 'CRITICAL', 'DEBUG')
def DLog(dmsg, dlvl=0):
    print "[%s][%s] %s" % (time.strftime("%x %X"), __dlvl_str[dlvl], dmsg)
    
def PrintErr():
    try:
        ei = sys.exc_info()
        sys.excepthook(ei[0], ei[1], ei[2])
    finally:
        ei = None

def RGB(red, green, blue):
    return ( (red&0xff) | ((green&0xff)<<8) | ((blue&0xff)<<16) )

def PrintHex(str):
    for i in str:
        DLog("%02X" % (ord(i), ), 0)
        
    print

def GetDistance(x0, y0, x1, y1):
    return ( (y1 - y0) ** 2 + (x1 - x0) ** 2 ) ** 0.5
    
def CirclePoints(point, radius=18):
    points = {}
    
    for i in range(0,361):
        dx = radius*math.cos(math.radians(i))
        dy = radius*math.sin(math.radians(i))
        nx = int(round(point[0] + dx))
        ny = int(round(point[1] + dy))
        points[i] = (nx, ny)
        
    return points

def ParseConfigFile(fname):
    sect_s = GetPrivateProfileString("", "", "", fname)
    if not sect_s: return None

    sect_b = []
    for s in sect_s.split('\x00'):
        if not s: continue
        
        key_s = GetPrivateProfileString(s, "", "", fname)
        key_b = []
        if key_s:
            for k in key_s.split('\x00'):
                if not k: continue
                
                val_s = GetPrivateProfileString(s, k, "", fname)
                if val_s: key_b.append( (k, val_s) )
            
        sect_b.append( (s, key_b) )
        
    return sect_b
