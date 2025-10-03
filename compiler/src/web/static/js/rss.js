
/*
    rss.js is the file responsible for generating secret shares from a secret and reconstructing a secret from a share.
*/

/*
Steps:
    1. Get the secret as input 
    2. Total number of shares -> peers 
    3. for (peers-1) -> generate a random byte by calling the random byte generator function 
        a. Bytes = f/ring size 
    4. Produce the last share for k 
    5. Sum of all the values we produced 
                                        k-2
        Shares[peers - 1] = secret - Σ      shares[i]
                                        i=0
    6. A map that would say which party gets which portion of the share indecies. 
*/

/*
    int totalNumShares; // total number of shares
    int numShares;      // shares PER PARTY
    int peers;          // total number of parties (n)
    int threshold;
    int ring_size;      // Same as the modulus 

    // Auto seeded will call sudo_random generator -> no need for hte key_seed
    // 
*/

/* 
In getRssShares:

    totalNumShares shares are generated.

    These are split among peers according to share_mapping.

    The last share (i.e., splitInput[totalNumShares - 1]) holds the negative sum of the previous shares, and thus the secret is reconstructed by summing all shares.
*/
function getRssShares(input, peers, threshold, ring_size, bits) {

    // C version starts from here inside RSS Class 
    // Since we can't have switch statement here, I defined the structure as a nested object, indexed by the number of peers
    const share_mapping = {
        3: [
            [1, 2], // p1
            [2, 0], // p2
            [0, 1], // p3
        ],
        5: [
            [4, 5, 6, 7, 8, 9],    // p1
            [7, 8, 1, 9, 2, 3],    // p2
            [9, 2, 5, 3, 6, 0],    // p3
            [3, 6, 8, 0, 1, 4],    // p4
            [0, 1, 2, 4, 5, 7],    // p5
        ],
        7: [
            [15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34], // p1
            [25, 26, 27, 5, 28, 29, 6, 30, 7, 8, 31, 32, 9, 33, 10, 11, 34, 12, 13, 14],     // p2
            [31, 32, 9, 19, 33, 10, 20, 11, 21, 1, 34, 12, 22, 13, 23, 2, 14, 24, 3, 4],     // p3
            [34, 12, 22, 28, 13, 23, 29, 2, 6, 16, 14, 24, 30, 3, 7, 17, 4, 8, 18, 0],       // p4
            [14, 24, 30, 33, 3, 7, 10, 17, 20, 26, 4, 8, 11, 18, 21, 27, 0, 1, 5, 15],       // p5
            [4, 8, 11, 13, 18, 21, 23, 27, 29, 32, 0, 1, 2, 5, 6, 9, 15, 16, 19, 25],        // p6
            [0, 1, 2, 3, 5, 6, 7, 9, 10, 12, 15, 16, 17, 19, 20, 22, 25, 26, 28, 31],        // p7
        ]
    };

    let totalNumShares = nCk(peers, threshold);
    let numShares = nCk(peers - 1, threshold);

    // C version starts here from inside getShares function 
    let result = [];

    // Split input into totalNumShares shares
    let splitInput = new Array(totalNumShares).fill(BigInt(0));
    let modulus = BigInt(1) << BigInt(bits); // 2^bits
    for (let i = 0; i < totalNumShares - 1; i++) { // Generate totalNumShares - 1 random shares
        splitInput[i] = getRandomBigInt(modulus, bits); // Used to create random bit
        // we don't need to making sure any bits beyond k are zeroed
    }
    splitInput[Number(BigInt(totalNumShares) - 1n)] = BigInt(input); // Compute the last share

    for (let i = 0; i < totalNumShares - 1; i++) {
        splitInput[totalNumShares - 1] -= splitInput[i];
    }
    splitInput[totalNumShares - 1] = splitInput[totalNumShares - 1] % modulus; // No shift needed in js

    // For testing
    let reconstructed_sec = BigInt(0);
    for (let i = 0; i < totalNumShares; i++) {
        reconstructed_sec = (reconstructed_sec + splitInput[i]) % modulus;
    }
    // reconstructed_sec = reconstructed_sec % modulus;
    console.log(`reconstructed_sec: ${reconstructed_sec}`);
    console.log(`Modulus: ${modulus}`);

    // Distribute shares per party
    for (let i = 0; i < peers; i++) {
        let result_T = [];
        for (let j = 0; j < numShares; j++) {
            let shareIdx = share_mapping[peers][i][j];
            result_T.push(splitInput[shareIdx].toString());
        }

        if (result_T.length === 0) {
            throw new Error("ERROR (getShares): empty share_str, something went very wrong.");
        }

        let shareStr = result_T.join(";");
        result.push(shareStr);
    }

    return result;
}

function splitfunc(str, delim) {
    let saveptr = 0; 
    let token;
    let result = [];

    // While there are tokens in the string (str) -> c code uses strtok_r((char *)str
    while (saveptr < str.length) {
        let next = str.indexOf(delim, saveptr); // Find the next delimiter in the string
        
        if (next === -1) { // No delimiter found, so take the rest of the string as a token
            token = str.substring(saveptr); 
            saveptr = str.length + 1; // Set saveptr beyond the string length to break out of the loop
        } else { // Extract the token from the string between saveptr and the delimiter
            token = str.substring(saveptr, next); 
            saveptr = next + delim.length; // Move saveptr past the delimiter
        }

        result.push(token); // Add the token to the result array
    }

    return result; // Return the array of split strings
}



