#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Author: Gality
Name：generator.py
Usage: python generator.py RC4key
require: None
Description: Generate encrypted shellcode（shellcode -> base64 -> RC4 -> base64）
E-mail: gality365@gmail.com
"""
import hashlib, base64, sys

'''
config your shellcode
'''
shellcode = "...Input your shellcode here..."


def rc4(text, key):
    # Use md5(key) to get 32-bit key instead raw key
    key = hashlib.md5(key).hexdigest()
    result = ''
    key_len = len(key)
    #1. init S-box
    box = list(range(256))#put 0-255 into S-box
    j = 0
    for i in range(256):#shuffle elements in S-box according to key
        j = (j + box[i] + ord(key[i%key_len]))%256
        box[i],box[j] = box[j],box[i]#swap elements
    i = j = 0
    for element in text:
        i = (i+1)%256
        j = (j+box[i])%256
        box[i],box[j] = box[j],box[i]
        k = chr(ord(element) ^ box[(box[i]+box[j])%256])
        result += k
    result = base64.b64encode(result)
    return result

def main():
    if len(sys.argv) != 2:
        print "Usage: python generator.py YourRC4Key"
        exit(0)
    else:
        key = sys.argv[1]


    #base64encode
    baseStr = base64.b64encode(shellcode)
    #RC4 + base64encode
    payload = rc4(baseStr, key)

    f = open('payload.txt', 'w')
    f.write(payload)
    f.close()
    print "Payload has been written to payload.txt！"

if __name__ == "__main__":
    main()

