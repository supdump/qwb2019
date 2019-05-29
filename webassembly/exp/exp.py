#!python3.6
import struct

def decrypt(v0, v1, key):
	delta = 0x9e3779b9
	n = 32
	sum = (delta * n)
	mask = 0xffffffff
	for round in range(n):
			v1 = (v1 - (((v0<<4 ^ v0>>5) + v0) ^ (sum + k[sum>>11 & 3]))) & mask
			sum = (sum - delta) & mask
			v0 = (v0 - (((v1<<4 ^ v1>>5) + v1) ^ (sum + k[sum & 3]))) & mask
	
	return struct.pack("i",v0) + struct.pack("i",v1)

block = [0xE7689695, 0xC91755b7, 0xCF1e03ad, 0x4B61c56f, 0x2Dfd9002, 0x930aed22, 0xECc97e30, 0xE0B1968c]
k = [0,0,0,0]

flag = ''
for i in range(4):
	 flag = flag + ((decrypt(block[i*2], block[i*2+1], k)).decode())

flag = flag + '\x65\x36\x38\x62\x62\x7d'

print(flag)