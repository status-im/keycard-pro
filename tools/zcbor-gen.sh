zcbor code -c cddl/eip4527.cddl -d -e -t eth-sign-request eth-signature hd-key crypto-multi-accounts --output-c app/ur/eip4527.c --output-h app/ur/eip4527.h
zcbor code -c cddl/ur.cddl -d -e -t ur-part --output-c app/ur/ur_part.c --output-h app/ur/ur_part.h
zcbor code -c cddl/auth.cddl -d -e -t dev-auth --output-c app/ur/auth.c --output-h app/ur/auth.h
zcbor code -c cddl/btc.cddl -d -e -t psbt --output-c app/ur/btc.c --output-h app/ur/btc.h