"""Minimal reader + bytecode decompiler for Unreal Engine 2 packages, version 129
(KnowWonder's engine build used by Harry Potter and the Prisoner of Azkaban PC).

Library module; see decompile.py and build_catalog.py for command-line entry points.
Token layouts follow UELib (EliotVU/Unreal-Library); output is goto-style pseudo-UnrealScript.
"""
import struct, os, sys

RF_HasStack = 0x02000000
FUNC_Final=0x1; FUNC_Defined=0x2; FUNC_Iterator=0x4; FUNC_Latent=0x8; FUNC_PreOperator=0x10
FUNC_Singular=0x20; FUNC_Net=0x40; FUNC_NetReliable=0x80; FUNC_Simulated=0x100; FUNC_Exec=0x200
FUNC_Native=0x400; FUNC_Event=0x800; FUNC_Operator=0x1000; FUNC_Static=0x2000; FUNC_NoExport=0x4000
FUNC_Const=0x8000; FUNC_Invariant=0x10000; FUNC_Public=0x20000; FUNC_Private=0x40000; FUNC_Protected=0x80000; FUNC_Delegate=0x100000
CPF_Edit=0x1; CPF_Const=0x2; CPF_Input=0x4; CPF_ExportObject=0x8; CPF_OptionalParm=0x10; CPF_Net=0x20
CPF_EditConstArray=0x40; CPF_Parm=0x80; CPF_OutParm=0x100; CPF_SkipParm=0x200; CPF_ReturnParm=0x400
CPF_CoerceParm=0x800; CPF_Native=0x1000; CPF_Transient=0x2000; CPF_Config=0x4000; CPF_Localized=0x8000
CPF_Travel=0x10000; CPF_EditConst=0x20000; CPF_GlobalConfig=0x40000

class Reader:
    def __init__(s,d,p=0): s.d=d; s.p=p
    def u8(s): v=s.d[s.p]; s.p+=1; return v
    def u16(s): v=struct.unpack_from('<H',s.d,s.p)[0]; s.p+=2; return v
    def i32(s): v=struct.unpack_from('<i',s.d,s.p)[0]; s.p+=4; return v
    def u32(s): v=struct.unpack_from('<I',s.d,s.p)[0]; s.p+=4; return v
    def u64(s): v=struct.unpack_from('<Q',s.d,s.p)[0]; s.p+=8; return v
    def f32(s): v=struct.unpack_from('<f',s.d,s.p)[0]; s.p+=4; return v
    def ci(s):
        d=s.d; b=d[s.p]; s.p+=1; neg=b&0x80; v=b&0x3f
        if b&0x40:
            sh=6
            while True:
                b=d[s.p]; s.p+=1; v|=(b&0x7f)<<sh; sh+=7
                if not b&0x80: break
        return -v if neg else v
    def fstr(s):
        n=s.ci()
        if n<0: t=s.d[s.p:s.p-2*n].decode('utf-16le'); s.p+=-2*n
        else: t=s.d[s.p:s.p+n].decode('latin1'); s.p+=n
        return t.rstrip('\0')
    def cstr(s):
        e=s.d.index(b'\0',s.p); t=s.d[s.p:e].decode('latin1'); s.p=e+1; return t
    def ustr(s):
        e=s.p
        while s.d[e:e+2]!=b'\0\0': e+=2
        t=s.d[s.p:e].decode('utf-16le'); s.p=e+2; return t

