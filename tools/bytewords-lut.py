words = ['BYTES', 'CRYPTO-HDKEY', 'CRYPTO-KEYPATH', 'CRYPTO-MULTI-ACCOUNTS', 'CRYPTO-OUTPUT', 'CRYPTO-PSBT', 'DEV-AUTH', 'ETH-SIGN-REQUEST', 'ETH-SIGNATURE', 'FS-DATA', 'FW-UPDATE']

def hf(w, m):
    sum = 0
    for c in w:
        sum += ord(c)
    return ((sum * m) >> 28) & 0x0f

mult = 1
repeat = True
lut = {}

while repeat:
    repeat = False
    lut = {}
    for w in words:
        h = hf(w, mult)
        if h in lut:
            mult = mult + 1
            repeat = True
            break
        else:
            lut[h] = w

print(lut)    
print(mult)
