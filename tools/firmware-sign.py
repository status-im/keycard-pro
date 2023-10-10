# This tool is for development only, not to be used for releases

import argparse
from secp256k1Crypto import PrivateKey
import hashlib

PAGE_SIZE = 8192
FW_PAGE_COUNT = 76
FW_SIZE = PAGE_SIZE * FW_PAGE_COUNT
FW_IV_SIZE = 588
SIG_SIZE = 64

def hash_firmware(fw):
    h = hashlib.sha256()
    h.update(fw[:FW_IV_SIZE])
    h.update(fw[FW_IV_SIZE+SIG_SIZE:])
    return h.digest()

def sign(sign_key, m):
    key = PrivateKey(bytes(bytearray.fromhex(sign_key)), raw=True)
    sig = key.ecdsa_sign(m, raw=True)
    return key.ecdsa_serialize_compact(sig)

def main():
    parser = argparse.ArgumentParser(description='Create a database from a token and chain list')
    parser.add_argument('-s', '--secret-key', help="the secret key file")
    parser.add_argument('-b', '--binary', help="the firmware binary file")
    parser.add_argument('-o', '--output', help="the output file")
    args = parser.parse_args()
    
    with open(args.secret_key) as f: 
        sign_key = f.read()

    fw = bytearray(b'\xff') * FW_SIZE
    with open(args.binary, 'rb') as f:
        actual_fw_size = f.readinto(fw)

    if (actual_fw_size % 16) != 0:
        actual_fw_size = ((actual_fw_size // 16) + 1) * 16

    m = hash_firmware(fw)
    signature = sign(sign_key, m)
    fw[FW_IV_SIZE:FW_IV_SIZE+SIG_SIZE] = signature

    with open(args.output, 'wb') as f:
        f.write(fw[0:actual_fw_size])

if __name__ == "__main__":
    main()