class Package:
    def __init__(s,path):
        s.filepath=path; s.name=os.path.splitext(os.path.basename(path))[0]
        s.d=d=open(path,'rb').read(); r=Reader(d)
        (s.tag,s.ver,s.lic,s.flags,nc,no,ec,eo,ic,io)=struct.unpack('<IHHIIIIIII',d[:36])
        r.p=no; s.names=[]
        for i in range(nc):
            s.names.append(r.fstr()); r.u32()
        r.p=io; s.imports=[]
        for i in range(ic):
            cp=r.ci(); cn=r.ci(); pk=r.i32(); on=r.ci()
            s.imports.append(dict(cpkg=s.names[cp],cls=s.names[cn],pkg=pk,name=s.names[on]))
        r.p=eo; s.exports=[]
        for i in range(ec):
            cl=r.ci(); su=r.ci(); pk=r.i32(); on=r.ci(); fl=r.u32(); sz=r.ci(); off=r.ci() if sz>0 else 0
            s.exports.append(dict(idx=i+1,cls=cl,sup=su,pkg=pk,name=s.names[on],flags=fl,size=sz,off=off))
        s.cache={}
    def objname(s,i):
        if i>0: return s.exports[i-1]['name']
        if i<0: return s.imports[-i-1]['name']
        return 'None'
    def objclass(s,i):
        if i>0:
            c=s.exports[i-1]['cls']; return 'Class' if c==0 else s.objname(c)
        if i<0: return s.imports[-i-1]['cls']
        return None
    def outer(s,i):
        if i>0: return s.exports[i-1]['pkg']
        if i<0: return s.imports[-i-1]['pkg']
        return 0
    def path(s,i):
        parts=[]
        while i:
            parts.append(s.objname(i)); i=s.outer(i)
        return '.'.join(reversed(parts))
    def reader(s,i):
        e=s.exports[i-1]; return Reader(s.d,e['off'])

# ---------- tagged properties ----------
PT={1:'Byte',2:'Int',3:'Bool',4:'Float',5:'Object',6:'Name',7:'Delegate',8:'Class',9:'Array',10:'Struct',11:'Vector',12:'Rotator',13:'Str',14:'Map',15:'FixedArray'}
def read_tagged(pkg,r,limit=None):
    out=[]
    while True:
        nm=pkg.names[r.ci()]
        if nm=='None': break
        info=r.u8(); t=info&0x0f; st=(info>>4)&7; arr=info&0x80
        sname=None
        if t==10: sname=pkg.names[r.ci()]
        if st==0: size=1
        elif st==1: size=2
        elif st==2: size=4
        elif st==3: size=12
        elif st==4: size=16
        elif st==5: size=r.u8()
        elif st==6: size=r.u16()
        else: size=r.i32()
        idx=0
        if arr and t!=3:
            b=r.u8()
            if b&0x80==0: idx=b
            elif b&0xc0==0x80: idx=((b&0x7f)<<8)|r.u8()
            else: idx=((b&0x3f)<<24)|(r.u8()<<16)|(r.u8()<<8)|r.u8()
        start=r.p; val=None
        try:
            if t==3: val=bool(arr)
            elif t==1: val=r.u8() if size==1 else r.d[start:start+size].hex()
            elif t==2: val=r.i32()
            elif t==4: val=round(r.f32(),4)
            elif t in (5,8): v=r.ci(); val=pkg.path(v) if v else 'None'
            elif t==6: val=pkg.names[r.ci()]
            elif t==13: val=Reader(r.d,start).fstr()
            elif t==11: val=struct.unpack_from('<fff',r.d,start)
            elif t==12: val=struct.unpack_from('<iii',r.d,start)
            elif t==10: val=('STRUCT',sname,r.d[start:start+size])
            elif t==9: val=('ARRAY',r.d[start:start+size])
            else: val=f'<{PT.get(t,t)} {size}b>'
        except Exception as e: val=f'<err {e}>'
        if t!=3: r.p=start+size
        out.append((nm,idx,PT.get(t,t),val))
    return out

