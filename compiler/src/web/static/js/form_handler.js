 
// This code does the following:
    // Fetches config data from /get-config.
    // Sets up file upload and CSV parsing.
    // Validates bit lengths for integers based on config.
    // Validates matrix dimensions.
    // Prepares and submits form data.
    // Generate the shares on the client side.
    // Encrypt the generated shares by multiplying it by 5 in the client side before sending it back to the server. 


const REPLICATED_SS = 1;
const SHAMIR_SS = 2;

// Get peers number of keys 
async function getAllPublicKeys(peers) {
    const keys = [];
    for (let i = 1; i <= peers; i++) {
        const pemKey = document.getElementById(`public_key_${i}`).value;
        keys.push(pemKey);
    }
    return keys;
}

function pemToDer(pem) {
    const lines = pem.replace(/-----(BEGIN|END)[\w\s]+-----/g, '').split('\n'); // remove the start and end of it
    const base64 = lines.join(''); // Joins Lines
    const binary = atob(base64); // Decodes Base64 to a binarry string
    const derArray = new Uint8Array(binary.length); // Creates Uint8Array array
    for (let i = 0; i < binary.length; i++) { // iterates through the binary string and sets each byte of the Uint8Array to the character code of the corresponding character in the binary string.
      derArray[i] = binary.charCodeAt(i);
    }
    return derArray.buffer; // Return the ArrayBuffer
  }

