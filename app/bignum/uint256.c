/*******************************************************************************
 *   Ledger Ethereum App
 *   (c) 2016-2019 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

// Adapted from https://github.com/calccrypto/uint256_t

#include <stdio.h>
#include <string.h>
#include "uint256.h"
#include "uint_common.h"

void convertUint256BE(const uint8_t *const data, uint32_t length, uint256_t *const target) {
    uint8_t tmp[32];

    memset(tmp, 0, sizeof(tmp) - length);
    memmove(tmp + sizeof(tmp) - length, data, length);
    readu256BE(tmp, target);
}

void readu256BE(const uint8_t *const buffer, uint256_t *const target) {
    readu128BE(buffer, &UPPER_P(target));
    readu128BE(buffer + 16, &LOWER_P(target));
}

bool zero256(const uint256_t *const number) {
    return (zero128(&LOWER_P(number)) && zero128(&UPPER_P(number)));
}

void copy256(uint256_t *const target, const uint256_t *const number) {
    copy128(&UPPER_P(target), &UPPER_P(number));
    copy128(&LOWER_P(target), &LOWER_P(number));
}

void clear256(uint256_t *const target) {
    clear128(&UPPER_P(target));
    clear128(&LOWER_P(target));
}

void shiftl256(const uint256_t *const number, uint32_t value, uint256_t *const target) {
    if (value >= 256) {
        clear256(target);
    } else if (value == 128) {
        copy128(&UPPER_P(target), &LOWER_P(number));
        clear128(&LOWER_P(target));
    } else if (value == 0) {
        copy256(target, number);
    } else if (value < 128) {
        uint128_t tmp1;
        uint128_t tmp2;
        uint256_t result;
        shiftl128(&UPPER_P(number), value, &tmp1);
        shiftr128(&LOWER_P(number), (128 - value), &tmp2);
        add128(&tmp1, &tmp2, &UPPER(result));
        shiftl128(&LOWER_P(number), value, &LOWER(result));
        copy256(target, &result);
    } else if ((256 > value) && (value > 128)) {
        shiftl128(&LOWER_P(number), (value - 128), &UPPER_P(target));
        clear128(&LOWER_P(target));
    } else {
        clear256(target);
    }
}

void shiftr256(const uint256_t *const number, uint32_t value, uint256_t *const target) {
    if (value >= 256) {
        clear256(target);
    } else if (value == 128) {
        copy128(&LOWER_P(target), &UPPER_P(number));
        clear128(&UPPER_P(target));
    } else if (value == 0) {
        copy256(target, number);
    } else if (value < 128) {
        uint128_t tmp1;
        uint128_t tmp2;
        uint256_t result;
        shiftr128(&UPPER_P(number), value, &UPPER(result));
        shiftr128(&LOWER_P(number), value, &tmp1);
        shiftl128(&UPPER_P(number), (128 - value), &tmp2);
        add128(&tmp1, &tmp2, &LOWER(result));
        copy256(target, &result);
    } else if ((256 > value) && (value > 128)) {
        shiftr128(&UPPER_P(number), (value - 128), &LOWER_P(target));
        clear128(&UPPER_P(target));
    } else {
        clear256(target);
    }
}

uint32_t bits256(const uint256_t *const number) {
    uint32_t result = 0;
    if (!zero128(&UPPER_P(number))) {
        result = 128;
        uint128_t up;
        copy128(&up, &UPPER_P(number));
        while (!zero128(&up)) {
            shiftr128(&up, 1, &up);
            result++;
        }
    } else {
        uint128_t low;
        copy128(&low, &LOWER_P(number));
        while (!zero128(&low)) {
            shiftr128(&low, 1, &low);
            result++;
        }
    }
    return result;
}

bool equal256(const uint256_t *const number1, const uint256_t *const number2) {
    return (equal128(&UPPER_P(number1), &UPPER_P(number2)) &&
            equal128(&LOWER_P(number1), &LOWER_P(number2)));
}

bool gt256(const uint256_t *const number1, const uint256_t *const number2) {
    if (equal128(&UPPER_P(number1), &UPPER_P(number2))) {
        return gt128(&LOWER_P(number1), &LOWER_P(number2));
    }
    return gt128(&UPPER_P(number1), &UPPER_P(number2));
}

bool gte256(const uint256_t *const number1, const uint256_t *const number2) {
    return gt256(number1, number2) || equal256(number1, number2);
}

void add256(const uint256_t *const number1,
            const uint256_t *const number2,
            uint256_t *const target) {
    uint128_t tmp;
    add128(&UPPER_P(number1), &UPPER_P(number2), &UPPER_P(target));
    add128(&LOWER_P(number1), &LOWER_P(number2), &tmp);
    if (gt128(&LOWER_P(number1), &tmp)) {
        uint128_t one;
        UPPER(one) = 0;
        LOWER(one) = 1;
        add128(&UPPER_P(target), &one, &UPPER_P(target));
    }
    add128(&LOWER_P(number1), &LOWER_P(number2), &LOWER_P(target));
}

void sub256(const uint256_t *const number1,
            const uint256_t *const number2,
            uint256_t *const target) {
    uint128_t tmp;
    sub128(&UPPER_P(number1), &UPPER_P(number2), &UPPER_P(target));
    sub128(&LOWER_P(number1), &LOWER_P(number2), &tmp);
    if (gt128(&tmp, &LOWER_P(number1))) {
        uint128_t one;
        UPPER(one) = 0;
        LOWER(one) = 1;
        sub128(&UPPER_P(target), &one, &UPPER_P(target));
    }
    sub128(&LOWER_P(number1), &LOWER_P(number2), &LOWER_P(target));
}

void or256(const uint256_t *const number1,
           const uint256_t *const number2,
           uint256_t *const target) {
    or128(&UPPER_P(number1), &UPPER_P(number2), &UPPER_P(target));
    or128(&LOWER_P(number1), &LOWER_P(number2), &LOWER_P(target));
}

void mul256(const uint256_t *const number1,
            const uint256_t *const number2,
            uint256_t *const target) {
    uint128_t top[4];
    uint128_t bottom[4];
    uint128_t products[4][4];
    uint128_t tmp, tmp2, fourth64, third64, second64, first64;
    uint256_t target1, target2;
    UPPER(top[0]) = 0;
    LOWER(top[0]) = UPPER(UPPER_P(number1));
    UPPER(top[1]) = 0;
    LOWER(top[1]) = LOWER(UPPER_P(number1));
    UPPER(top[2]) = 0;
    LOWER(top[2]) = UPPER(LOWER_P(number1));
    UPPER(top[3]) = 0;
    LOWER(top[3]) = LOWER(LOWER_P(number1));
    UPPER(bottom[0]) = 0;
    LOWER(bottom[0]) = UPPER(UPPER_P(number2));
    UPPER(bottom[1]) = 0;
    LOWER(bottom[1]) = LOWER(UPPER_P(number2));
    UPPER(bottom[2]) = 0;
    LOWER(bottom[2]) = UPPER(LOWER_P(number2));
    UPPER(bottom[3]) = 0;
    LOWER(bottom[3]) = LOWER(LOWER_P(number2));

    for (int y = 3; y > -1; y--) {
        for (int x = 3; x > -1; x--) {
            mul128(&top[x], &bottom[y], &products[3 - x][y]);
        }
    }

    UPPER(fourth64) = 0;
    LOWER(fourth64) = LOWER(products[0][3]);
    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[0][2]);
    UPPER(tmp2) = 0;
    LOWER(tmp2) = UPPER(products[0][3]);
    add128(&tmp, &tmp2, &third64);
    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[0][1]);
    UPPER(tmp2) = 0;
    LOWER(tmp2) = UPPER(products[0][2]);
    add128(&tmp, &tmp2, &second64);
    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[0][0]);
    UPPER(tmp2) = 0;
    LOWER(tmp2) = UPPER(products[0][1]);
    add128(&tmp, &tmp2, &first64);

    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[1][3]);
    add128(&tmp, &third64, &tmp2);
    copy128(&third64, &tmp2);
    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[1][2]);
    add128(&tmp, &second64, &tmp2);
    UPPER(tmp) = 0;
    LOWER(tmp) = UPPER(products[1][3]);
    add128(&tmp, &tmp2, &second64);
    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[1][1]);
    add128(&tmp, &first64, &tmp2);
    UPPER(tmp) = 0;
    LOWER(tmp) = UPPER(products[1][2]);
    add128(&tmp, &tmp2, &first64);

    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[2][3]);
    add128(&tmp, &second64, &tmp2);
    copy128(&second64, &tmp2);
    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[2][2]);
    add128(&tmp, &first64, &tmp2);
    UPPER(tmp) = 0;
    LOWER(tmp) = UPPER(products[2][3]);
    add128(&tmp, &tmp2, &first64);

    UPPER(tmp) = 0;
    LOWER(tmp) = LOWER(products[3][3]);
    add128(&tmp, &first64, &tmp2);
    copy128(&first64, &tmp2);

    clear256(&target1);
    shiftl128(&first64, 64, &UPPER(target1));
    clear256(&target2);
    UPPER(UPPER(target2)) = UPPER(third64);
    shiftl128(&third64, 64, &LOWER(target2));
    add256(&target1, &target2, target);
    clear256(&target1);
    copy128(&UPPER(target1), &second64);
    add256(&target1, target, &target2);
    clear256(&target1);
    copy128(&LOWER(target1), &fourth64);
    add256(&target1, &target2, target);
}

void divmod256(const uint256_t *const l,
               const uint256_t *const r,
               uint256_t *const retDiv,
               uint256_t *const retMod) {
    uint256_t copyd, adder, resDiv, resMod;
    uint256_t one;
    clear256(&one);
    UPPER(LOWER(one)) = 0;
    LOWER(LOWER(one)) = 1;
    uint32_t diffBits = bits256(l) - bits256(r);
    clear256(&resDiv);
    copy256(&resMod, l);
    if (gt256(r, l)) {
        copy256(retMod, l);
        clear256(retDiv);
    } else {
        shiftl256(r, diffBits, &copyd);
        shiftl256(&one, diffBits, &adder);
        if (gt256(&copyd, &resMod)) {
            shiftr256(&copyd, 1, &copyd);
            shiftr256(&adder, 1, &adder);
        }
        while (gte256(&resMod, r)) {
            if (gte256(&resMod, &copyd)) {
                sub256(&resMod, &copyd, &resMod);
                or256(&resDiv, &adder, &resDiv);
            }
            shiftr256(&copyd, 1, &copyd);
            shiftr256(&adder, 1, &adder);
        }
        copy256(retDiv, &resDiv);
        copy256(retMod, &resMod);
    }
}

uint32_t tostring256(const uint256_t *const number, uint32_t baseParam, uint32_t decimals, char out[UINT256_STRING_LEN]) {
    uint256_t rDiv;
    uint256_t rMod;
    uint256_t base;
    copy256(&rDiv, number);
    clear256(&rMod);
    clear256(&base);
    UPPER(LOWER(base)) = 0;
    LOWER(LOWER(base)) = baseParam;
    uint32_t offset = 0;

    if ((baseParam < 2) || (baseParam > 16)) {
        return false;
    }

    do {
        divmod256(&rDiv, &base, &rDiv, &rMod);
        out[offset++] = HEXDIGITS[(uint8_t) LOWER(LOWER(rMod))];
        if (offset == decimals) {
          out[offset++] = '.';
        }
    } while (!zero256(&rDiv));

    while(offset < decimals) {
      out[offset++] = '0';
    }

    if (offset == decimals) {
      out[offset++] = '.';
    }

    if (offset == (decimals + 1)) {
      out[offset++] = '0';
    }

    out[offset] = '\0';
    reverseString(out, offset);
    return offset;
}