# ---------- object deserialization ----------
STRUCTLIKE={'Class','State','Function','Struct'}
PROPCLS={'ByteProperty','IntProperty','BoolProperty','FloatProperty','ObjectProperty','NameProperty','StrProperty','ClassProperty','ArrayProperty','StructProperty','VectorProperty','RotatorProperty','DelegateProperty','MapProperty','FixedArrayProperty','StringProperty','PointerProperty'}
def load(pkg,i):
    if i in pkg.cache: return pkg.cache[i]
    e=pkg.exports[i-1]; cls=pkg.objclass(i); r=pkg.reader(i)
    o=dict(idx=i,name=e['name'],cls=cls)
    pkg.cache[i]=o
    if e['flags']&RF_HasStack:
        node=r.ci(); sn=r.ci(); r.u64(); r.u32()
        if node: r.ci()
    if cls!='Class':
        o['props']=read_tagged(pkg,r)
    if cls in STRUCTLIKE or cls in PROPCLS or cls in ('Const','Enum'):
        o['super']=r.ci(); o['next']=r.ci()
    if cls=='Const': o['value']=r.fstr()
    elif cls=='Enum':
        n=r.ci(); o['values']=[pkg.names[r.ci()] for _ in range(n)]
    elif cls in PROPCLS:
        o['arraydim']=r.i32(); o['pflags']=r.u32(); o['category']=pkg.names[r.ci()]
        if o['pflags']&CPF_Net: r.u16()
        if cls in ('ByteProperty',): o['enum']=r.ci()
        elif cls in ('ObjectProperty',): o['pclass']=r.ci()
        elif cls=='ClassProperty': o['pclass']=r.ci(); o['meta']=r.ci()
        elif cls=='StructProperty': o['struct']=r.ci()
        elif cls in ('ArrayProperty','FixedArrayProperty'): o['inner']=r.ci()
        elif cls=='DelegateProperty': o['func']=r.ci()
        elif cls=='MapProperty': o['key']=r.ci(); o['val']=r.ci()
    elif cls in STRUCTLIKE:
        o['scripttext']=r.ci(); o['children']=r.ci(); o['friendly']=pkg.names[r.ci()]
        if pkg.ver>=120: o['cpptext']=r.ci()
        o['line']=r.i32(); o['textpos']=r.i32()
        o['scriptsize']=r.i32(); o['scriptpos']=r.p
        # bytecode parsed lazily; need to skip it now
        dec=Decompiler(pkg,o); o['code']=dec.parse(r)
        if cls=='Function':
            o['inative']=r.u16(); o['prec']=r.u8(); o['fflags']=r.u32()
            if o['fflags']&FUNC_Net: r.u16()
        elif cls in ('State','Class'):
            o['probe']=r.u64(); o['ignore']=r.u64(); o['labeltable']=r.u16(); o['sflags']=r.u32()
            if cls=='Class':
                o['cflags']=r.u32(); r.p+=16
                n=r.ci(); deps=[]
                for _ in range(n): deps.append(r.ci()); r.i32(); r.i32()
                n=r.ci(); [r.ci() for _ in range(n)]
                o['within']=r.ci(); o['config']=pkg.names[r.ci()]
                if pkg.ver>=99:
                    n=r.ci(); o['hidecat']=[pkg.names[r.ci()] for _ in range(n)]
                o['defaults']=read_tagged(pkg,r)
    o['end']=r.p
    return o

def children(pkg,o):
    out=[]; c=o.get('children',0)
    while c>0:
        ch=load(pkg,c); out.append(ch); c=ch.get('next',0)
    return out

# ---------- native table ----------
class Natives:
    def __init__(s): s.t={}
    def add_pkg(s,pkg):
        for e in pkg.exports:
            if pkg.objclass(e['idx'])=='Function':
                try: f=load(pkg,e['idx'])
                except Exception: continue
                if f.get('inative'):
                    s.t[f['inative']]=dict(name=f['name'],friendly=f['friendly'],flags=f['fflags'],prec=f['prec'],pkg=pkg.name)
NATIVES=Natives()

CASTS={0x39:'RotatorToVector',0x3A:'int',0x3B:'bool',0x3C:'float',0x3D:'byte',0x3E:'bool',0x3F:'float',0x40:'byte',0x41:'int',0x42:'float',0x43:'byte',0x44:'int',0x45:'bool',0x46:'name',0x47:'bool',0x48:'bool',0x49:'byte',0x4A:'int',0x4B:'bool',0x4C:'float',0x4D:'vector',0x4E:'rotator',0x4F:'bool',0x50:'rotator',0x51:'bool',0x52:'string',0x53:'string',0x54:'string',0x55:'string',0x56:'string',0x57:'string',0x58:'string',0x59:'string'}

class Tok:
    __slots__=('op','pos','a','kids','text')
    def __init__(s,op,pos): s.op=op; s.pos=pos; s.a={}; s.kids=[]; s.text=None

