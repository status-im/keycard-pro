words = ['BYTES', 'CRYPTO-ACCOUNT', 'CRYPTO-HDKEY', 'CRYPTO-MULTI-ACCOUNTS', 'CRYPTO-OUTPUT', 'CRYPTO-PSBT', 'DEV-AUTH', 'ETH-SIGN-REQUEST', 'ETH-SIGNATURE', 'FS-DATA', 'FW-UPDATE']

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


print("const char *const ur_type_string[] = {")
for i in range(16):
    if i in lut:
        print(f"  \"{lut[i]}\",")
    else:
        print("  NULL,")
print("};\n")

print("typedef enum {")
for i in range(16):
    if i in lut:
        print(f"  {lut[i].replace("-", "_")} = {i},")
print(f"  UR_ANY_TX = 255")
print("} ur_type_t;\n")

print(f"Multiplier: {mult}")
