

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

    // Print the config data to the webpage inside a preformatted block - For debuging purpose to see the config file 
    // const debugDiv = document.createElement("pre");
    // debugDiv.style.background = "#f4f4f4";
    // debugDiv.style.padding = "10px";
    // debugDiv.style.border = "1px solid #ccc";
    // debugDiv.style.overflowX = "auto";
    // debugDiv.textContent = JSON.stringify(configData, null, 4); // Pretty-print JSON
    // document.body.insertBefore(debugDiv, document.body.firstChild); // Show at the top of the page
    
    const form = document.getElementById("dynamic-form");
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
    function formatSubmittedData(data, variableConfig) {
        let outputHtml = "";

        Object.entries(data).forEach(([key, value]) => {
            let numbers = [];
            let variable = variableConfig?.variables?.find(v => v.name === key);
            let varName = variable?.name || key; // Extract actual variable name
            let dimension = variable?.dimension ?? 0; // Default dimension to 0 if not found

            if (Array.isArray(value)) {
                value.forEach((item, rowIndex) => {
                    if (typeof item === "object" && item !== null) {
                        let row = [];
                        Object.values(item).forEach((val, colIndex) => {
                            let num = val.replace(/[^0-9]/g, ""); // Extract only numbers
                            if (num) {
                                if (dimension === 2) {
                                    // Store row-column formatted value for matrices
                                    row.push(`${varName}[${rowIndex}][${colIndex}] = ${num}`);
                                } else {
                                    // Store index-based formatted value for arrays
                                    row.push(`${varName}[${rowIndex}] = ${num}`);
                                }
                            }
                        });
                        if (row.length) numbers.push(...row);
                    }
                });
            } else if (typeof value === "string" || typeof value === "number") {
                let num = String(value).replace(/[^0-9]/g, "");
                if (num) numbers.push(`${key} = ${num}`);
            }

            if (numbers.length > 0) {
                outputHtml += `<strong>${key}:</strong><br>`;
                outputHtml += `${numbers.join(", ")}<br><br>`;
            }
        });

        return outputHtml.trim();
    }


    // Function to call the shamir/rss classes to construct the shares for n parties 
    function generateShares(data) {
        let generatedShares = {};
    
        // Check if `data` is a string or object
        if (typeof data === "string") {
            // Process it as a newline-separated string
            let lines = data.split(/\r?\n/).map(line => line.trim()).filter(line => line !== "");
            let key = null;
    
            for (let line of lines) {
                line = line.trim();
                if (line.endsWith(":")) {
                    key = line.slice(0, -1); // Remove ":" to get key
                } else if (key && line !== "") {
                    if (line.includes(",")) {
                        // It's a list, split and multiply each number by 5 (only if it's numeric)
                        generatedShares[key] = line.split(",").map(num => {
                            let parsedNum = parseInt(num.trim());
                            return isNaN(parsedNum) ? num.trim() : parsedNum * 5;
                        });
                    } else {
                        // It's a single value, multiply if it's a number
                        let parsedNum = parseInt(line);
                        generatedShares[key] = isNaN(parsedNum) ? line : parsedNum * 5;
                    }
                    key = null; // Reset key after storing value
                }
            }
        } else if (typeof data === "object") {
            // Process as JSON object
            for (let [key, value] of Object.entries(data)) {
                if (Array.isArray(value)) {
                    // Multiply numeric elements by 5
                    generatedShares[key] = value.map(num => (typeof num === "number" ? num * 5 : num));
                } else if (typeof value === "number") {
                    // Multiply single numeric values by 5
                    generatedShares[key] = value * 5;
                } else {
                    // Keep other values unchanged
                    generatedShares[key] = value;
                }
            }
        }
    
        return JSON.stringify(generatedShares);
    }


    // Function to validate bit length of entered values - used for integers only
    async function validateBitLength(formData) {
        let errors = {}; // Store validation error messages for each field

        for (const [key, value] of Object.entries(formData)) {    
            // Find the variable in configData.variables array
            let variable = configData.variables.find(v => v.name === key);

            // Validate only if the variable is an integer
            if (variable && variable.type === "int" && variable.bit_len1) {
                const expectedBitLength = variable.bit_len1;
                let number = parseInt(value, 10); // Convert value to an integer

                if (isNaN(number)) {
                    errors[key] = `❌ The value must be a valid number.`;
                    continue; // Skip further checks for this variable
                }

                // Determine min/max values based on signed/unsigned
                let minValue, maxValue;

                if (value.startsWith("-")) {
                    // If the value is negative, assume it's signed
                    minValue = -(Math.pow(2, expectedBitLength - 1)); // -2^(bit_len1-1)
                    maxValue = Math.pow(2, expectedBitLength - 1) - 1; // 2^(bit_len1-1) - 1
                } else {
                    // If it's positive, check both signed & unsigned
                    let unsignedMax = Math.pow(2, expectedBitLength) - 1; // 2^bit_len1 - 1
                    let signedMax = Math.pow(2, expectedBitLength - 1) - 1; // 2^(bit_len1-1) - 1
                    let signedMin = -(Math.pow(2, expectedBitLength - 1)); // -2^(bit_len1-1)

                    // Decide whether to validate as signed or unsigned
                    if (number > signedMax) {
                        // If the number is larger than signed max, treat it as unsigned
                        minValue = 0;
                        maxValue = unsignedMax;
                    } else {
                        // Otherwise, treat it as signed
                        minValue = signedMin;
                        maxValue = signedMax;
                    }
                }

                // Check range
                if (number < minValue || number > maxValue) {
                    errors[key] = `❌ The value (${number}) exceeds the allowed range (${minValue} to ${maxValue}) for a ${expectedBitLength}-bit integer.`;
                }
            }
        }

        // Update the UI with error messages
        document.querySelectorAll(".error-message").forEach(el => el.remove()); // Clear previous errors

        Object.entries(errors).forEach(([key, message]) => {
            let inputField = document.querySelector(`[name="${key}"]`);
            if (inputField) {
                let errorSpan = document.createElement("span");
                errorSpan.className = "error-message";
                errorSpan.style.color = "red";
                errorSpan.style.fontSize = "0.9em";
                errorSpan.style.display = "block";
                errorSpan.textContent = message;
                inputField.parentNode.appendChild(errorSpan);
            }
        });

        return Object.keys(errors).length === 0; // Return true if no errors
    }

    // Function to validate array and matrix size
    async function validateMatrixSize(formData) {
        let errors = {}; // Store validation error messages for each field
        console.log("Starting validation...");
    
        for (const [key, value] of Object.entries(formData)) {
            console.log(`Validating field: ${key}`);
            
            // Find the variable in configData.variables array
            let variable = configData.variables.find(v => v.name === key);
            console.log(`Found variable:`, variable);
    
            // Validate only if the variable is a matrix (dimension === 2)
            if (variable && variable.dimension === 2) {
                const expectedRows = variable.size1;
                const expectedCols = variable.size2;
                console.log(`Expected matrix size for ${key}: ${expectedRows}x${expectedCols}`);
    
                // Check if the value is an array (matrix)
                if (!Array.isArray(value)) {
                    console.log(`❌ ${key} is not an array`);
                }
                if (value.length !== expectedRows) {
                    console.log(`❌ ${key} has incorrect number of rows: expected ${expectedRows}, got ${value.length}`);
                }
                if (value.some(row => !Array.isArray(row) || row.length !== expectedCols)) {
                    console.log(`❌ ${key} has rows with incorrect number of columns.`);
                }
    
                // If the matrix is invalid, add an error
                if (!Array.isArray(value) || value.length !== expectedRows || value.some(row => !Array.isArray(row) || row.length !== expectedCols)) {
                    errors[key] = `❌ The matrix size for ${key} must be ${expectedRows}x${expectedCols}.`;
                }
            }
        }
    
        console.log("Validation completed, errors found:", errors);
    
        // Update the UI with error messages
        document.querySelectorAll(".error-message").forEach(el => el.remove()); // Clear previous errors
        Object.entries(errors).forEach(([key, message]) => {
            console.log(`Displaying error for field: ${key}`);
    
            let inputField = document.querySelector(`[name="${key}"]`);
            if (inputField) {
                let errorSpan = document.createElement("span");
                errorSpan.className = "error-message";
                errorSpan.style.color = "red";
                errorSpan.style.fontSize = "0.9em";
                errorSpan.style.display = "block";
                errorSpan.textContent = message;
                inputField.parentNode.appendChild(errorSpan);
            }
        });
    
        const noErrors = Object.keys(errors).length === 0;
        console.log(`Validation result: ${noErrors ? "Passed" : "Failed"}`);
    
        return noErrors; // Return true if no errors
    }
    


    form.addEventListener("submit", async function (event) {
        event.preventDefault();

        let formData = {};
        new FormData(event.target).forEach((value, key) => {
            let inputElement = document.querySelector(`[name="${key}"]`);
            
            if (inputElement && inputElement.type === "file") {
                let actualKey = key.startsWith("file_") ? key.substring(5) : key; // Remove "file_" prefix only for file inputs
                formData[actualKey] = inputElement.dataset.csvContent ? JSON.parse(inputElement.dataset.csvContent) : null;
            } else {
                formData[key] = value; // Keep original key for non-file inputs
            }
        });

        console.log("Submitting Data:", formData);

        // Validate bit length and matrix size before proceeding
        
        const isBitLengthValid = await validateBitLength(formData);
        const isMatrixSizeValid = await validateMatrixSize(formData);

        if (!isBitLengthValid) {
            alert("Bit_length is off!");
            return;
        } else if (!isMatrixSizeValid) {
            alert("Data is not formated correctly!");
            return;
        } else {

            // If validations pass, format the data for display
            let formattedData = formatSubmittedData(formData);

            // Preview the submitted data
            previewDiv.innerHTML = `<h2>Submitted Data</h2>${formattedData}`;

            // Generate shares
            // let new_generatedShares = generateShares(data);

            // Preview the shares - used for testing purposes 
            // previewDiv.innerHTML += `<h2>Generated Shared for all parties!</h2>${new_generatedShares}`;
        }
    });

    // Attach event listener for file inputs
    document.querySelectorAll('input[type="file"]').forEach(input => {
        input.addEventListener("change", handleFileUpload);
    });
});