class Decompiler:
    def __init__(s,pkg,owner): s.pkg=pkg; s.owner=owner
    def parse(s,r):
        s.r=r; s.mem=0; size=s.owner['scriptsize']; stmts=[]
        try:
            while s.mem<size:
                stmts.append(s.next())
        except Exception as ex:
            stmts.append(('ERROR',repr(ex),s.mem))
            # can't recover position reliably; caller will be off. mark
            s.owner['parse_error']=repr(ex)
            r.p=s.owner['scriptpos']  # unknown; leave
            raise
        return stmts
    def obj(s): s.mem+=4; return s.r.ci()
    def name(s): s.mem+=4; return s.pkg.names[s.r.ci()]
    def b(s): s.mem+=1; return s.r.u8()
    def w(s): s.mem+=2; return s.r.u16()
    def i(s): s.mem+=4; return s.r.i32()
    def call_args(s):
        args=[]
        while True:
            t=s.next()
            if t.op==0x16: break
            args.append(t)
        return args
    def next(s):
        pos=s.mem; op=s.b(); t=Tok(op,pos); a=t.a
        if op>=0x60:
            if op<0x70: idx=((op-0x60)<<8)|s.b()
            else: idx=op
            a['native']=idx; t.kids=s.call_args(); t.op='native'; return t
        if op in (0x00,0x01,0x02): a['obj']=s.obj()
        elif op==0x04: t.kids=[s.next()]
        elif op==0x05: a['size']=s.b(); t.kids=[s.next()]
        elif op==0x06: a['off']=s.w()
        elif op==0x07: a['off']=s.w(); t.kids=[s.next()]
        elif op==0x08: pass
        elif op==0x09: a['line']=s.w(); t.kids=[s.next()]
        elif op==0x0A:
            a['off']=s.w()
            if a['off']!=0xFFFF: t.kids=[s.next()]
        elif op==0x0B: pass
        elif op==0x0C:
            labels=[]
            while True:
                n=s.name(); p=s.i()
                if n=='None': break
                labels.append((n,p))
            a['labels']=labels
        elif op==0x0D: t.kids=[s.next()]
        elif op==0x0E: t.kids=[s.next()]
        elif op==0x0F: t.kids=[s.next(),s.next()]
        elif op==0x10: t.kids=[s.next(),s.next()]
        elif op==0x11: t.kids=[s.next(),s.next(),s.next(),s.next()]
        elif op in (0x12,0x19): t.kids=[s.next()]; a['skip']=s.w(); a['size']=s.b(); t.kids.append(s.next())
        elif op==0x13: a['cls']=s.obj(); t.kids=[s.next()]
        elif op==0x14: t.kids=[s.next(),s.next()]
        elif op==0x15: a['line']=s.w(); t.kids=[s.next()]
        elif op==0x16: pass
        elif op==0x17: pass
        elif op==0x18: a['size']=s.w(); t.kids=[s.next()]
        elif op==0x1A: t.kids=[s.next(),s.next()]
        elif op==0x1B: a['name']=s.name(); t.kids=s.call_args()
        elif op==0x1C: a['func']=s.obj(); t.kids=s.call_args()
        elif op==0x1D: a['v']=s.i()
        elif op==0x1E: s.mem+=4; a['v']=round(s.r.f32(),6)
        elif op==0x1F: v=s.r.cstr(); s.mem+=len(v)+1; a['v']=v
        elif op==0x20: a['obj']=s.obj()
        elif op==0x21: a['v']=s.name()
        elif op==0x22: s.mem+=12; a['v']=(s.r.i32(),s.r.i32(),s.r.i32())
        elif op==0x23: s.mem+=12; a['v']=(round(s.r.f32(),4),round(s.r.f32(),4),round(s.r.f32(),4))
        elif op==0x24: a['v']=s.b()
        elif op in (0x25,0x26,0x27,0x28,0x2A): pass
        elif op==0x29: a['obj']=s.obj()
        elif op==0x2C: a['v']=s.b()
        elif op==0x2D: t.kids=[s.next()]
        elif op==0x2E: a['cls']=s.obj(); t.kids=[s.next()]
        elif op==0x2F: t.kids=[s.next()]; a['off']=s.w()
        elif op in (0x30,0x31): pass
        elif op in (0x32,0x33): a['obj']=s.obj(); t.kids=[s.next(),s.next()]
        elif op==0x34: v=s.r.ustr(); s.mem+=len(v)*2+2; a['v']=v
        elif op==0x35: s.mem+=8; s.r.p+=8
        elif op==0x36: a['obj']=s.obj(); t.kids=[s.next()]
        elif op==0x37: t.kids=[s.next()]
        elif op==0x38: a['name']=s.name(); t.kids=s.call_args()
        elif op==0x39: a['cast']=s.b(); t.kids=[s.next()]
        elif op==0x3A: pass
        elif op in (0x3B,0x3C,0x3D,0x3E): t.kids=[s.next(),s.next(),s.next()]
        elif op==0x3F: pass
        elif op in (0x40,0x41): t.kids=[s.next(),s.next(),s.next()]
        elif op==0x42: a['ver']=s.i(); a['line']=s.i(); a['tp']=s.i(); a['code']=s.b()
        elif op==0x43: a['prop']=s.obj(); a['name']=s.name(); t.kids=s.call_args()
        elif op==0x44: a['name']=s.name()
        elif op==0x45: t.kids=[s.next(),s.next()]
        elif op==0x47: pass
        elif op==0x48: t.kids=[s.next()]; s.w(); t.kids.append(s.next()); s.w(); t.kids.append(s.next())
        else: raise ValueError(f'bad token 0x{op:02x} at mem {pos} file {s.r.p-1:#x} in {s.owner["name"]}')
        return t