document.addEventListener("DOMContentLoaded", async function () {

    // Get the configdata from the server - this is the json data written by picco
    let configData = null;

    // Function to fetch config data once
    async function fetchConfig() {
        try {
            const response = await fetch("/get-config");
            if (!response.ok) {
                throw new Error("Failed to fetch config");
            }
            return await response.json();
        } catch (error) {
            console.error("Error fetching config:", error);
            return null;
        }
    }
    
    // Fetch config on load
    configData = await fetchConfig();
    if (!configData) {
        alert("Error: Could not load configuration.");
        return;
    }

    // Accessing the variables from the JSON file
    const form = document.getElementById("dynamic-form");

    // Accessing the settings from the JSON file
    const technique = Number(document.getElementById('technique_1').value);
    const bits = Number(document.getElementById('bits_1').value);
    const peers = Number(document.getElementById('peers_1').value);
    const threshold = Number(document.getElementById('threshold_1').value);
    const inputs = Number(document.getElementById('inputs_1').value);
    const outputs = Number(document.getElementById('outputs_1').value);
    const fieldSize = BigInt(document.getElementById('modulus_1').value); // fieldSize is the modulus -> Needed for shamir only 
    let ring_size = bits; // The ring size is the bit size! -> Needed for RSS only
    const public_keys = await getAllPublicKeys(peers); // Accessing the public keys from the JSON file 

    // Print for testing purpose
    // console.log("Technique:", technique); 
    // console.log("Bits:", bits); 
    // console.log("Peers:", peers); 
    // console.log("Threshold:", threshold); 
    // console.log("Inputs:", inputs); 
    // console.log("Outputs:", outputs); 
    // console.log("Field Size (Modulus):", fieldSize);
    // console.log("Ring Size :", ring_size);
    const key_inputs = document.querySelectorAll("input[id^='public_key_']");
    // Print for testing purpose
    // key_inputs.forEach((input, index) => {
    //     console.log(`Public Key for Peer ${index + 1}:`);
    //     console.log(input.value);
    // });

    const previewDiv = document.createElement("div");
    previewDiv.id = "preview";
    form.after(previewDiv); // Adds preview section after the form

    // Function to handle file uploads and parse CSV data
    function handleFileUpload(event) {
        const file = event.target.files[0];
        if (!file) return;

        const reader = new FileReader();
        reader.onload = function (e) {
            const csvData = e.target.result;
            parseCSV(file.name, csvData, event.target.name);
        };
        reader.readAsText(file);
    }

    // Function to parse CSV file contents into JSON format to display on the screen (used for testing)
    function parseCSV(filename, csvData, fieldName) {
        const rows = csvData
            .trim()
            .split("\n")
            .map(row => row.split(",").map(cell => cell.replace(/"/g, '').trim())) // Clean up each cell
            .filter(row => row.length > 0 && row.some(cell => cell !== "")); // Remove empty rows
    
        document.querySelector(`[name="${fieldName}"]`).dataset.csvContent = JSON.stringify(rows);
    
        // Display CSV preview in the UI
        const preview = document.getElementById("preview");
        preview.innerHTML += `<h3>CSV Preview: ${filename}</h3><pre>${JSON.stringify(rows, null, 2)}</pre>`;
    }


    // Function to format the submitted data 
    function formatSubmittedData(data) {
        let outputHtml = "";
        Object.entries(data).forEach(([key, value]) => {
            let numbers = [];
            let variable = configData?.variables?.find(v => v.name === key);
            let varName = variable?.name || key; // Extract actual variable name
            let dimension = Number(variable?.dimension ?? 0); // Default dimension to 0 if not found
            let variableType = variable?.type; // <-- Get the type (int or float)
    
            if (Array.isArray(value)) {
                value.forEach((item, rowIndex) => {
                    if (typeof item === "object" && item !== null) { // This likely handles rows from CSV
                        let row = [];
                        Object.values(item).forEach((val, colIndex) => {
                            let processedValue;
                            if (variableType === 'float') {
                                // Allow digits and a single decimal point for floats
                                // This regex matches digits, and optionally a decimal point followed by digits.
                                // It also handles negative numbers if they might appear.
                                processedValue = String(val).match(/^-?\d*\.?\d*$/)?.[0];
                                if (processedValue === undefined || processedValue === null) {
                                    // Fallback or error handling for invalid float format
                                    console.warn(`Warning: Invalid float format detected for ${varName} at [${rowIndex}][${colIndex}]: "${val}". Storing as is or handling error.`);
                                    processedValue = val; // Store original or handle as an error
                                }
                            } else {
                                // For integers, strictly remove non-digit characters
                                processedValue = String(val).replace(/[^0-9-]/g, ""); // Allow digits and negative sign
                            }
                            
                            if (processedValue) {
                                if (dimension === 2) {
                                    row.push(`${varName}[${rowIndex}][${colIndex}] = ${processedValue}`);
                                } else { // dimension === 1
                                    row.push(`${varName}[${rowIndex}] = ${processedValue}`);
                                }
                            }
                        });
                        if (row.length) numbers.push(...row);
                    } else { // Handles 1D arrays that might be directly passed as array of simple values
                        // This block might need refinement based on how your 1D arrays are actually structured in 'value'
                        // from the CSV reader. Assuming 'value' is an array of strings like ["0.1", "3", "4.555"]
                         let processedValue;
                         if (variableType === 'float') {
                             processedValue = String(item).match(/^-?\d*\.?\d*$/)?.[0];
                             if (processedValue === undefined || processedValue === null) {
                                 console.warn(`Warning: Invalid float format detected for ${varName} at index ${rowIndex}: "${item}". Storing as is or handling error.`);
                                 processedValue = item;
                             }
                         } else {
                             processedValue = String(item).replace(/[^0-9-]/g, "");
                         }
                         if (processedValue) {
                             numbers.push(`${varName}[${rowIndex}] = ${processedValue}`);
                         }
                    }
                });
            } else if (typeof value === "string" || typeof value === "number") { // Single value (dimension 0)
                let processedValue;
                if (variableType === 'float') {
                    processedValue = String(value).match(/^-?\d*\.?\d*$/)?.[0];
                    if (processedValue === undefined || processedValue === null) {
                        console.warn(`Warning: Invalid float format detected for ${varName}: "${value}". Storing as is or handling error.`);
                        processedValue = value;
                    }
                } else {
                    processedValue = String(value).replace(/[^0-9-]/g, "");
                }
                if (processedValue) numbers.push(`${key} = ${processedValue}`);
            }
    
            if (numbers.length > 0) {
                outputHtml += `<strong>${key}:</strong><br>`;
                outputHtml += `${numbers.join(", ")}<br><br>`;
            }
        });
    
        return outputHtml.trim();
    }

    /*
        aesKey (16 bytes): This is the raw 128-bit AES key.
        iv (12 bytes): Explicitly forge.random.getBytesSync(12).
        tag (16 bytes): tagLength: 128 bits, which is 128 / 8 = 16 bytes.
        encryptedKey: This is the aesKey (16 bytes) encrypted using RSA-OAEP. 
                The size of this output depends directly on your RSA public key's modulus size.
    */
    async function encrypt_share(shareBuffer, publicKeyPem) {
        // Step 1: Generate AES key (128-bit)
        const aesKey = forge.random.getBytesSync(16); // 128-bit key
    
        // Step 2: Generate random IV (12 bytes for AES-GCM)
        const iv = forge.random.getBytesSync(12);
    
        // Step 3: Encrypt the share using AES-GCM
        const cipher = forge.cipher.createCipher('AES-GCM', aesKey);
        cipher.start({ iv: iv, tagLength: 128 });
        cipher.update(forge.util.createBuffer(shareBuffer, 'utf8'));
        cipher.finish();
    
        const ciphertext = cipher.output.getBytes();
        const tag = cipher.mode.tag.getBytes();
    
        // Step 4: Encrypt AES key using RSA-OAEP with SHA-256
        let publicKey;
        try {
            publicKey = forge.pki.publicKeyFromPem(publicKeyPem);
        } catch (e) {
            console.error("Invalid public key:", e);
            return null;
        }
    
        const encryptedKey = publicKey.encrypt(aesKey, 'RSA-OAEP', {
            md: forge.md.sha256.create()
        });
    
        // Step 5: Combine IV + ciphertext + tag, encode all as base64
        const combinedBytes = iv + ciphertext + tag;
    
        // Step 6: Concatenate the RSA-encrypted AES Key and the combined AES-GCM parts
        // Order: [RSA-encrypted AES Key] + [IV + Ciphertext + Tag]
        const fullEncryptedBinaryBlob = encryptedKey + combinedBytes;

        // Step 7: Base64 encode the *entire* concatenated binary blob
        const base64EncodedBlob = forge.util.encode64(fullEncryptedBinaryBlob);

        // Return the single Base64 string
        return base64EncodedBlob;
    }
    

    // Helper function to convert ArrayBuffer to Base64 -> for testing only
    function arrayBufferToBase64(buffer) {
        const bytes = new Uint8Array(buffer);
        const binary = bytes.reduce((acc, byte) => acc + String.fromCharCode(byte), "");
        return btoa(binary);
    }


    // It extracts the number from the formData (formattedData) instead of the DOM element
    function getSecretValue(formData) {
        // Iterate through all keys in formData
        for (let key in formData) {
            const value = formData[key];
    
            // Check if the value is a string 
            if (typeof value === "string") {
                // Try to match the 'key = value' format
                const match = value.match(/(\w+)\s*=\s*(\d+)/); // Match 'key = value'
                if (match) {
                    return BigInt(match[2]); // Extract the number after '=' and convert to BigInt
                }
    
                // If no match for 'key = value', check if the value is just a number
                const num = value.trim();
                if (!isNaN(num)) {
                    return num; // Return the value as BigInt
                }
            }
        }
    
        // If no valid number found, log an error and return null
        console.error("Failed to extract valid secret value from formData");
        return null;
    }
    
    
    /**
     * I tested this in a separate file with a main function but the actual test should be done while running an actual example
        * I replaced the C float type with JavaScript's Float32Array or DataView to handle the 32-bit floating-point value.
        * The bit manipulation is done using >>> (unsigned right shift) and & (bitwise AND) to extract the sign, exponent, and mantissa.
        * The Math.pow() function is used to handle powers of two, similar to the pow function in C.
        * The function returns an object with the components: significand, exponent, zeroFlag, and sign.
     *
     * The integers that will be stored in elements are as follows:
     * 1. Significand part (significand)
     * 2. Exponent (p)
     * 3. Flag indicating zero (z)
     * 4. Sign (s)
     */
    function convertFloat(value, K, L) {
        const buffer = new ArrayBuffer(4);
        const view = new DataView(buffer);
        view.setFloat32(0, value);

        const rawValue = view.getUint32(0);
        let s = rawValue >>> 31; // Extract the sign bit
        let e = rawValue & 0x7F800000; // Extract the exponent
        e >>= 23;
        let m = rawValue & 0x007FFFFF; // Extract the significand (mantissa)

        let z;
        let v, p, k;
        let significand = 0, one = 1, two = 2, tmp = 0, tmpm = 0;

        if (e === 0 && m === 0) {
            s = 0;
            z = 1;
            significand = 0;
            p = 0;
        } else {
            z = 0;
            if (L < 8) {
                k = (1 << L) - 1; // Raise two to the power of L using shifting and subtract 1, then store it to k
                if (e - 127 - K + 1 > k) {
                    p = k;
                    significand = one << K;        // Raise one to the power of K and store it to significand
                    significand = significand - 1; // Subtract 1
                } else if (e - 127 - K + 1 < -k) {
                    p = -k;
                    significand = 1; // Set the value of significand to 1
                } else {
                    p = e - 127 - K + 1;
                    m = m + (1 << 23);
                    tmpm = m; // Set the value of tmpm to m
                    if (K < 24) {
                        try {
                            tmp = Math.pow(two, (24 - K)); // Raise two to the power of (24 - K) using shifting and store it to tmp
                        } catch (e) {
                            throw new Error("An exception occurred during pow operation: " + e.message);
                        }
                        if (tmp === 0) { // Division by zero check
                            throw new Error("Division by zero: overflow in significand calculation");
                        }
                        significand = tmpm / tmp; // Perform division of tmpm by tmp and store it to significand
                    } else {
                        significand = tmpm * Math.pow(two, (K - 24)); // Raise tmpm to the power of (K - 24) and store it to significand
                    }
                }
            } else {
                p = e - 127 - K + 1;
                m = m + (1 << 23);
                tmpm = m; // Set the value of tmpm to m
                if (K < 24) {
                    try {
                        tmp = Math.pow(two, (24 - K)); // Raise two to the power of (24 - K) and store it to tmp
                    } catch (e) {
                        throw new Error("An exception occurred during pow operation: " + e.message);
                    }
                    if (tmp === 0) { // Division by zero check
                        throw new Error("Division by zero: overflow in significand calculation");
                    }
                    significand = tmpm / tmp; // Perform division of tmpm by tmp and store it to significand
                } else {
                    significand = tmpm; // Set significand to tmpm
                    significand = significand * Math.pow(two, (K - 24)); // Raise significand to the power of (K - 24) and store it to significand
                }
            }
        }

        return {
            significand: significand,
            exponent: p,
            zeroFlag: z,
            sign: s
        };
    }

    // Function to validate bit length of entered values (scalar or array) - used for integers only
    async function validateArrayBitLength(varName, inputValue) {
        const errors = {};
        const warnings = {};

        console.log(`--- Starting validation for variable: ${varName} ---`);
        console.log(`Number to validate:`, inputValue);

        const variable = configData.variables.find(v => v.name === varName);
        if (!variable || variable.type !== "int") {
            return true; // Allow continuation since it's not an integer field
        }

        const expectedBitLength = variable.bit_len1;

        // Normalize input to array
        const values = Array.isArray(inputValue)
            ? inputValue
            : typeof inputValue === "string" && inputValue.includes(",")
                ? inputValue.split(",").map(s => s.trim())
                : [inputValue];

        console.log(`valuesvaluesvaluesvaluesvaluesvalues:`, values);
        
        // Validate each element
        values.forEach((val, idx) => {
            if (isNaN(val)) {
                errors[idx] = `❌ Value '${val}' at index ${idx} is not a valid number.`;
                return;
            }

            const absNum = Math.abs(Number(val));
            const actualBitLength = absNum.toString(2).length;

            if (actualBitLength > 64) {
                errors[idx] = `❌ The provided '${val}' at index ${idx} exceeds 64 bits and cannot be accepted. If you believe this is correct, please contact the organizers.`;
            } else if (actualBitLength > expectedBitLength) {
                warnings[idx] = `⚠️ The provided '${val}' at index ${idx} requires ${actualBitLength} bits, which is more than the expected ${expectedBitLength}-bit integer. Are you sure this is correct? If yes, click "OK" and ignore the warning! If no, re-enter your inputs!`;
            }

        });

        // Clear previous messages
        document.querySelectorAll(".error-message, .warning-message").forEach(el => el.remove());

        // Display messages
        values.forEach((val, idx) => {
            const fieldName = varName + (values.length > 1 ? `[${idx}]` : "");
            const inputField = document.querySelector(`[name="${fieldName}"]`) || document.querySelector(`[name="${varName}"]`);
        
            if (errors[idx]) {
                if (!inputField) {
                    // Alert the error since it cannot be shown in the DOM
                    alert(errors[idx]);
                    console.warn(`(Alerted) ${errors[idx]}`);
                } else {
                    const errorSpan = document.createElement("span");
                    errorSpan.className = "error-message";
                    errorSpan.style.color = "red";
                    errorSpan.style.fontSize = "0.9em";
                    errorSpan.style.display = "block";
                    errorSpan.textContent = errors[idx];
                    inputField.parentNode.appendChild(errorSpan);
                }
            } else if (warnings[idx]) {
                if (!inputField) {
                    alert(`Warning: ${warnings[idx]}`);
                    console.warn(`Warning: ${warnings[idx]}`);
                } else {
                    const warnSpan = document.createElement("span");
                    warnSpan.className = "warning-message";
                    warnSpan.style.color = "orange";
                    warnSpan.style.fontSize = "0.9em";
                    warnSpan.style.display = "block";
                    warnSpan.textContent = warnings[idx];
                    inputField.parentNode.appendChild(warnSpan);
                }
            }
        });

        if (Object.keys(errors).length > 0) {
            return false; // Block further steps
        }

        return true; // Allow continuation
    }

    // Function to validate bit length of entered values - used for integers only
    async function validateSingleVaritLength(varName, number) {

        // Find the variable definition in config
        const variable = configData.variables.find(v => v.name === varName);

        if (!variable) {
            return true;
        }

        const expectedBitLength = variable.bit_len1;
        let actualBitLength = 0;

        // Check for non-numeric input
        if (isNaN(number)) {
            alert(`❌ '${varName}' must be a valid number.`);
            return false;
        } else {
            // Compute actual bit length (handle negative values too)
            const absNumber = Math.abs(Number(number));
            actualBitLength = absNumber.toString(2).length;

            if (actualBitLength > 64) {
                alert(`❌ The provided value (${number}) exceeds 64 bits and cannot be accepted. If you believe this is correct, please contact the organizers.`);
                return false;
            } else if (actualBitLength > expectedBitLength) {
                alert(`⚠️ The provided value (${number}) requires ${actualBitLength} bits, which is more than the expected ${expectedBitLength}-bit integer. Are you sure this is correct?`);
                return true;
            }
        }
        return true; 
    }


    // Function to validate array and matrix size
    // async function validateMatrixSize(formData) {
    //     let errors = {}; // Store validation error messages for each field
    //     console.log("Starting validation...");
    
    //     for (const [key, value] of Object.entries(formData)) {
    //         console.log(`Validating field: ${key}`);
            
    //         // Find the variable in configData.variables array
    //         let variable = configData.variables.find(v => v.name === key);
    //         console.log(`Found variable:`, variable);
    
    //         // Validate only if the variable is a matrix (dimension === 2)
    //         if (variable && variable.dimension === 2) {
    //             const expectedRows = variable.size1;
    //             const expectedCols = variable.size2;
    //             console.log(`Expected matrix size for ${key}: ${expectedRows}x${expectedCols}`);
    
    //             // Check if the value is an array (matrix)
    //             if (!Array.isArray(value)) {
    //                 console.log(`❌ ${key} is not an array`);
    //             }
    //             if (value.length !== expectedRows) {
    //                 console.log(`❌ ${key} has incorrect number of rows: expected ${expectedRows}, got ${value.length}`);
    //             }
    //             if (value.some(row => !Array.isArray(row) || row.length !== expectedCols)) {
    //                 console.log(`❌ ${key} has rows with incorrect number of columns.`);
    //             }
    
    //             // If the matrix is invalid, add an error
    //             if (!Array.isArray(value) || value.length !== expectedRows || value.some(row => !Array.isArray(row) || row.length !== expectedCols)) {
    //                 errors[key] = `❌ The matrix size for ${key} must be ${expectedRows}x${expectedCols}.`;
    //             }
    //         }
    //     }
    
    //     console.log("Validation completed, errors found:", errors);
    
    //     // Update the UI with error messages
    //     document.querySelectorAll(".error-message").forEach(el => el.remove()); // Clear previous errors
    //     Object.entries(errors).forEach(([key, message]) => {
    //         console.log(`Displaying error for field: ${key}`);
    
    //         let inputField = document.querySelector(`[name="${key}"]`);
    //         if (inputField) {
    //             let errorSpan = document.createElement("span");
    //             errorSpan.className = "error-message";
    //             errorSpan.style.color = "red";
    //             errorSpan.style.fontSize = "0.9em";
    //             errorSpan.style.display = "block";
    //             errorSpan.textContent = message;
    //             inputField.parentNode.appendChild(errorSpan);
    //         }
    //     });
    
    //     const noErrors = Object.keys(errors).length === 0;
    //     console.log(`Validation result: ${noErrors ? "Passed" : "Failed"}`);
    
    //     return noErrors; // Return true if no errors
    // }
    
    // Function to validate array and matrix size and element types
    async function validateMatrixSize(formData, configData, secret) {
        let errors = []; // Store all validation error messages

        for (const [key, value] of Object.entries(formData)) {

            let variable = configData.variables.find(v => v.name === key);

            if (variable && variable.dimension >= 1) {
                const expectedRows = variable.size1;
                const expectedCols = variable.size2;
                const expectedType = variable.type;
                // console.log(`Type ${expectedType}`);

                if (!Array.isArray(value)) {
                    errors.push(`❌ ${key} must be an array. Please check your CSV file and resubmit.`);
                    continue;
                }

                if (variable.dimension === 1) {
                    console.log(`expectedRows ${expectedRows}, expectedCols ${expectedCols}, expectedType ${expectedType}, secret.length ${secret.length}`);
                    if (value.length > expectedRows) { // Check based on the row, since the data will be stored in rows for a 1D array
                        errors.push(`❌ Array ${key} has an incorrect number of rows. Expected an array of ${expectedRows} rows, but found ${value.length}. Please correct the number of elements in your CSV file and resubmit.`);
                    }
                    if (variable.type == "int") {
                        value.forEach((element, index) => {
                            const elementStr = String(element).trim(); // Trim for internal checks
                            if (elementStr === "") {
                                errors.push(`❌ ${key}[${index}] has an empty value. Please provide an integer in your CSV file and resubmit.`);
                            } else if (isNaN(parseInt(elementStr, 10)) || elementStr.includes('.') || elementStr.toLowerCase().includes('e')) {
                                errors.push(`❌ ${key}[${index}] must be an integer, but found '${String(element).replace(/,+$/, '')}'. Please correct the data type in your CSV file and resubmit.`);
                            }
                        });
                    }
                } else if (variable.dimension === 2) {
                    if (value.length !== expectedRows) { // The data will be stored both in rows and cols
                        errors.push(`❌ Array ${key} has an incorrect number of rows. Expected ${expectedRows} rows, but found ${value.length}. Please correct the number of rows in your CSV file and resubmit.`);
                    } else {
                        value.forEach((row, rowIndex) => {
                            if (!Array.isArray(row) || row.length !== expectedCols) {
                                errors.push(`❌ ${key}[${rowIndex}] has an incorrect number of columns. Expected ${expectedCols} columns. Please correct the number of columns in your CSV file and resubmit.`);
                            } else {
                                if (variable.type == "int") {
                                    row.forEach((element, colIndex) => {
                                        const elementStr = String(element).trim(); // Trim for internal checks
                                        if (elementStr === "") {
                                            errors.push(`❌ ${key}[${rowIndex}][${colIndex}] has an empty value. Please provide an integer in your CSV file and resubmit.`);
                                        } else if (isNaN(parseInt(elementStr, 10)) || elementStr.includes('.') || elementStr.toLowerCase().includes('e')) {
                                            errors.push(`❌ ${key}[${rowIndex}][${colIndex}] must be an integer, but found '${String(element).replace(/,+$/, '')}'. Please correct the data type in your CSV file and resubmit.`);
                                        }
                                    });
                                }
                            }
                        });
                    }
                }
            }
        }

        if (errors.length > 0) {
            alert(`Validation Errors:\n${errors.join('\n')}\n`);
            return false; // Indicate validation failure
        }

        return true; // Indicate validation success
    }

    function containsFloat(formData) {
        return Object.keys(formData).some(key => {
            const variable = configData.variables.find(v => v.name === key);
            return variable && variable.type === "float";
        });
    }
    
    function submitEncryptedSharesToServer(encryptedData) {
        fetch('/your-server-endpoint', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ shares: encryptedData })
        })
        .then(response => response.json())
        .then(data => {
            alert('Encrypted shares successfully submitted!');
        })
        .catch(error => {
            console.error('Error submitting shares:', error);
            alert('Failed to submit encrypted shares.');
        });
    }

    async function handleFloatInput(formData, varName, secret, size1, size2, technique, bits, peers, threshold, fieldSize, ring_size) {

        // Step 1: Convert the float into a 4-element array
        let convertedObj = convertFloat(parseFloat(secret), size1, size2);
    
        // Convert object to array in a specific order
        let convertedSecrets = [
            convertedObj.significand,
            convertedObj.exponent,
            convertedObj.zeroFlag,
            convertedObj.sign
        ];

        // This will store shares for each component (4 arrays of length `peers`)
        let all_shares = [];
        for (let i = 0; i < 4; i++) {
            // Step 1: Get correct float format
            let component = convertedSecrets[i];
    
            // Step 2: Generate shares
            let shares;
            if (technique == SHAMIR_SS) {
                shares = getShares(component, peers, threshold, fieldSize, bits);
            } else if (technique == REPLICATED_SS) {
                shares = getRssShares(component, peers, threshold, ring_size, bits);
            }

            // Store the value for each in the array 
            all_shares.push(shares);  // 4 arrays of shares
        }

        // Step 3: Return the total_shares -> I get 1 float value -> convert it to 4 values -> get 4 shares for each 4 values -> return and array that has peers number of values for each number  
        // Reorganize shares by peer: result[peerIndex] = [a1, b1, c1, d1]
        let peer_shares = Array.from({ length: peers }, () => []);

        for (let i = 0; i < peers; i++) {
            for (let j = 0; j < 4; j++) {
                peer_shares[i].push(all_shares[j][i]);
            }
        }
        return peer_shares;
    }
    
    async function handleIntInput(formData, varName, secret, technique, bits, peers, threshold, fieldSize, ring_size) {

        let shares;
            
        // Step 1: Getting the shares 
        if (technique == SHAMIR_SS) { // Generate shares -> Shamir
            shares = getShares(secret, peers, threshold, fieldSize, bits);
        } else if (technique == REPLICATED_SS) { // Generate shares -> RSS
            shares = getRssShares(secret, peers, threshold, ring_size, bits); // fieldsize is send for the ring size
        }
        
        // Step 2: Return the shares
        return shares;
    }

    form.addEventListener("submit", async function (event) {
        event.preventDefault();
        let error_detected = false;

        let formData = {};
        // Populate formData from the form inputs
        new FormData(event.target).forEach((value, key) => {
            let inputElement = document.querySelector(`[name="${key}"]`);

            // Handle file inputs (assuming CSV content is stored in dataset)
            if (inputElement && inputElement.type === "file") {
                let actualKey = key.startsWith("file_") ? key.substring(5) : key; // Remove "file_" prefix only for file inputs
                formData[actualKey] = inputElement.dataset.csvContent ? JSON.parse(inputElement.dataset.csvContent) : null;
            } else {
                formData[key] = value; // Keep original key for non-file inputs
            }
        });


        // If validations pass, format the data for display
        let formattedData = formatSubmittedData(formData);
        previewDiv.innerHTML = `<h2> New Submitted Data</h2>${formattedData}`;

        // This array will store all the raw shares for each peer.
        // all_shares_per_party[peerIndex] will be an array of share blocks, where each share block is either
        // [single_int_share] or [float_comp1, float_comp2, float_comp3, float_comp4].
        let all_shares_per_party = Array.from({ length: peers }, () => []);

        // This array will store the order of original variable names (and their types/dimensions)
        // as their shares are generated and pushed into `all_shares_per_party`.
        // This is crucial for reconstructing the final string in the desired format (e.g., "a=shares").
        const processedVariableOrder = [];

        // Read the configData to get variable information (name, type, dimension, etc.)
        const variables = configData?.variables || [];
        const extractedInfo = variables.map(variable => {
            return {
                name: variable.name,
                type: variable.type,
                dimension: variable.dimension,
                bit_len1: variable.bit_len1,
                bit_len2: variable.bit_len2,
                size1: variable.size1,
                size2: variable.size2,
                inputParty: variable.input_party
            };
        });

        // // Print for testing purpose
        // console.log("Extracted Info (from configData):", extractedInfo);
        // // Iterate over extractedInfo array and log the properties
        // extractedInfo.forEach(info => {
        //     console.log(`Name: ${info.name}`);
        //     console.log(`Type: ${info.type}`);
        //     dimension = info.dimension;
        //     console.log(`Dimension: ${info.dimension}`);
        //     console.log(`Bit Length 1: ${info.bit_len1}`);
        //     console.log(`Bit Length 2: ${info.bit_len2}`);
        //     console.log(`Size1: ${info.size1}`);
        //     console.log(`Size2: ${info.size2}`);
        //     console.log(`Input Party: ${info.inputParty}`);
        // });

        const varMetadataMap = new Map();

        // Process each variable from the formData
        for (let [varName, secret] of Object.entries(formData)) {

            // Config data lookup for getting the bit_length and dimension
            const variableInfo = extractedInfo.find(info => info.name === varName);
            const dimension = variableInfo.dimension;
            const ftype = variableInfo.type;

            if (!varMetadataMap.has(varName)) {
                varMetadataMap.set(varName, {
                    type: variableInfo.type,
                    dimension: variableInfo.dimension,
                    bit_len1: variableInfo.bit_len1,
                    bit_len2: variableInfo.bit_len2,
                    size1: variableInfo.size1,
                    size2: variableInfo.size2,
                    inputParty: variableInfo.inputParty
                });
            }

            console.log(`varName = ${varName}, secret = ${secret}, dimension = ${dimension}, ftype = ${ftype}`);
            if (dimension == 0) { // Non-array (single value)
                if (ftype == 'float') { // FLOAT INPUT
                    console.log(`Handled as float: varName = ${varName}, secret = ${secret}, dimension = ${dimension}, ftype = ${ftype}`);
                    // Generate shares for the float input
                    let float_peer_shares = await handleFloatInput(formData, varName, secret, variableInfo.bit_len1, variableInfo.bit_len2, technique, bits, peers, threshold, fieldSize, ring_size);
                    // Store the array of 4 float components for each peer
                    for (let i = 0; i < peers; i++) {
                        all_shares_per_party[i].push(float_peer_shares[i]);  // pushes [a, b, c, d] per peer
                    }
                    // Record this variable in the processing order
                    processedVariableOrder.push({ name: varName, type: ftype, dimension: 0 });
                } else { // INT INPUT
                    console.log(`Handled as int: varName = ${varName}, secret = ${secret}, dimension = ${dimension}, ftype = ${ftype}`);
                    // Validate bit length before proceeding
                    const isBitLengthValid = await validateSingleVaritLength(varName, secret);
                    if (isBitLengthValid == true) {
                        // Generate shares for the int input
                        let int_peer_shares = await handleIntInput(formData, varName, secret, technique, bits, peers, threshold, fieldSize, ring_size);
                        // Store the single int share wrapped in an array for consistency
                        for (let i = 0; i < peers; i++) {
                            all_shares_per_party[i].push([int_peer_shares[i]]); // wrap each int in an array
                        }
                        // Record this variable in the processing order
                        processedVariableOrder.push({ name: varName, type: ftype, dimension: 0 });
                    } else { // Stop all the processes after this
                        error_detected = true; 
                    }
                }
            } else { // Array processing (1D or 2D)
                if (error_detected == false) {
                    // Validate matrix size
                    const isMatrixSizeValid = await validateMatrixSize(formData, configData, secret);
                    if (!isMatrixSizeValid) {
                        error_detected = true; // Stop processing if validation fails
                    } else {
                        if (dimension == 1) { // 1D ARRAY
                            if (ftype === "float") { // 1D ARRAY OF FLOAT
                                console.log(`Handled as float 1D array: varName = ${varName}, secret = ${secret}, dimension = ${dimension}, ftype = ${ftype}`);
                                for (let i = 0; i < variableInfo.size1; i++) {
                                    let value = secret[i];
                                    console.log(`value = ${value}`);

                                    if (('' + value).includes(',')) { // Check if it's not a number or contains a comma (indicating multiple values)
                                        alert(`❌ Variable '${varName}' is a 1-dimensional array. Expected ${variableInfo.size1} values, provided as a single column in a CSV file.`);
                                        error_detected = true;
                                        break; // Stop processing this array if an invalid value is found
                                    }

                                    if (value === undefined || value === null) {
                                        alert(`❌ Empty value provided. Variable '${varName}' is a 1-dimensional array. Expected ${variableInfo.size1} values, provided as a single column in a CSV file.`);
                                        error_detected = true;
                                        break; // Stop processing this array if an invalid value is found
                                    }
                                    
                                    if (isNaN(Number(value))) {
                                        alert(`❌ At least one malformed value provided for variable ${varName}. Please provide all the numbers in your CSV file and resubmit.`);
                                        error_detected = true;
                                        break; // Stop processing this array if an invalid value is found
                                    }

                                    let float_peer_shares = await handleFloatInput(formData, varName, value, variableInfo.bit_len1, variableInfo.bit_len2, technique, bits, peers, threshold, fieldSize, ring_size);
                                    for (let i = 0; i < peers; i++) {
                                        all_shares_per_party[i].push(float_peer_shares[i]);  // pushes [a, b, c, d] per peer
                                    }
                                    processedVariableOrder.push({ name: varName, type: ftype, dimension: 1 });
                                }
                            } else if (ftype === "int") { // 1D ARRAY OF INT
                                console.log(`Handled as int 1D array: varName = ${varName}, secret = ${secret}, dimension = ${dimension}, ftype = ${ftype}`);
                                for (let i = 0; i < variableInfo.size1; i++) {
                                    let value = secret[i]; // "value = 4,32,444235252525252525"
                                    console.log(`value = ${value}`);

                                    if (('' + value).includes(',')) { // Check if it's not a number or contains a comma (indicating multiple values)
                                        alert(`❌ Variable '${varName}' is a 1-dimensional array. Expected ${variableInfo.size1} values, provided as a single column in a CSV file.`);
                                        error_detected = true;
                                        break; // Stop processing this array if an invalid value is found
                                    }

                                    if (value === undefined || value === null) {
                                        alert(`❌ Empty value provided. Variable '${varName}' is a 1-dimensional array. Expected ${variableInfo.size1} values, provided as a single column in a CSV file.`);
                                        error_detected = true;
                                        break; // Stop processing this array if an invalid value is found
                                    }

                                    if (isNaN(Number(value))) {
                                        alert(`❌ At least one malformed value provided for variable ${varName}. Please provide all the numbers in your CSV file and resubmit.`);
                                        error_detected = true;
                                        break; // Stop processing this array if an invalid value is found
                                    }

                                    const isBitLengthValid = await validateArrayBitLength(varName, value);
                                    if (error_detected == false) {
                                        if (isBitLengthValid == true) {
                                            let int_peer_shares = await handleIntInput(formData, varName, value, technique, bits, peers, threshold, fieldSize, ring_size);
                                            for (let i = 0; i < peers; i++) {
                                                all_shares_per_party[i].push([int_peer_shares[i]]);  // wrap each int in an array
                                            }
                                            processedVariableOrder.push({ name: varName, type: ftype, dimension: 1 });
                                        }  else { // Stop all the processes after this
                                            error_detected = true; 
                                        }
                                    }
                                }
                            }
                        } else if (dimension == 2) { // 2D ARRAY
                            if (ftype === "float") { // 2D ARRAY OF FLOAT
                                console.log(`Handled as float 2D array: varName = ${varName}, secret = ${secret}, dimension = ${dimension}, ftype = ${ftype}`);
                                for (let i = 0; i < secret.length; i++) {
                                    let value_arr = secret[i];
                                    console.log(`value_arr = ${value_arr}`);

                                    for (let i = 0; i < value_arr.length; i++) {
                                        let value = value_arr[i];
                                        console.log(`value = ${value}`);

                                        // Convert value to string and trim whitespace for robust checking
                                        const trimmedValue = (value + '').trim(); // Convert to string and remove leading/trailing whitespace

                                        if (value === undefined || value === null || trimmedValue === '') {
                                            alert(`❌ At least one empty value provided for variable ${varName}. Please provide all the numbers in your CSV file and resubmit.`);
                                            error_detected = true;
                                            break; // Stop processing this array if an invalid value is found
                                        }

                                        if (isNaN(Number(value))) {
                                            alert(`❌ At least one malformed value provided for variable ${varName}. Please provide all the numbers in your CSV file and resubmit.`);
                                            error_detected = true;
                                            break; // Stop processing this array if an invalid value is found
                                        }

                                        const float_peer_shares = await handleFloatInput(formData, varName, value, variableInfo.bit_len1, variableInfo.bit_len2, technique, bits, peers, threshold, fieldSize, ring_size);
                                        for (let i = 0; i < peers; i++) {
                                            all_shares_per_party[i].push(float_peer_shares[i]);
                                        }
                                        processedVariableOrder.push({ name: varName, type: ftype, dimension: 2 });
                                    }
                                }
                            } else if (ftype === "int") { // 2D ARRAY OF INT
                                console.log(`Handled as int 2D array: varName = ${varName}, secret = ${secret}, dimension = ${dimension}, ftype = ${ftype}`);
                                for (let i = 0; i < secret.length; i++) {
                                    let value_arr = secret[i];
                                    console.log(`value_arr = ${value_arr}`);
                                    for (let i = 0; i < value_arr.length; i++) {
                                        let value = value_arr[i];
                                        console.log(`value = ${value}`);
                                        const isBitLengthValid = await validateArrayBitLength(varName, value);
                                        if (isBitLengthValid == true) {
                                            const int_peer_shares = await handleIntInput(formData, varName, value, technique, bits, peers, threshold, fieldSize, ring_size);
                                            for (let i = 0; i < peers; i++) {
                                                all_shares_per_party[i].push([int_peer_shares[i]]); 
                                            }
                                            processedVariableOrder.push({ name: varName, type: ftype, dimension: 2 });
                                        }  else { // Stop all the processes after this
                                            error_detected = true; 
                                        }
                                    }
                                }
                            }
                        } // End of 2D array handling
                    } // End of isMatrixSizeValid else
                } // If no errors given
            } // End of array handling 
        } // End of processing each variable in formData

        let allEncryptedShares = [];
        if (error_detected == false) {
            // Print for testing purpose
            all_shares_per_party.forEach((shares, index) => {
                console.log(`111 Raw shares for Peer ${index + 1}:`, shares);
            });

            // Loop through each peer to format and encrypt their shares
            for (let peerIndex = 0; peerIndex < peers; peerIndex++) {
                const sharesOfEachPeer = all_shares_per_party[peerIndex]; // Get the shares of each peer
                // console.log(`222 Raw shares for Peer ${peerIndex + 1}: `, sharesOfEachPeer);

                let finalStringForPeer = "";
                let currentShareIdx = 0; // Tracks position in sharesOfEachPeer for the current peer

                // Iterate through the processed variables in the order they were originally encountered
                // to correctly reconstruct the formatted string for each peer.
                const shareStringsByVariable = new Map(); // Temporarily holds formatted shares grouped by varName

                for (const processedVar of processedVariableOrder) {
                    const varName = processedVar.name;
                    const varType = processedVar.type;
                    const varDim = processedVar.dimension;
                    // console.log(`333 Processing variable: ${varName}, Type: ${varType}, Dimension: ${varDim}`); // Added print statement

                    // Get the raw share block corresponding to this variable/element for the current peer
                    const shareBlock = sharesOfEachPeer[currentShareIdx];
                    let formattedShareElement;

                    // Format the raw share block based on its type
                    if (varType === 'int') {
                        // For an integer, the shareBlock is an array like [12345n]
                        formattedShareElement = shareBlock[0].toString();
                    } else if (varType === 'float') {
                        // For a float, the shareBlock is an array of 4 components like [c1, c2, c3, c4]
                        formattedShareElement = shareBlock.join(',');
                    } else {
                        // Fallback for any other unexpected types
                        formattedShareElement = shareBlock.toString();
                    }

                    // Group the formatted share element under its variable name.
                    // For arrays, all elements of the array will be collected under the same varName key.
                    if (!shareStringsByVariable.has(varName)) {
                        shareStringsByVariable.set(varName, []);
                    }
                    shareStringsByVariable.get(varName).push(formattedShareElement);

                    currentShareIdx++; // Move to the next raw share block in sharesOfEachPeer
                }

                // After processing all variable shares for this peer, construct the final string.
                for (const [varName, sharesArray] of shareStringsByVariable) {
                    // Join all elements' shares for this variable with commas (e.g., "1,2,3" for an array)
                    // and append the variable name and an equals sign, followed by a newline.
                    const metadata = varMetadataMap.get(varName); // The map is maintain above. Now, I want to find that variable and write the shares based on the info of that variable 
                    const {
                        type,
                        dimension,
                        bit_len1,
                        bit_len2,
                        size1,
                        size2,
                        inputParty
                    } = metadata;

                    // console.log(`\n\nVarName: ${varName}, Type: ${type}, Dimension: ${dimension}\n\n`)
                    // console.log(`\n--- Processing Var: ${varName} (Type: ${type}, Dim: ${dimension}, Size1: ${size1}, Size2: ${size2}) ---`);
                    // console.log(`Initial sharesArray for ${varName}:`, sharesArray);
                    // console.log(`Length of sharesArray for ${varName}: ${sharesArray.length}`);
                

                    if (type == 'int') { // INT
                        if (dimension === 0 || dimension == 1) { // Regular and 1D array
                            finalStringForPeer += `${varName}=${sharesArray.join(',')}\n`;
                        } else if (dimension === 2) { // 2D int array (size1 rows, size2 columns, each 1 share)
                            let formatted2DString = '';
                            for (let i = 0; i < size1; i++) {
                                // Each row has 'size2' integer shares
                                const rowShares = sharesArray.slice(i * size2, (i + 1) * size2);
                                formatted2DString += `${varName}=${rowShares.join(',')}\n`;
                            }
                            finalStringForPeer += formatted2DString;
                        }
                    } else if (type == 'float') { // FLOAT
                        const SHARES_PER_FLOAT = 4;
                        if (dimension === 0) { // Regular 
                            finalStringForPeer += `${varName}=${sharesArray.join(',')}\n`;
                        } else if (dimension === 1 || dimension == 2) { // 1D float array (size1 conceptual floats, each 4 shares)
                            // For both 1D and 2D float arrays, the structure of sharesArray is now confirmed
                            // to be an array where each element is a string representing one conceptual float
                            // (e.g., "share1,share2,share3,share4").
                            // We simply iterate over this array and append each string.
                            // console.log(`FLOAT Dim ${dimension}: Processing array ${varName}. Expected conceptual floats: ${size1 * (dimension === 2 ? size2 : 1)}. Actual sharesArray length: ${sharesArray.length}`);

                            for (const floatSharesString of sharesArray) {
                                finalStringForPeer += `${varName}=${floatSharesString}\n`;
                                // console.log(`  Added line for Dim ${dimension}: ${varName}=${floatSharesString}`);
                            }
                        } 
                    }  
                }
                // Remove the trailing newline character, if any
                finalStringForPeer = finalStringForPeer.trimEnd();

                console.log(`\n\n 444 Final string for Peer ${peerIndex + 1} before encryption:\n${finalStringForPeer}\n\n`);

                // Encrypt the entire formatted string for the peer
                const encrypted = await encrypt_share(finalStringForPeer, public_keys[peerIndex]);
                console.log(`555 Final string for Peer ${peerIndex + 1} after encryption: `, encrypted);

                const peerPayload = {
                    peer: peerIndex + 1,
                    shares: [encrypted] // Each peer's payload is now a single encrypted string
                };

                allEncryptedShares.push(peerPayload);
            }
        }
        // console.log("All Encrypted Shares Ready to Send:", allEncryptedShares);

        if (error_detected == false) {
            try {
                // Send the encrypted shares to the server
                await fetch('/submit-config', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ encrypted_variable_shares: allEncryptedShares }),
                });
                previewDiv.innerHTML += `<h2>Successfully submitted encrypted shares!</h2>`;
            } catch (error) {
                console.error('Error submitting data:', error);
                previewDiv.innerHTML += `<h2 style="color: red;">Error submitting encrypted shares!</h2>`;
            }
        } else {
            console.log('Error submitting data!');
        }
    });

    // Attach event listener for file inputs
    document.querySelectorAll('input[type="file"]').forEach(input => {
        input.addEventListener("change", handleFileUpload);
    });
});
