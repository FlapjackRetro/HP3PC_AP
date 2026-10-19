import sys,re
def parse_patch(t):
    secs=[];cur=None
    for l in t.splitlines():
        l=l.strip()
        if not l: continue
        if l.startswith('['): cur=(l,[]); secs.append(cur)
        else:
            k,v=l.split('=',1); cur[1].append((k,v))
    return secs
def apply(ini,patch):
    nl='\r\n' if '\r\n' in ini else '\n'
    lines=ini.split(nl)
    for sec,kvs in parse_patch(patch):
        # find section
        idx=[i for i,l in enumerate(lines) if l.strip().lower()==sec.lower()]
        if not idx: lines+=['',sec]; idx=[len(lines)-1]
        s=idx[0]; e=s+1
        while e<len(lines) and not lines[e].strip().startswith('['): e+=1
        for k,v in kvs:
            hit=[i for i in range(s+1,e) if lines[i].split('=',1)[0].strip().lower()==k.lower()]
            if hit: lines[hit[0]]=f'{lines[hit[0]].split("=",1)[0]}={v}'
            else:
                ins=e
                while ins>s+1 and not lines[ins-1].strip(): ins-=1
                lines.insert(ins,f'{k}={v}'); e+=1
    return nl.join(lines)
if __name__=='__main__':
    ini=open(sys.argv[1],newline='',encoding='latin1').read()
    for p in sys.argv[3:]: ini=apply(ini,open(p,encoding='latin1').read())
    open(sys.argv[2],'w',newline='',encoding='latin1').write(ini)