# ---------- pretty printer ----------
def fname(pkg,i):
    return pkg.objname(i)
def expr(pkg,t,owner=None):
    op=t.op; a=t.a; K=t.kids
    E=lambda x: expr(pkg,x,owner)
    if op=='native':
        n=NATIVES.t.get(a['native'])
        args=[E(k) for k in K]
        if not n: return f'native{a["native"]}({", ".join(args)})'
        f=n['friendly']; fl=n['flags']
        if fl&FUNC_Operator:
            if fl&FUNC_PreOperator: return f'{f}{args[0]}' if args else f
            if len(args)==1: return f'{args[0]}{f}'
            if len(args)==2: return f'({args[0]} {f} {args[1]})'
        return f'{f}({", ".join(args)})'
    if op in (0x00,0x01): return fname(pkg,a['obj'])
    if op==0x02: return 'Default.'+fname(pkg,a['obj'])
    if op==0x04: return 'return '+E(K[0]) if K and K[0].op not in (0x0b,) else 'return'
    if op==0x05: return f'switch ({E(K[0])})'
    if op==0x06: return f'goto L{a["off"]:04x}'
    if op==0x07: return f'if (!{E(K[0])}) goto L{a["off"]:04x}'
    if op==0x08: return 'stop'
    if op==0x09: return f'assert({E(K[0])})'
    if op==0x0A: return f'case {E(K[0])}: (next L{a["off"]:04x})' if K else 'default:'
    if op==0x0B: return ''
    if op==0x0C: return 'labels '+', '.join(f'{n}@L{p:04x}' for n,p in a['labels'])
    if op==0x0D: return f'goto {E(K[0])}'
    if op==0x0E: return E(K[0])
    if op in (0x0F,0x14,0x45): return f'{E(K[0])} = {E(K[1])}'
    if op==0x10: return f'{E(K[1])}[{E(K[0])}]'
    if op==0x11: return f'new({E(K[0])},{E(K[1])},{E(K[2])}) {E(K[3])}'
    if op==0x12: return f'{E(K[0])}.static.{E(K[1])}'
    if op==0x19: return f'{E(K[0])}.{E(K[1])}'
    if op==0x13: return f'class<{fname(pkg,a["cls"])}>({E(K[0])})'
    if op==0x15: return E(K[0])
    if op==0x17: return 'self'
    if op==0x18: return E(K[0])
    if op==0x1A: return f'{E(K[1])}[{E(K[0])}]'
    if op==0x1B: return f'{a["name"]}({", ".join(E(k) for k in K)})'
    if op==0x1C:
        fn=fname(pkg,a['func']); 
        # super call heuristic
        pre=''
        if owner and fn==owner['name']: pre='super.'
        return f'{pre}{fn}({", ".join(E(k) for k in K)})'
    if op in (0x1D,0x2C,0x24): return str(a['v'])
    if op==0x1E: return repr(a['v'])
    if op in (0x1F,0x34): return '"'+a['v'].replace('"','\\"')+'"'
    if op==0x20: return f"{pkg.objclass(a['obj'])}'{pkg.path(a['obj'])}'"
    if op==0x21: return f"'{a['v']}'"
    if op==0x22: return f'rot{a["v"]}'
    if op==0x23: return f'vect{a["v"]}'
    if op==0x25: return '0'
    if op==0x26: return '1'
    if op==0x27: return 'true'
    if op==0x28: return 'false'
    if op==0x29: return f'nativeparm {fname(pkg,a["obj"])}'
    if op==0x2A: return 'None'
    if op==0x2D: return E(K[0])
    if op==0x2E: return f'{fname(pkg,a["cls"])}({E(K[0])})'
    if op==0x2F: return f'foreach {E(K[0])}  /*end L{a["off"]:04x}*/'
    if op==0x30: return 'iteratorpop'
    if op==0x31: return 'iteratornext'
    if op==0x32: return f'({E(K[0])} == {E(K[1])})'
    if op==0x33: return f'({E(K[0])} != {E(K[1])})'
    if op==0x36: return f'{E(K[0])}.{fname(pkg,a["obj"])}'
    if op==0x37: return f'{E(K[0])}.Length'
    if op==0x38: return f'global.{a["name"]}({", ".join(E(k) for k in K)})'
    if op==0x39: return f'{CASTS.get(a["cast"],hex(a["cast"]))}({E(K[0])})'
    if op==0x3A: return 'return'
    if op==0x40: return f'{E(K[0])}.Insert({E(K[1])},{E(K[2])})'
    if op==0x41: return f'{E(K[0])}.Remove({E(K[1])},{E(K[2])})'
    if op==0x42: return ''
    if op==0x43: return f'{a["name"]}({", ".join(E(k) for k in K)})'
    if op==0x44: return a['name']
    if op==0x48: return f'({E(K[0])} ? {E(K[1])} : {E(K[2])})'
    return f'<op{op:#x}>'

