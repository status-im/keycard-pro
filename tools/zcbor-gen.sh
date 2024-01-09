zcbor code -c cddl/eip4527.cddl -e -t eth-signature hd-key crypto-multi-accounts --output-c app/ur/eip4527_encode.c --output-h app/ur/eip4527_encode.h
zcbor code -c cddl/eip4527.cddl -d -t eth-sign-request --output-c app/ur/eip4527_decode.c --output-h app/ur/eip4527_decode.h
