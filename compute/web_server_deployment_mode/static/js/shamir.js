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

function getShares(secret, peers, threshold, fieldSize) {
    let coefficient = bigInt(0);
    let temp = bigInt(0);
    let sharingMatrix = computeMatrix(peers, threshold, fieldSize);

    let shares = [];
    for (let i = 0; i < peers; i++) {
        shares[i] = bigInt(0);
    }
    if (secret < 0) {
        secret = secret % fieldSize;
    }

    for (let degree = 0; degree < threshold + 1; degree++) {
        if (degree === 0) {
            coefficient = secret;
        } else {
            coefficient = getRandomBigInt(fieldSize);
            if (degree === threshold && coefficient === bigInt(0)) {
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

    return result;
}

function getRandomBigInt(max) {
    return BigInt(Math.floor(Math.random() * Number(max)));
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
    return (BigInt(base) ** BigInt(exponent)) % BigInt(mod);
}

function modInv(value, fieldSize) {
    value = bigInt(value);
    fieldSize = bigInt(fieldSize);
    const temp = fieldSize.minus(2);
    return modPow(value, temp, fieldSize);
}

function computeMatrix(peers, threshold, fieldSize) {
    let sharingMatrix = new Array(peers).fill(null).map(() => new Array(peers).fill(bigInt(0)));

    for (let i = 0; i < peers; i++) {
        for (let j = 0; j < 2 * threshold + 1; j++) {
            let t1 = bigInt(i + 1);
            let t2 = BigInt(j);
            sharingMatrix[i][j] = modPow(t1, t2, fieldSize)
        }
    }
    return sharingMatrix;
}

const secret = 15;
const peers = 3;
const threshold = 1;
const fieldSize = 67n;

const shares = getShares(secret, peers, threshold, fieldSize);
console.log('Shares:', shares);

const recoveredSecret = reconstructSecret(shares, fieldSize);

console.log('Recovered Secret:', Number(recoveredSecret));