def decompile_struct(pkg,o,indent='    '):
    lines=[]; tgt=set()
    def walk(t):
        if isinstance(t,Tok):
            if t.op in (0x06,0x07): tgt.add(t.a['off'])
            if t.op==0x0A and t.a['off']!=0xFFFF: tgt.add(t.a['off'])
            for k in t.kids: walk(k)
    for st in o.get('code',[]): walk(st)
    for st in o.get('code',[]):
        if not isinstance(st,Tok): lines.append(f'{indent}// {st}'); continue
        if st.pos in tgt: lines.append(f'  L{st.pos:04x}:')
        s=expr(pkg,st,o)
        if s: lines.append(indent+s+';')
    return lines

def fflags_str(f):
    out=[]
    for bit,n in [(FUNC_Native,'native'),(FUNC_Final,'final'),(FUNC_Static,'static'),(FUNC_Simulated,'simulated'),(FUNC_Latent,'latent'),(FUNC_Iterator,'iterator'),(FUNC_Singular,'singular'),(FUNC_Exec,'exec'),(FUNC_Event,'event'),(FUNC_Private,'private'),(FUNC_Protected,'protected'),(FUNC_Delegate,'delegate')]:
        if f&bit: out.append(n)
    if f&FUNC_Event: out.remove('event'); kw='event'
    elif f&FUNC_Delegate: kw='delegate'
    else: kw='function'
    if 'delegate' in out: out.remove('delegate')
    return ' '.join(out+[kw])

def ptype(pkg,p):
    c=p['cls']
    if c=='ObjectProperty': return pkg.objname(p['pclass'])
    if c=='ClassProperty': return f'class<{pkg.objname(p["meta"])}>'
    if c=='StructProperty': return pkg.objname(p['struct'])
    if c=='ByteProperty': return pkg.objname(p['enum']) if p.get('enum') else 'byte'
    if c in ('ArrayProperty',):
        inner=load(pkg,p['inner']) if p['inner']>0 else None
        return f'array<{ptype(pkg,inner) if inner else "?"}>'
    if c=='DelegateProperty': return 'delegate<'+pkg.objname(p['func'])+'>'
    return {'IntProperty':'int','BoolProperty':'bool','FloatProperty':'float','NameProperty':'name','StrProperty':'string','VectorProperty':'vector','RotatorProperty':'rotator'}.get(c,c)

