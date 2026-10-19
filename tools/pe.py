import struct
class PE:
    def __init__(s,path):
        s.d=d=open(path,'rb').read()
        pe=struct.unpack('<I',d[0x3c:0x40])[0]; s.opt=opt=pe+24
        s.base=struct.unpack('<I',d[opt+28:opt+32])[0]
        ns=struct.unpack('<H',d[pe+6:pe+8])[0]; so=opt+struct.unpack('<H',d[pe+20:pe+22])[0]
        s.secs=[]
        for i in range(ns):
            x=d[so+40*i:so+40*i+40]
            vs,va,rs,rp=struct.unpack('<IIII',x[8:24]); s.secs.append((x[:8].rstrip(b'\0').decode(),va,max(vs,rs),rp,rs))
    def off(s,rva):
        for n,va,sz,rp,rs in s.secs:
            if va<=rva<va+sz: return rva-va+rp
    def exports(s):
        d=s.d;opt=s.opt
        erva=struct.unpack('<I',d[opt+96:opt+100])[0]
        if not erva: return []
        eo=s.off(erva)
        nf,nn,af,anp,aop=struct.unpack('<IIIII',d[eo+20:eo+40])
        ex=[]
        for i in range(nn):
            no=s.off(struct.unpack('<I',d[s.off(anp)+4*i:s.off(anp)+4*i+4])[0]); name=d[no:d.index(b'\0',no)].decode()
            o=struct.unpack('<H',d[s.off(aop)+2*i:s.off(aop)+2*i+2])[0]
            rva=struct.unpack('<I',d[s.off(af)+4*o:s.off(af)+4*o+4])[0]
            t=s.base+rva; fo=s.off(rva)
            if fo is not None and fo<len(d)-5 and d[fo]==0xe9: t=t+5+struct.unpack('<i',d[fo+1:fo+5])[0]
            ex.append((t,name))
        return sorted(ex)
