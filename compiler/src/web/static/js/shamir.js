/*
    Shamir.js is the file responsible for generating secret shares from a secret and reconstructing a secret from a share.
 */

/* function getShares(secret, peers, threshold, fieldSize) {
    const coefficients = [BigInt(secret)];
    for (let i = 1; i < threshold + 1; i++) {
        coefficients.push(BigInt(Math.floor(Math.random() * Number(fieldSize))));
    }

    const shares = [];
    for (let i = 1; i <= peers; i++) {
        shares.push(_evalAt(coefficients, BigInt(i), fieldSize));
    }

    return shares;
}

function _evalAt(poly, x, prime) {
    let accum = 0n;
    for (let coeff of poly.slice().reverse()) {
        accum *= x;
        accum += coeff;
        accum %= prime;
    }
    return accum;
}
  Note: Wikipedia's approach.
*/


function getShares(secret, peers, threshold, fieldSize, bits) {
    fieldSize = BigInt(fieldSize);  // Ensure fieldSize is BigInt
    secret = BigInt(secret);

    let coefficient = BigInt(0);
    let temp = BigInt(0);
    let sharingMatrix = computeMatrix(peers, threshold, fieldSize);

    let shares = [];
    for (let i = 0; i < peers; i++) {
        shares[i] = BigInt(0);
    }
    if (secret < BigInt(0)) {
        secret = secret % fieldSize;
    }    

    for (let degree = 0; degree < threshold + 1; degree++) {
        if (degree === 0) {
            coefficient = secret;
        } else {
            coefficient = getRandomBigInt(fieldSize, bits);
            if (degree === threshold && coefficient === BigInt(0)) {
                coefficient++;
            }
        }
        for (let peer = 0; peer < peers; peer++){
            temp = modMul(sharingMatrix[peer][degree], coefficient, fieldSize);
            shares[peer] = modAdd(shares[peer], temp, fieldSize);
        }
    }
    return shares;
}

function computeLagrangeWeight(peers, fieldSize) {
    const lagrangeWeight = Array(peers).fill(0n);

    for (let peer = 0; peer < peers; peer++) {
        const point = BigInt(peer) + 1n;
        let nom = 1n;
        let denom = 1n;

        for (let l = 0; l < peers; l++) {
            if (l !== peer) {
                const t1 = BigInt(l) + 1n;
                nom = modMul(nom, t1, fieldSize);

                const t2 = point;
                const temp = modSub(t1, t2, fieldSize);
                denom = modMul(denom, temp, fieldSize);
            }
        }

        const invDenom = modInv(denom, fieldSize);
        lagrangeWeight[peer] = modMul(nom, invDenom, fieldSize);
    }

    return lagrangeWeight;
}

function reconstructSecret(shares, fieldSize) {
    let result = 0n;
    let lagrangeWeight = computeLagrangeWeight(shares.length, fieldSize);

    for (let peer = 0; peer < shares.length; peer++) {
        const tempMul = modMul(shares[peer], lagrangeWeight[peer], fieldSize);
        result = modAdd(result, tempMul, fieldSize);
    }

    if (result*2n > fieldSize) {
        result = result - fieldSize;
    }

    return result;
}

function floatReconstructSecret(y, fieldSize) {
    const size = y[0].length; // assuming all y[peer] have the same length
    const peers = y.length;

    let result = Array(size).fill(0n);

    for (let i = 0; i < size; i++) {
        for (let peer = 0; peer < peers; peer++) {
            const parsed_int = BigInt(y[peer][i]); // parse string to BigInt
            const temp = modMul(parsed_int, lagrangeWeight[peer], fieldSize);
            result[i] = modAdd(result[i], temp, fieldSize);
        }
    }

    // Flip negative (do not optimize)
    for (let k = 0; k < 4; k++) {
        if (k === 1) {
            // deal with negative results
            let tmp = result[1] * 2n;
            if (tmp > fieldSize) {
                result[1] = result[1] - fieldSize;
            }
        }
    }

    // Convert result to number values
    const v = Number(result[0]);
    const p = Number(result[1]);
    const z = Number(result[2]);
    const s = Number(result[3]);

    let element = 0;
    if (z === 1) {
        element = 0;
    } else {
        element = v * Math.pow(2, p);
        if (s === 1)
            element = -element;
    }

    return element;
}


function getRandomBigInt(max, bits) {
    // return BigInt(Math.floor(Math.random() * Number(max)));

    // Calculate how many bytes are needed to store 'bits'
    const byteLength = Math.ceil(bits / 8);

    // Fill the array with cryptographically secure random values
    const buf = new Uint8Array(byteLength);
    crypto.getRandomValues(buf);

    // Convert byte array to a BigInt
    let big_int = 0;
    for (let i = 0; i < buf.length; i++) {
        big_int = BigInt(big_int) * BigInt(256) + BigInt(buf[i]);
    }
    return big_int % BigInt(max);
}

function modMul(x, y, mod) {
    return (BigInt(x) * BigInt(y)) % BigInt(mod);
}

function modAdd(x, y, mod) {
    return (BigInt(x) + BigInt(y)) % BigInt(mod);
}

function modSub(x, y, mod) {
    return (BigInt(x) - BigInt(y) + BigInt(mod)) % BigInt(mod);
}


function modPow(base, exponent, mod) {
    // Exponent is huge so the computation fails
    // M said we need to do mod reduction -> The function used in the c version of this code does this 
    // correctly. So, the js translation is not correct on this portion 
    // return (BigInt(base) ** exponent) % BigInt(mod);

    let result = 1n;
    base = base % mod; // Update base if it is greater than or equal to mod

    while (exponent > 0) {
        if (exponent % BigInt(2) === BigInt(1))
        result = (result * base) % mod;
        exponent = exponent >> BigInt(1); 
        base = (base * base) % mod;
    }
    return result;
}

function modInv(value, fieldSize) {
    value = BigInt(value);
    fieldSize = BigInt(fieldSize);
    const temp = fieldSize - 2n;
    return modPow(value, temp, fieldSize);
}

function computeMatrix(peers, threshold, fieldSize) {
    let sharingMatrix = new Array(peers).fill(null).map(() => new Array(peers).fill(BigInt(0)));

    for (let i = 0; i < peers; i++) {
        for (let j = 0; j < 2 * threshold + 1; j++) {
            let t1 = BigInt(i + 1);
            let t2 = BigInt(j);
            sharingMatrix[i][j] = modPow(t1, t2, fieldSize)
        }
    }
    return sharingMatrix;
}

// const secret = 15;
// const peers = 3;
// const threshold = 1;
// const fieldSize = 67n;

// const shares = getShares(secret, peers, threshold, fieldSize);
// console.log('Shares:', shares);

// const recoveredSecret = reconstructSecret(shares, fieldSize);

// console.log('Recovered Secret:', Number(recoveredSecret));