def dump_class(pkg,ci):
    c=load(pkg,ci); L=[]
    sup=pkg.objname(pkg.exports[ci-1]['sup']) if pkg.exports[ci-1]['sup'] else None
    L.append(f'class {c["name"]}'+(f' extends {sup}' if sup else '')+(f' config({c["config"]})' if c.get('config') not in (None,'None','System') else '')+';')
    def dump_fields(owner,ind):
        for ch in reversed(children(pkg,owner)):
            k=ch['cls']
            if k in PROPCLS:
                if ch['pflags']&CPF_Parm: continue
                fl=[]
                if ch['pflags']&CPF_Config: fl.append('config')
                if ch['pflags']&CPF_Travel: fl.append('travel')
                if ch['pflags']&CPF_Transient: fl.append('transient')
                if ch['pflags']&CPF_Localized: fl.append('localized')
                if ch['pflags']&CPF_Native: fl.append('native')
                ed='()' if ch['pflags']&CPF_Edit else ''
                arr=f'[{ch["arraydim"]}]' if ch['arraydim']>1 else ''
                L.append(f'{ind}var{ed} {" ".join(fl)+" " if fl else ""}{ptype(pkg,ch)} {ch["name"]}{arr};')
            elif k=='Const': L.append(f'{ind}const {ch["name"]} = {ch["value"]};')
            elif k=='Enum': L.append(f'{ind}enum {ch["name"]} {{ {", ".join(ch["values"])} }};')
            elif k=='Struct':
                L.append(f'{ind}struct {ch["name"]} {{'); dump_fields(ch,ind+'    '); L.append(f'{ind}}};')
        for ch in reversed(children(pkg,owner)):
            k=ch['cls']
            if k=='Function':
                params=[];ret=None;locs=[]
                for p in children(pkg,ch):
                    if p['cls'] not in PROPCLS: continue
                    if p['pflags']&CPF_ReturnParm: ret=ptype(pkg,p)
                    elif p['pflags']&CPF_Parm:
                        q=('optional ' if p['pflags']&CPF_OptionalParm else '')+('out ' if p['pflags']&CPF_OutParm else '')
                        params.append(f'{q}{ptype(pkg,p)} {p["name"]}')
                    else: locs.append(f'{ptype(pkg,p)} {p["name"]}')
                hdr=f'{ind}{fflags_str(ch["fflags"])} {ret+" " if ret else ""}{ch["name"]}({", ".join(params)})'
                if ch['fflags']&FUNC_Native and ch.get('inative'): hdr+=f' /*native {ch["inative"]}*/'
                if not ch.get('code') or (len(ch['code'])==1 and isinstance(ch['code'][0],Tok) and ch['code'][0].op in (0x04,0x47)):
                    L.append(hdr+';'); continue
                L.append(hdr+' {')
                for l in locs: L.append(f'{ind}    local {l};')
                L.extend(decompile_struct(pkg,ch,ind+"    "))
                L.append(f'{ind}}}')
            elif k=='State':
                L.append(f'{ind}state {ch["name"]}'+(f' extends {pkg.objname(ch["super"])}' if ch.get('super') else '')+' {')
                dump_fields(ch,ind+'    ')
                if ch.get('code'):
                    L.append(f'{ind}  // state code'); L.extend(decompile_struct(pkg,ch,ind+"    "))
                L.append(f'{ind}}}')
    dump_fields(c,'    ')
    if c.get('code'): L.append('  // class code'); L+=decompile_struct(pkg,c,'    ')
    L.append('defaultproperties {')
    for n,idx,t,v in c.get('defaults',[]):
        L.append(f'    {n}{"("+str(idx)+")" if idx else ""}={fmtval(v)}')
    L.append('}')
    return '\n'.join(L)

def decode_struct_array(pkg,raw):
    r=Reader(raw); n=r.ci(); out=[]
    for _ in range(n):
        out.append({k if not i else f'{k}({i})':v for k,i,t,v in read_tagged(pkg,r)})
    return out
def fmtval(v):
    if isinstance(v,tuple) and v and v[0]=='ARRAY': return f'<array {len(v[1])}b>'
    if isinstance(v,tuple) and v and v[0]=='STRUCT': return f'<{v[1]} {v[2].hex()[:64]}>'
    return v