/**
 * Reconstructs secrets from a vector of shares using a simple summation.
 *
 * 1. string[][] partyShares - A 2D array where each inner array represents a party's shares.
 * Each string in the inner array is a semicolon-separated string
 * of shares for a single secret.
 * Example: [["1;2;3", "4;5;6"], ["7;8;9", "10;11;12"]]
 * Here, there are two parties and two secrets to reconstruct.
 * The format of each share string is "(share_1;share_2;...)".
 *      
 * 2. ring_size - The number of secrets to reconstruct. 
 *      
 * 3. Return value is an array of reconstructed secrets (as BigInts). The length of this
 * array will be equal to the 'size' parameter.
 */

/*
Example: 
Secret is -> 5

party 1: 34;-89
party 2: -89;60 
party 3:  60;34

Side note, the shares should all be unsigned integers, not signed. 
The secret (denoted by x) can be recovered by adding all the unique shares together, i.e.,:

x = 34 + (-89) + 60 => 5

*/

// Based on Alessandro's code -> Not working but also not needed
function reconstructRssSecret(input, peers, threshold, ring_size) { // size is ring size
    let totalNumShares = nCk(peers, threshold);
    let SHIFT = 2 ** ring_size - 1; 
    const share_mapping = {
        3: [
            [1, 2], // p1
            [2, 0], // p2
            [0, 1], // p3
        ],
        5: [
            [4, 5, 6, 7, 8, 9],    // p1
            [7, 8, 1, 9, 2, 3],    // p2
            [9, 2, 5, 3, 6, 0],    // p3
            [3, 6, 8, 0, 1, 4],    // p4
            [0, 1, 2, 4, 5, 7],    // p5
        ],
        7: [
            [15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34], // p1
            [25, 26, 27, 5, 28, 29, 6, 30, 7, 8, 31, 32, 9, 33, 10, 11, 34, 12, 13, 14],     // p2
            [31, 32, 9, 19, 33, 10, 20, 11, 21, 1, 34, 12, 22, 13, 23, 2, 14, 24, 3, 4],     // p3
            [34, 12, 22, 28, 13, 23, 29, 2, 6, 16, 14, 24, 30, 3, 7, 17, 4, 8, 18, 0],       // p4
            [14, 24, 30, 33, 3, 7, 10, 17, 20, 26, 4, 8, 11, 18, 21, 27, 0, 1, 5, 15],       // p5
            [4, 8, 11, 13, 18, 21, 23, 27, 29, 32, 0, 1, 2, 5, 6, 9, 15, 16, 19, 25],        // p6
            [0, 1, 2, 3, 5, 6, 7, 9, 10, 12, 15, 16, 17, 19, 20, 22, 25, 26, 28, 31],        // p7
        ]
    };

    const result = [];
    let shareStrs = [];
    const extracted = Array.from({ length: peers }, () =>
        Array(totalNumShares).fill(0)
    );

    console.log("Initialized extracted matrix:", extracted);

    // p: 0->n
    // i: 0->size
    // j: 0->totalNumShares
    for (let p = 0; p < input.length; p++) { // peer.size() = size
        const peer = input[p];
        console.log(`\nProcessing peer ${p}:`, peer);
        for (let i = 0; i < peer.length; i++) { // peer.size() = size
            console.log(`\nPeer ${i}:`, peer[i], peer[i+1]);
            shareStrs = splitfunc(peer[i], ";"); // There should be a splitfunc used
            for (let j = 0; j < shareStrs.length; j++) {
                // parseInt() is a JavaScript function that parses a string and 
                // returns an integer. It takes two arguments: the string to 
                // parse and an optional radix (base) to use for the conversion. 
                // If the radix is not specified, parseInt() assumes a radix of 10. 

                // The 10 is added to match std::from_chars default parsing in base 10
                const extractedInput = parseInt(shareStrs[j], 10); 
                extracted[i][share_mapping[peers][j]] = extractedInput;
            }
        }
    }
    console.log("\nFinal extracted matrix:", extracted);

    // iterating through all values
    for (const shares of extracted) {
        let accumulator = 0;
        // summing all shares together
        for (const s of shares) {
            accumulator += s;
        }
        accumulator = accumulator & SHIFT; // masking
        // **** ANB: what is the protocol if the result is negative?
        result.push(accumulator); // JavaScript handles signed integers natively
    }

    return result;
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


function nCk(n, k) {
    if (k > n) {
        throw new Error("ERROR (nCk): n must be >= k");
    }

    let res = 1;

    // Take advantage of symmetry: C(n, k) == C(n, n - k) -> Optimize calculation by reducing k
    if (k > n - k) {
        k = n - k;
    }

    // Calculate binomial coefficient
    for (let i = 0; i < k; ++i) {
        res *= (n - i);
        res /= (i + 1);
    }

    return res;
}