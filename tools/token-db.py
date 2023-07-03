import json
import argparse
import struct

CHAIN_MAGIC = 0x4348
ERC20_MAGIC = 0x3020

def serialize_addresses(addresses):
    res = b''
    for id, address in addresses.items():
        res = res + struct.pack("<B20s", id, bytes.fromhex(address[2:]))

    return res
        
def serialize_db(f, chains, tokens):
    for chain in chains.values():
        chain_len = 4 + 1 + len(chain["ticker"]) + 1 + len(chain["name"]) + 1
        f.write(struct.pack("<HHIB", CHAIN_MAGIC, chain_len, chain["chainId"], chain["id"]))
        f.write(bytes(chain["ticker"], "ascii") + b'\0')
        f.write(bytes(chain["name"], "ascii") + b'\0')
    
    for token in tokens.values():
        addresses = serialize_addresses(token["addresses"])
        token_len = 1 + len(addresses) + 1 + len(token["ticker"]) + 1
        f.write(struct.pack("<HHB", ERC20_MAGIC, token_len, len(token["addresses"])))
        f.write(addresses)
        f.write(struct.pack("B", token["decimals"]))
        f.write(bytes(token["ticker"], "ascii") + b'\0')

def lookup_chain(chains_json, chain_id):
    for chain in chains_json:
        if chain["chainId"] == chain_id:
            return chain
    return None

def process_token(tokens, chains, token_json, chains_json):
    chain_id = token_json["chainId"]

    chain = chains.get(chain_id)
    if chain is None:
        chain_json = lookup_chain(chains_json, chain_id)
        chain = {
            "id": process_token.inner_chain_id,
            "name": chain_json["name"],
            "ticker": chain_json["nativeCurrency"]["symbol"],
            "decimals": chain_json["nativeCurrency"]["decimals"],
            "chainId": chain_id,
        }
        
        process_token.inner_chain_id = process_token.inner_chain_id + 1
        chains[chain_id] = chain

    symbol = token_json["symbol"]
    token = tokens.get(symbol)
    
    if token is None:
        token = {
            "addresses": {},
            "ticker": symbol,
            "decimals": token_json["decimals"]
        }
        tokens[symbol] = token

    token["addresses"][chain["id"]] = token_json["address"]

def main():
    parser = argparse.ArgumentParser(description='Create a database from a token and chain list')
    parser.add_argument('-t', '--token-list', help="the token list json")
    parser.add_argument('-c', '--chain-list', help="the chain list json")
    parser.add_argument('-o', '--output', help="the output file")

    args = parser.parse_args()
    token_list = json.load(open(args.token_list))
    chain_list = json.load(open(args.chain_list))

    tokens = {}
    chains = {}

    for token in token_list["tokens"]:
        process_token(tokens, chains, token, chain_list)
    
    with open(args.output, 'wb') as f:
        serialize_db(f, chains, tokens)

if __name__ == "__main__":
    process_token.inner_chain_id = 0
    main()