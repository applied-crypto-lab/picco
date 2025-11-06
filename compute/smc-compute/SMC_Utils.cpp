/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University at Buffalo (SUNY)

   PICCO is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PICCO is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/
#include "SMC_Utils.h"
#include "bit_utils.hpp"
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::vector;

// Helper to print OpenSSL errors
void handle_openssl_error() {
    ERR_print_errors_fp(stderr); // function that prints the error strings for all errors that OpenSSL has recorded to standard error stream. 
    throw std::runtime_error("OpenSSL Error");
}

/* Function to Base64 decode a string into a vector of unsigned chars
    This function decodes a Base64 encoded string into its original raw binary data.
    It uses OpenSSL's BIO (Basic Input/Output) library for efficient Base64 processing.
    `BIO_f_base64()` creates a Base64 filter BIO, and `BIO_new_mem_buf()` creates a memory BIO
    that holds the input string. `BIO_push` chains them together.
    `BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL)` is crucial to ensure compatibility with
    Base64 encodings that do not include newline characters (like those produced by `forge.util.encode64`).
    `BIO_read` then performs the decoding.
    Info taken from online resources and "https://docs.openssl.org/master/man3/BIO_f_base64/#synopsis"
*/
std::vector<unsigned char> base64_decode(const std::string& encoded_string) {
    // Create a BIO filter for Base64 decoding
    BIO *b64 = BIO_new(BIO_f_base64());

    // Create a memory BIO that uses the input string as its data source
    BIO *bmem = BIO_new_mem_buf(encoded_string.data(), encoded_string.length());

    // Chain the Base64 filter on top of the memory buffer
    bmem = BIO_push(b64, bmem);

    // Disable newlines in Base64 encoding (matches forge.util.encode64 behavior)
    // (Forge's encode64 does not add newlines, so we disable them to match behavior)
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // Allocate a buffer large enough to hold the decoded data 
    std::vector<unsigned char> decoded_data(encoded_string.length()); // Max possible size
    
    // Perform the base64 decoding and store the actual number of decoded bytes
    int len = BIO_read(bmem, decoded_data.data(), decoded_data.size());

    // Free all BIO resources (both b64 and bmem)
    BIO_free_all(bmem);

    // Check if decoding failed (BIO_read returns negative on error)
    if (len < 0) {
        throw std::runtime_error("Base64 decoding failed.");
    }

    // Resize the buffer to contain only the valid decoded bytes
    decoded_data.resize(len);

    // Return the decoded byte array
    return decoded_data;
}

/* 
    Function to read file content (expected to be a Base64 string) and return decoded raw binary.
    This function reads an entire file, assuming its content is a single line of Base64 encoded data.
    It then calls `base64_decode` to convert the Base64 string into raw binary bytes.
*/
std::vector<unsigned char> read_base64_file_and_decode_to_bytes(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }
    std::string full_b64_blob;
    if (!std::getline(file, full_b64_blob)) { // Read the single line containing the Base64 blob
        throw std::runtime_error("File format error: Could not read full Base64 blob from file.");
    }
    file.close();

    // For testing
    // std::cout << "  -- DECODING PHASE --" << std::endl;
    // std::cout << "  Input Base64 string from file (" << full_b64_blob.length() << " chars): " << full_b64_blob.substr(0, std::min((size_t)50, full_b64_blob.length())) << "..." << std::endl;
    // std::cout << "  Performing Base64 decoding..." << std::endl;

    // Call the base64_decode function to decode the input string into raw bytes
    std::vector<unsigned char> decoded_full_blob = base64_decode(full_b64_blob);
    // For testing
    // std::cout << "  Successfully decoded to raw binary blob. Decoded size: " << decoded_full_blob.size() << " bytes." << std::endl;
    // std::cout << "  --------------------" << std::endl;

    // Return the decoded byte vector
    return decoded_full_blob;
}


/* 
    Function to write a vector of unsigned char to a file
*/
void write_bytes_to_file(const std::string& filename, const std::vector<unsigned char>& data) {
    std::ofstream file(filename, std::ios::binary | std::ios::trunc); // trunc to overwrite
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        throw std::runtime_error("Failed to write to file: " + filename);
    }
}

// Function to decrypt RSA-OAEP
// private_key_pem: Your private RSA key in PEM format (string)
// encrypted_data: The data to decrypt (encrypted AES key)
/*
// This function performs RSA decryption using the OAEP (Optimal Asymmetric Encryption Padding) scheme.
// RSA-OAEP is a secure padding scheme for RSA encryption that adds randomness and uses a hash function
// (SHA-256 in this case) to prevent certain attacks.
// It loads the RSA private key from a PEM string, initializes an EVP_PKEY_CTX for decryption,
// sets the padding to RSA_PKCS1_OAEP_PADDING, and crucially configures both the message digest (OAEP_MD)
// and MGF1 (Mask Generation Function 1) hash algorithms to SHA-256, mirroring the encryption side.
// The decryption is a two-pass process: first to get the output size, then to perform the actual decryption.
// Source: OpenSSL EVP_PKEY Functions (https://www.openssl.org/docs/manmaster/man3/EVP_PKEY_decrypt.html)
// and RSA-OAEP (https://www.rfc-editor.org/rfc/rfc3447#section-7.1)
*/
std::vector<unsigned char> rsa_oaep_decrypt(const std::string& private_key_pem,
                                            const std::vector<unsigned char>& encrypted_aes_key_bytes) {
    // Create BIO memory buffer from PEM string
    BIO* mem_bio = BIO_new_mem_buf(private_key_pem.data(), private_key_pem.length());
    if (!mem_bio) handle_openssl_error();

    // Read RSA private key from BIO
    RSA* rsa_key_raw = PEM_read_bio_RSAPrivateKey(mem_bio, NULL, NULL, NULL);
    BIO_free(mem_bio);
    if (!rsa_key_raw) handle_openssl_error();

    // Wrap raw RSA key into EVP_PKEY
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) { RSA_free(rsa_key_raw); handle_openssl_error(); }
    if (1 != EVP_PKEY_assign_RSA(pkey, rsa_key_raw)) { // Assign RSA key to EVP_PKEY
        RSA_free(rsa_key_raw); EVP_PKEY_free(pkey); handle_openssl_error();
    }

    // Create decryption context
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) { EVP_PKEY_free(pkey); handle_openssl_error(); }

    // Initialize decryption with RSA-OAEP padding
    if (1 != EVP_PKEY_decrypt_init(ctx)) handle_openssl_error();
    if (1 != EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING)) handle_openssl_error();

    // Set OAEP hashing algorithm (SHA-256)
    // --- THESE ARE THE CRITICAL LINES FOR OAEP HASHES ---
    // Set the message digest algorithm (hash used for the input to OAEP)
    if (1 != EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256())) handle_openssl_error();
    // Set the MGF1 (Mask Generation Function 1) hash algorithm
    if (1 != EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256())) handle_openssl_error();
    // --- END CRITICAL LINES ---

    // Get size needed for decrypted output
    size_t decrypted_len = 0;
    // First call to determine buffer size
    if (1 != EVP_PKEY_decrypt(ctx, NULL, &decrypted_len, encrypted_aes_key_bytes.data(), encrypted_aes_key_bytes.size())) handle_openssl_error();

    // Perform actual decryption
    std::vector<unsigned char> decrypted_data(decrypted_len);
    // Second call to perform decryption
    if (1 != EVP_PKEY_decrypt(ctx, decrypted_data.data(), &decrypted_len, encrypted_aes_key_bytes.data(), encrypted_aes_key_bytes.size())) handle_openssl_error();

    // Cleanup
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey); // This also frees the underlying RSA_KEY

    decrypted_data.resize(decrypted_len);
    return decrypted_data;
}


// Function to decrypt AES-GCM
// key: Decrypted AES key (16 bytes for 128-bit AES)
// iv: IV (12 bytes)
// tag: Authentication Tag (16 bytes)
// ciphertext: Encrypted data
/*
    This function performs AES decryption in GCM (Galois/Counter Mode). AES-GCM is an authenticated encryption
    mode, meaning it provides both confidentiality (encryption) and integrity/authenticity (via an authentication tag).
    It initializes an EVP_CIPHER_CTX with `EVP_aes_128_gcm()`, sets the IV length, the decryption key, and the IV.
    `EVP_DecryptUpdate` processes the ciphertext. Crucially, before `EVP_DecryptFinal_ex`, the authentication tag
    is set using `EVP_CIPHER_CTX_ctrl` with `EVP_CTRL_GCM_SET_TAG`. `EVP_DecryptFinal_ex` then performs the final
    decryption and verifies the authenticity of the data using the provided tag. If the tag verification fails,
    the function throws a `std::runtime_error`.
    Source: OpenSSL AES-GCM Documentation (https://www.openssl.org/docs/manmaster/man3/EVP_aes_128_gcm.html)
    and EVP Symmetric Encryption and Decryption (https://www.openssl.org/docs/manmaster/man3/EVP_EncryptInit.html)
*/
std::vector<unsigned char> aes_gcm_decrypt(const std::vector<unsigned char>& key,
                                           const std::vector<unsigned char>& iv,
                                           const std::vector<unsigned char>& tag,
                                           const std::vector<unsigned char>& ciphertext) {
    // Create context for AES-GCM decryption
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_openssl_error();

    std::vector<unsigned char> plaintext(ciphertext.size()); // Plaintext max size same as ciphertext
    int len = 0;
    int plaintext_len = 0;

    // Initialize AES-128-GCM decryption operation.
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL)) handle_openssl_error();

    // Set IV length (optional, it defaults to the standard for the cipher in use)
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL)) handle_openssl_error();

    // Set key and IV
    if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key.data(), iv.data())) handle_openssl_error();

    // Provide the message to be decrypted, and obtain the plaintext output.
    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size())) handle_openssl_error();
    plaintext_len = len;

    // Set expected tag value.
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), (void*)tag.data())) handle_openssl_error();

    // Finalize the decryption. A return value of 1 indicates success, 0 indicates failure.
    // (e.g., authentication tag verification failed).
    int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
    EVP_CIPHER_CTX_free(ctx);

    if (ret <= 0) { // Check return value to see if tag verification passed
        throw std::runtime_error("AES-GCM decryption failed or tag verification failed.");
    }
    plaintext_len += len;
    plaintext.resize(plaintext_len);
    return plaintext;
}

// Assuming SMC_Utils or a similar class can provide the private key
std::string get_private_key_pem_from_smc_utils(const char* private_key_filename) {
    std::ifstream file(private_key_filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open private key file: " + std::string(private_key_filename));
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/*
    Decryption code that is written based on the encryption code in js on form_handler.js file

    JavaScript code performs a two-step encryption process:

        1. AES-GCM Encryption: The actual share (converted to a string) is encrypted using 
        AES-GCM with a randomly generated 128-bit (16 bytes) AES key and a 12-byte random 
        IV. The output of this step is the ciphertext and an authentication tag 
        (128 bits / 16 bytes). The IV, ciphertext, and tag are then concatenated.
        
        2. RSA-OAEP Encryption: The randomly generated AES key from step 1 is then encrypted
        using RSA-OAEP with SHA-256 and the recipient's public RSA key.

    Therefore, the C++ decryption process will need to reverse these steps:

        1. Separate the concatenated combinedData back into its three components: 
                a. IV (first 12 bytes)
                b. Ciphertext (middle part)
                c. Authentication tag (last 16 bytes, as tagLength was 128 bits).

        2. Use the decrypted AES key, the extracted IV, and the authentication tag to decrypt 
        the ciphertext and verify its authenticity.

        3. The result will be the original share as a string. 
*/

// --- Main Decryption Function ---
// combined_data_bytes: The concatenation of IV + AES-GCM_Ciphertext + AES-GCM_Tag.
// encrypted_key_bytes: The RSA-OAEP encrypted AES key.
// public_keys: Key in PEM string format. (Parties public keys)
std::string decrypt_share(const std::vector<unsigned char>& full_encrypted_file_content,
                          const std::string& private_key_pem) {

    std::string decrypted_share_string;

    // Determine sizes from JS encryption:
    const size_t iv_length = 12;    // As per JS: forge.random.getBytesSync(12)
    const size_t tag_length = 16;   // As per JS: tagLength: 128 bits = 16 bytes
    const size_t aes_key_length = 16; // AES-128 key is 16 bytes

    // Assuming the format is [RSA_OAEP_Encrypted_AES_Key][IV][AES_GCM_Ciphertext][AES_GCM_Tag]
    // The RSA encrypted key size depends on the RSA key size, but the decrypted AES key is 16 bytes.
    // The *encrypted* AES key size will be the RSA key's modulus size (e.g., 256 bytes for a 2048-bit RSA key).
    // You MUST know the exact size of the RSA-OAEP encrypted AES key.
    // Let's assume for this example, the RSA key size is 2048 bits, so encrypted_key_size = 256 bytes.
    // If your RSA key size is different, you'll need to adjust this.
    // A more robust solution would be to prepend the encrypted_key_size to the file.
    const size_t encrypted_aes_key_size = 256; // Example: for 2048-bit RSA key

    if (full_encrypted_file_content.size() < encrypted_aes_key_size + iv_length + tag_length) {
        throw std::runtime_error("Encrypted file content is too short.");
    }

    // Extract encrypted AES key
    std::vector<unsigned char> encrypted_key_bytes(
        full_encrypted_file_content.begin(),
        full_encrypted_file_content.begin() + encrypted_aes_key_size
    );
    // For testing
    // std::cout << "  - Encrypted AES Key (" << encrypted_key_bytes.size() << " bytes) extracted." << std::endl;

    // Extract IV
    std::vector<unsigned char> iv(
        full_encrypted_file_content.begin() + encrypted_aes_key_size,
        full_encrypted_file_content.begin() + encrypted_aes_key_size + iv_length
    );
    // For testing
    // std::cout << "  - IV (" << iv.size() << " bytes) extracted." << std::endl;

    // Extract Tag
    std::vector<unsigned char> tag(
        full_encrypted_file_content.end() - tag_length,
        full_encrypted_file_content.end()
    );
    // For testing
    // std::cout << "  - Tag (" << tag.size() << " bytes) extracted." << std::endl;

    // Extract Ciphertext
    std::vector<unsigned char> ciphertext(
        full_encrypted_file_content.begin() + encrypted_aes_key_size + iv_length,
        full_encrypted_file_content.end() - tag_length
    );
    // For testing
    // std::cout << "  - Ciphertext (" << ciphertext.size() << " bytes) extracted." << std::endl;


    // --- Step 1: Decrypt the AES key using RSA-OAEP ---
    // std::cout << "Performing RSA-OAEP decryption of AES key..." << std::endl;
    std::vector<unsigned char> decrypted_aes_key = rsa_oaep_decrypt(private_key_pem, encrypted_key_bytes);
    if (decrypted_aes_key.size() != aes_key_length) {
        throw std::runtime_error("Decrypted AES key has incorrect length.");
    }
    // std::cout << "  - AES Key decrypted (" << decrypted_aes_key.size() << " bytes)." << std::endl;

    // --- Step 2: Use the decrypted AES key, IV, and Tag to decrypt the Ciphertext (AES-GCM) ---
    // std::cout << "Performing AES-GCM decryption of ciphertext..." << std::endl;
    std::vector<unsigned char> decrypted_plaintext = aes_gcm_decrypt(decrypted_aes_key, iv, tag, ciphertext);
    // std::cout << "  - Ciphertext decrypted (" << decrypted_plaintext.size() << " bytes)." << std::endl;

    // Convert decrypted bytes to string
    decrypted_share_string.assign(reinterpret_cast<const char*>(decrypted_plaintext.data()), decrypted_plaintext.size());

    return decrypted_share_string;
}

// Constructors
SMC_Utils::SMC_Utils(int _id, std::string runtime_config, std::string privatekey_filename, int numOfInputPeers, int numOfOutputPeers, std::string *IO_files, int numOfPeers, int threshold, int bits, std::string mod, std::vector<int> &seed_map, int num_threads, int mode) {
    id = _id;
#if __DEPLOYMENT__
    printf("DEPLOYMENT MODE\n");
#else
    printf("BENCHMARK MODE\n");
#endif

    std::cout << "SMC_Utils constructor\n";
#if __SHAMIR__
    priv_int modulus;
    ss_init(modulus);
    mpz_set_str(modulus, mod.c_str(), BASE_10);
#endif

    nodeConfig = new NodeConfiguration(id, runtime_config, bits, (numOfPeers - 1));

    std::cout << "Creating the NodeNetwork\n";
    NodeNetwork *nodeNet = new NodeNetwork(nodeConfig, privatekey_filename, num_threads);
    net = *nodeNet; // dereferencing, net is "copy initialized"

    seedSetup(seed_map, numOfPeers, threshold);

    std::cout << "Creating SecretShare\n";
#if __SHAMIR__
    if (num_threads > 1) {
        net.launchManager(); // launching thread manager here as to not conflict with seed setup, only done for Shamir since RSS doesn't support multithreading
        printf("Technique: SHAMIR\n");
    }
    ss = new SecretShare(numOfPeers, threshold, modulus, id, num_threads, net.getPRGseeds(), shamir_seeds_coefs);
#endif
#if __RSS__

    if (bits > 8 * sizeof(priv_int_t)) {
        std::cerr << "ring size cannot be larger than the bitlength of priv_int_t\nExiting...\n";
        std::exit(1);
    }
    printf("Technique: RSS\n");
    ss = new replicatedSecretShare<std::remove_pointer_t<priv_int>>(id, numOfPeers, threshold, bits, rss_share_seeds);

#endif

// initialize input and output streams (deployment mode only)
#if __DEPLOYMENT__
// #if __SHAMIR__
    try {
        inputStreams = new std::ifstream[numOfInputPeers];
        outputStreams = new std::ofstream[numOfOutputPeers];
        for (int i = 0; i < numOfInputPeers; i++) {
            if (mode == 2) { // If mode is 2 -> -d -> web-server
                std::string private_key_pem_content = get_private_key_pem_from_smc_utils(privatekey_filename.c_str());
                std::string current_file_path = IO_files[i]; 
                // For testing
                // std::cout << "\nAttempting to decrypt and overwrite input file: " << current_file_path << std::endl;
                // std::cout << "\nFiles used for: " << privatekey_filename << ", " << IO_files[i] << std::endl;
                try {
                    // 1. Read the Base64 encoded file content and DECODE it into raw binary bytes.
                    //    'read_base64_file_and_decode_to_bytes' will perform the Base64 decoding.
                    std::vector<unsigned char> decoded_full_blob = read_base64_file_and_decode_to_bytes(current_file_path);

                    // 2. Decrypt the content
                    std::string decrypted_content_str = decrypt_share(decoded_full_blob, private_key_pem_content);
                    std::vector<unsigned char> decrypted_content_bytes(decrypted_content_str.begin(), decrypted_content_str.end());
                    // std::cout << "Decrypted content size: " << decrypted_content_bytes.size() << " bytes." << std::endl;

                    // 3. Store the decrypted content back into the same file
                    write_bytes_to_file(current_file_path, decrypted_content_bytes);
                    // std::cout << "Successfully decrypted and overwrote file: " << current_file_path << std::endl;

                } catch (const std::runtime_error& e) {
                    std::cerr << "Error decrypting file " << current_file_path << ": " << e.what() << std::endl;
                    exit(1);
                }
            }
            inputStreams[i].open(IO_files[i].c_str(), std::ifstream::in);
            if (!inputStreams[i]) {
                std::cout << "Input files from input party " + std::to_string(i + 1) + " could not be opened\n";
                std::exit(1);
            }
        }
        for (int i = 0; i < numOfOutputPeers; i++) {
            std::stringstream c;
            c << id;
            std::string s = c.str();
            IO_files[numOfInputPeers + i] = IO_files[numOfInputPeers + i] + s;
            outputStreams[i].open(IO_files[numOfInputPeers + i].c_str(), std::ofstream::out);
            if (!outputStreams[i]) {
                std::cout << "Output files from output party " + std::to_string(i + 1) + " could not be opened\n";
                std::exit(1);
            }
        }
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[SMC_Utils, constructor] " + error);
    }
#endif
}

/* Specific SMC Utility Functions */
// open a single integer value
int SMC_Utils::smc_open(priv_int var, int threadID) {
    return Open_int(var, threadID, net, ss);
}

// open a single floating-point value
float SMC_Utils::smc_open(priv_int *var, int threadID) {
    return Open_float(var, threadID, net, ss);
}

#if __SHAMIR__
void SMC_Utils::smc_open(priv_int *result, priv_int *input, int size, int threadID) {
    Open(result, input, size, threadID, net, ss);
}
#endif

#if __RSS__
void SMC_Utils::smc_open(priv_int result, priv_int *var, int size, int threadID) {
    Open(result, var, size, threadID, net, ss);
}
#endif

// for integer variable I/O
// why is this a pointer?
void SMC_Utils::smc_input(int id, int *var, std::string type, int threadID) {
    try {
        ss->ss_input(id, var, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
}

// why is this a pointer?
void SMC_Utils::smc_input(int id, priv_int *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    try {
        ss->ss_input(id, var, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }

#else
    ss->ss_input(var, type);
#endif
}

// for float variable I/O
void SMC_Utils::smc_input(int id, float *var, std::string type, int threadID) {
    try {
        ss->ss_input(id, var, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
}

// input private float
void SMC_Utils::smc_input(int id, priv_int **var, std::string type, int threadID) {
#if __DEPLOYMENT__
    try {
        ss->ss_input(id, var, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
#else
    ss->ss_input(var, type);
#endif
}

// one-dimensional int array I/O
void SMC_Utils::smc_input(int id, priv_int *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    try {
        ss->ss_input(id, var, size, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
#else
    ss->ss_input(var, size, type);
#endif
}

void SMC_Utils::smc_input(int id, int *var, int size, std::string type, int threadID) {
    try {
        ss->ss_input(id, var, size, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
}

// one-dimensional float array I/O
void SMC_Utils::smc_input(int id, priv_int **var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    try {
        ss->ss_input(id, var, size, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
#else
    ss->ss_input(var, size, type);
#endif
}

#if __SHAMIR__

void SMC_Utils::smc_add_ui(priv_int rop, priv_int op1, uint op2) {

    ss->modAdd(rop, op1, op2);
}
void SMC_Utils::smc_sub_ui(priv_int rop, priv_int op1, uint op2) {

    ss->modSub(rop, op1, op2);
}
#endif
#if __RSS__

void SMC_Utils::smc_add_ui(priv_int *rop, priv_int *op1, uint op2) {

    ss->modAdd(rop, op1, op2, 1);
}
void SMC_Utils::smc_sub_ui(priv_int *rop, priv_int *op1, uint op2) {

    ss->modSub(rop, op1, op2, 1);
}
#endif

void SMC_Utils::smc_input(int id, float *var, int size, std::string type, int threadID) {
    try {
        ss->ss_input(id, var, size, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
}

void SMC_Utils::smc_output(int id, int *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif // otherwise, do nothing
}

void SMC_Utils::smc_output(int id, priv_int *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, float *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, priv_int **var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, priv_int *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, int *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, priv_int **var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, float *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

/* SMC Addition */
/********* singular operations *******/
// 1) private int = private int + private int
void SMC_Utils::smc_add(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, a, b);
}
// 3) private int = private int + public int
void SMC_Utils::smc_add(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, a, b);
}
// 4) private int = public int + private int
void SMC_Utils::smc_add(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, b, a);
}
// 5) private float = private float + public float
void SMC_Utils::smc_add(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_add(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}
// 6) private float = private float + public float
void SMC_Utils::smc_add(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_add(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}
// 2) private float = private float + private float
void SMC_Utils::smc_add(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "+", threadID, net, ss);
}

/************ batch operations *********/
// 1) private *int = public *int + private *int
void SMC_Utils::smc_add(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, b, a, size);
}
// 2) private *int = private *int + public *int
void SMC_Utils::smc_add(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, a, b, size);
}
// 3) private *int = private *int + private *int
void SMC_Utils::smc_add(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, a, b, size);
}
// 6) private *float = private *float + public *floats
void SMC_Utils::smc_add(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    smc_add(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&btmp, size);
}
// 5) private *float = public *float + private *float
void SMC_Utils::smc_add(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_add(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}
// 4) private *float = private *float + private *float
void SMC_Utils::smc_add(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "+", threadID, net, ss);
}

void SMC_Utils::smc_set(priv_int *a, priv_int *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

void SMC_Utils::smc_set(priv_int **a, priv_int **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

void SMC_Utils::smc_set(float *a, priv_int **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size,std::string type, int threadID) {
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

void SMC_Utils::smc_set(float a, priv_int *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

void SMC_Utils::smc_set(priv_int a, priv_int result, int alen, int resultlen, std::string type, int threadID) {
    ss_set(a, result, alen, resultlen, type, threadID, net, ss);
}

void SMC_Utils::smc_set(priv_int *a, priv_int *result, int alen, int resultlen, int size, std::string type, int threadID) {

#if __SHAMIR__
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen, resultlen, type, threadID, net, ss);
#endif

#if __RSS__
    ss_set(a, result, alen, resultlen, size, type, threadID, net, ss);
#endif
}

// this routine should implement in a way that result = a + share[0]
void SMC_Utils::smc_set(int a, priv_int result, int alen, int resultlen, std::string type, int threadID) {
    ss_set(a, result, alen, resultlen, type, threadID, net, ss);
}

void SMC_Utils::smc_set(int *a, priv_int *result, int alen, int resultlen, int size, std::string type, int threadID) {
    for (size_t i = 0; i < size; i++) {
        ss_set(a[i], result[i], alen, resultlen, type, threadID, net, ss);
    }
}

void SMC_Utils::smc_priv_eval(priv_int a, priv_int b, priv_int cond, int threadID) {
    ss_priv_eval(a, b, cond, threadID, net, ss);
}

// floating point
void SMC_Utils::smc_priv_eval(priv_int *a, priv_int *b, priv_int cond, int threadID) {
    ss_priv_eval(a, b, cond, threadID, net, ss);
}

/* SMC Subtraction */
void SMC_Utils::smc_sub(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_sub(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}

void SMC_Utils::smc_sub(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_sub(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_sub(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_sub(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

// batch operations of subtraction
void SMC_Utils::smc_sub(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, ss);
}

void SMC_Utils::smc_sub(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, ss);
}

void SMC_Utils::smc_sub(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, ss);
}
#if __SHAMIR__
/* SMC Multiplication */
void SMC_Utils::smc_mult(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    // double check this works (passing reference to essentiall up-cast to priv_int*)
    // cant use & for some reason, wont compile
    // replacing priv_int* with & works on
    Mult(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), 1, threadID, net, ss);
}
#endif
/******************************************************/
void SMC_Utils::smc_mult(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modMul(result, a, b);
}

void SMC_Utils::smc_mult(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modMul(result, b, a);
}

void SMC_Utils::smc_mult(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "*", threadID, net, ss);
}

void SMC_Utils::smc_mult(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_mult(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_mult(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_mult(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}

#if __SHAMIR__
// private float * private int (needs more examinations)
void SMC_Utils::smc_mult(priv_int *a, priv_int b, priv_int *result, int alen_sig, int alen_exp, int blen, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int b1, one;
    ss_init(b1);
    ss_init_set_ui(one, 1);
    // start computation
    ss->modSub(b1, one, b);
    smc_lor(a[2], b1, result[2], 1, 1, 1, type, threadID);
    // free the memory
    ss_clear(b1);
    ss_clear(one);
}
#endif

void SMC_Utils::smc_mult(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modMul(result, b, a, size);
}

void SMC_Utils::smc_mult(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modMul(result, a, b, size);
}

void SMC_Utils::smc_mult(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    Mult(result, a, b, size, threadID, net, ss);
}

void SMC_Utils::smc_mult(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    smc_mult(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_mult(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_mult(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_mult(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "*", threadID, net, ss);
}

/* SMC Integer Division*/
#if __SHAMIR__
void SMC_Utils::smc_div(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), resultlen, 1, threadID, net, ss);
    // doOperation_IntDiv(result, a, b, resultlen, threadID, net, ss);
}

void SMC_Utils::smc_div(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv_Pub(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, resultlen, 1, threadID, net, ss);
}

void SMC_Utils::smc_div(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), resultlen, 1, threadID, net, ss);
}
#endif

void SMC_Utils::smc_div(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv(result, a, b, resultlen, size, threadID, net, ss);
}

void SMC_Utils::smc_div(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv(result, a, b, resultlen, size, threadID, net, ss);
}

void SMC_Utils::smc_div(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv_Pub(result, a, b, resultlen, size, threadID, net, ss);
}

/* SMC Floating-point division */
void SMC_Utils::smc_div(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "/", threadID, net, ss);
}

void SMC_Utils::smc_div(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int **results = (priv_int **)malloc(sizeof(priv_int *));
    priv_int **as = (priv_int **)malloc(sizeof(priv_int *));
    priv_int **bs;

    as[0] = (priv_int *)malloc(sizeof(priv_int) * 4);
    results[0] = (priv_int *)malloc(sizeof(priv_int) * 4);
    ss_batch_convert_to_private_float(&b, &bs, alen_sig, alen_exp, 1, ss);

    for (int i = 0; i < 4; i++) {
        ss_init(results[0][i]);
        ss_init_set(as[0][i], a[i]);
    }

    doOperation_FLDiv_Pub(as, bs, results, alen_sig, 1, threadID, net, ss);
    ss_process_results(results, resultlen_sig, resultlen_exp, alen_sig, alen_exp, 1, threadID, net, ss);

    for (int i = 0; i < 4; i++)
        ss_set(result[i], results[0][i]);

    // free the memory
    ss_batch_free_operator(&as, 1);
    ss_batch_free_operator(&bs, 1);
    ss_batch_free_operator(&results, 1);
}

void SMC_Utils::smc_div(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_div(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_div(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "/", threadID, net, ss);
}

void SMC_Utils::smc_div(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    doOperation_FLDiv_Pub(a, btmp, result, alen_sig, size, threadID, net, ss);
    ss_process_results(result, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID, net, ss);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_div(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_div(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}

/* All Comparisons */
#if __SHAMIR__
// Int
// 1) private int < private int
void SMC_Utils::smc_lt(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
}
// 2) private int < public int
void SMC_Utils::smc_lt(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, ss);
}
// 3) public int < private int
void SMC_Utils::smc_lt(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
}
#endif

// Float
// 4) private float < private float
void SMC_Utils::smc_lt(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
}
// take b turn to array of 4 ints, using convertfloat same like the flltz and tq. 
// float can be larger than priv below -> but the size can be different for float, so we can use
// same as a or float -> so if the float is larger than a then it can change the size of the private a -> so we keeep the size same for both a and pub b
// 5) private float < public float -> (a, pb)
void SMC_Utils::smc_lt(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
    ss_batch_free_operator(&btmp, 4);
  
    // New_A_Code 
    // int *btmp = new int[4]; // add memory for btmp
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, "<0", threadID, net, ss);

    // delete[] btmp;
    // delete[] elements;
}
// 6) public float < private float -> (pa, b) -> Flip the order (b, pa) and the result (modSub)
void SMC_Utils::smc_lt(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss_batch_free_operator(&atmp, 4);

    // New_A_Code
    // int *atmp = new int[4]; // add memory for atmp
    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, "<0", threadID, net, ss);
    
    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int = private *int < private *int
void SMC_Utils::smc_lt(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}
// 2) private *int = private *float < private *float
void SMC_Utils::smc_lt(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
}

// New_A_code
// int 
// 3) private *int = public *int < private *int
void SMC_Utils::smc_lt(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}
// 4) private *int = private *int < public *int
void SMC_Utils::smc_lt(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}

// float
// 5) private *int = public *float < private *float
void SMC_Utils::smc_lt(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
}
// 6) private *int = private *float < public *float
void SMC_Utils::smc_lt(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
}

#if __SHAMIR__
// 1) private int > private int
void SMC_Utils::smc_gt(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, ss);
}
// 2) private int > public int
void SMC_Utils::smc_gt(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&b, MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, ss);
}
// 3) public int > private int
void SMC_Utils::smc_gt(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, ss);
}
#endif
// 4) private float > private float
void SMC_Utils::smc_gt(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, b, a, resultlen, blen_sig, blen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
}

// 5) private float > public float

void SMC_Utils::smc_gt(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
    ss_batch_free_operator(&btmp, 4);
    
    // New_A_Code
    // New version that uses the new protocol between private and public 
    // int *btmp = new int[4]; 

    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, "<0", threadID, net, ss);

    // delete[] btmp;
    // delete[] elements;
}

// 6) public float > private float
void SMC_Utils::smc_gt(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss_batch_free_operator(&atmp, 4);

    // New_A_Code
    // New version that uses the new protocol between private and public 
    // int *atmp = new int[4];

    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }
    
    // ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, "<0", threadID, net, ss);

    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int = private *int > private *int
void SMC_Utils::smc_gt(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
}
// 2) private *int = private *float > private *float
void SMC_Utils::smc_gt(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
}

// New_A_code
// int 
// 3) private *int = public *int > private *int
void SMC_Utils::smc_gt(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
}
// 4) private *int = private *int > public *int
void SMC_Utils::smc_gt(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
}

// float 
// 5) private *int = public *float > private *float
void SMC_Utils::smc_gt(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
}
// 6) private *int = private *float > public *float
void SMC_Utils::smc_gt(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
}

#if __SHAMIR__
// 1) private int <= private int
void SMC_Utils::smc_leq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 2) private int <= public int
void SMC_Utils::smc_leq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&b, MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 3) public int <= private int
void SMC_Utils::smc_leq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
#endif
// 4) private float <= private float
void SMC_Utils::smc_leq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, b, a, resultlen, blen_sig, blen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 5) private float <= public float
void SMC_Utils::smc_leq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);

    // New_A_Code 
    // int *btmp = new int[4]; 
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result);

    // delete[] btmp;
    // delete[] elements;
}

// 6) public float <= private float
void SMC_Utils::smc_leq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);

    // New_A_Code 
    // int *atmp = new int[4];

    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result);

    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int = private *int <= private *int
void SMC_Utils::smc_leq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 2) private *int = private *float <= private *float
void SMC_Utils::smc_leq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
    ss->modSub(result, 1, result, size);
}

// New_A_code
// int 
// 3) private *int = public *int <= private *int
void SMC_Utils::smc_leq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 4) private *int = private *int <= public *int
void SMC_Utils::smc_leq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// float 
// 5) private *int = public *float <= private *float
void SMC_Utils::smc_leq(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}
// 6) private *int = private *float <= public *float
void SMC_Utils::smc_leq(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}

#if __SHAMIR__
// 1) private int >= private int
void SMC_Utils::smc_geq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 2) private int >= public int
void SMC_Utils::smc_geq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 3) public int >= private int
void SMC_Utils::smc_geq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
#endif
// 4) private float >= private float
void SMC_Utils::smc_geq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 5) private float >= public float
void SMC_Utils::smc_geq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);

    // New_A_code
    // int *btmp = new int[4]; 
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result); 

    // delete[] btmp;
    // delete[] elements;
}
// 6) public float >= private float
void SMC_Utils::smc_geq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);

    // New_A_code
    // int *atmp = new int[4];
    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result);
    
    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int = private *int >= private *int
void SMC_Utils::smc_geq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 2) private *int = private *float >= private *float
void SMC_Utils::smc_geq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
    ss->modSub(result, 1, result, size);
}

// New_A_code 
// int
// 3) private *int = public *int >= private *int
void SMC_Utils::smc_geq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 4) private *int = private *int >= public *int
void SMC_Utils::smc_geq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// float
// 5) private *int = public *float >= private *float
void SMC_Utils::smc_geq(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}
// 6) private *int = private *float >= public *float
void SMC_Utils::smc_geq(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}

#if __SHAMIR__
// Equality and Inequality
// 1) private int == private int
void SMC_Utils::smc_eqeq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
}
// 2) private int == public int
void SMC_Utils::smc_eqeq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, ss);
}
// 3) public int == private int
void SMC_Utils::smc_eqeq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, ss);
}
#endif
// 4) private float == private float
void SMC_Utils::smc_eqeq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "==", threadID, net, ss);
}
// 5) private float == public float
void SMC_Utils::smc_eqeq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "==", threadID, net, ss);
    ss_batch_free_operator(&btmp, 4);

    // New_A_Code
    // int *btmp = new int[4]; 
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, "==", threadID, net, ss);
    
    // delete[] btmp;
    // delete[] elements;
}
// 6) public float == private float
void SMC_Utils::smc_eqeq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "==", threadID, net, ss);
    ss_batch_free_operator(&atmp, 4);

    // New_A_code
    // int *atmp = new int[4];
    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, "==", threadID, net, ss);
   
    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int == private *int
void SMC_Utils::smc_eqeq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}
// 3) private *int == public *int
void SMC_Utils::smc_eqeq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}
// 4) public *int == private *int
void SMC_Utils::smc_eqeq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, b, a, alen, blen, resultlen, size, threadID, net, ss);
}
// 2) private *float == private *float
void SMC_Utils::smc_eqeq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
}
// New_A_code
// 5) public *float == private **float
void SMC_Utils::smc_eqeq(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
}
// 6) private **float == public *float
void SMC_Utils::smc_eqeq(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
}

#if __SHAMIR__
// 1) private int != private int
void SMC_Utils::smc_neq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 2) private int != public int
void SMC_Utils::smc_neq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 3) public int != private int
void SMC_Utils::smc_neq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
#endif
// 4) private float != private float
void SMC_Utils::smc_neq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "==", threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 6) public float != private float
void SMC_Utils::smc_neq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "==", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);

    // New_A_code
    // int *atmp = new int[4];
    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }
    // ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, "==", threadID, net, ss);    
    // ss->modSub(result, 1, result);
    // delete[] atmp;
    // delete[] elements;
}
// 5) private float != public float
void SMC_Utils::smc_neq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "==", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);

    // New_A_code
    // int *btmp = new int[4]; 
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }
    // ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, "==", threadID, net, ss);
    // ss->modSub(result, 1, result);
    // delete[] btmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int != private *int
void SMC_Utils::smc_neq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 2) private *float != private *float
void SMC_Utils::smc_neq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
    ss->modSub(result, 1, result, size);
}

// New_A_code 
// int
// 3) private *int != public *int
void SMC_Utils::smc_neq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 4) public *int != private *int
void SMC_Utils::smc_neq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}

// New_A_code
// float
// 5) private *float != public *float
void SMC_Utils::smc_neq(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}
// 6) public *float != private *float
void SMC_Utils::smc_neq(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}

// batch logical operations
// 2) private float && private float 
void SMC_Utils::smc_land(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    LogicalAnd(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}

void SMC_Utils::smc_lor(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    LogicalOr(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}

// batch logical operations
void SMC_Utils::smc_lnot(priv_int *a, int size, priv_int *result, int alen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, 1, a, size);
}

void SMC_Utils::smc_band(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    // ensuring the first argument always has the longer bitlength
    if (blen > alen) {
        BitAnd(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    } else {
        BitAnd(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    }
}
void SMC_Utils::smc_band(int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitAnd(b, a, result, blen, alen, resultlen, size, threadID, net, ss);
}
void SMC_Utils::smc_band(priv_int *a, int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitAnd(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}
void SMC_Utils::smc_bxor(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    // ensuring the first argument always has the longer bitlength
    if (blen > alen) {
        BitXor(b, a, result, blen, alen, resultlen, size, threadID, net, ss);
    } else {

        BitXor(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    }
}
void SMC_Utils::smc_bxor(int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitXor(b, a, result, blen, alen, resultlen, size, threadID, net, ss);
}
void SMC_Utils::smc_bxor(priv_int *a, int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitXor(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}

void SMC_Utils::smc_bor(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen > alen) {
        BitOr(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    } else {
        BitOr(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    }
}

void SMC_Utils::smc_bor(int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitOr(b, a, result, blen, alen, resultlen, size, threadID, net, ss);
}
void SMC_Utils::smc_bor(priv_int *a, int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitOr(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}

#if __SHAMIR__

// bitwise operations
void SMC_Utils::smc_band(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen > alen) {
        BitAnd(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    } else {
        BitAnd(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    }
}

void SMC_Utils::smc_bxor(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen > alen) {
        BitXor(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    } else {
        BitXor(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    }
}

void SMC_Utils::smc_bor(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen > alen) {
        BitOr(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    } else {
        BitOr(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    }
}

// logical Operations
// 1) private int && private int 
void SMC_Utils::smc_land(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    LogicalAnd(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
}

void SMC_Utils::smc_lnot(priv_int a, priv_int result, int alen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, 1, a);
}

void SMC_Utils::smc_lor(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    LogicalOr(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
}

void SMC_Utils::smc_shr(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    smc_shr(MPZ_CAST(a), MPZ_CAST(b), alen, blen, MPZ_CAST(result), resultlen, 1, type, threadID);
}

void SMC_Utils::smc_shr(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    smc_shr(MPZ_CAST(a), (int *)&b, alen, blen, MPZ_CAST(result), resultlen, 1, type, threadID);
}
#endif

void SMC_Utils::smc_shr(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    if (blen == -1) { // public b
        // check that m is !> k
        int *b_tmp = (int *)malloc(sizeof(int) * size);
        for (int i = 0; i < size; i++)
            b_tmp[i] = ss_get_si(b[i]);
        doOperation_Trunc(result, a, alen, b_tmp, size, threadID, net, ss);
        free(b_tmp);
    } else
        doOperation_TruncS(result, a, alen, b, size, threadID, net, ss);
}

void SMC_Utils::smc_shr(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_Trunc(result, a, alen, b, size, threadID, net, ss);
}

#if __SHAMIR__
// ANB: does the compiler actually ever produce code that calls this?
void SMC_Utils::smc_shl(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen == -1) {           // b is public, but stored in an priv_int
        ss->modPow2(result, b); // problem for RSS, since we have an incompatible type ()
        ss->modMul(result, a, result);
    } else {
        doOperation_Pow2(MPZ_CAST(result), MPZ_CAST(b), blen, 1, threadID, net, ss);
        Mult(MPZ_CAST(result), MPZ_CAST(result), MPZ_CAST(a), 1, threadID, net, ss);
    }
}
#endif

void SMC_Utils::smc_shl(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modPow2(result, b);
    ss->modMul(result, a, result);
}

void SMC_Utils::smc_shl(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    if (blen == -1) {
        // b is PUBLIC, but stored in type priv_int
        ss->modPow2(result, b, size);
        ss->modMul(result, a, result, size);
    } else {
        doOperation_Pow2(result, b, blen, size, threadID, net, ss);
        Mult(result, result, a, size, threadID, net, ss);
    }
}

void SMC_Utils::smc_shl(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modPow2(result, b, size);
    ss->modMul(result, a, result, size);
}

// Dot Product
void SMC_Utils::smc_dot(priv_int *a, priv_int *b, int size, priv_int result, int threadID) {
    doOperation_DotProduct(a, b, result, size, threadID, net, ss);
}

void SMC_Utils::smc_dot(priv_int **a, priv_int **b, int size, int array_size, priv_int *result, std::string type, int threadID) {
    doOperation_DotProduct(a, b, result, size, array_size, threadID, net, ss);
}

// one-dimension private integer singular read
void SMC_Utils::smc_privindex_read(priv_int index, priv_int *array, priv_int result, int dim, std::string type, int threadID) {
    doOperation_PrivIndex_int(index, array, result, dim, 0, threadID, net, ss);
}

// two-dimension private integer singular read
void SMC_Utils::smc_privindex_read(priv_int index, priv_int **array, priv_int result, int dim1, int dim2, std::string type, int threadID) {
    doOperation_PrivIndex_int_arr(index, array, result, dim1, dim2, 0, threadID, net, ss);
}

// one-dimension private float singular read
void SMC_Utils::smc_privindex_read(priv_int index, priv_int **array, priv_int *result, int dim, std::string type, int threadID) {
    doOperation_PrivIndex_float(index, array, result, dim, 1, threadID, net, ss);
}

// two-dimension private float singular read
void SMC_Utils::smc_privindex_read(priv_int index, priv_int ***array, priv_int *result, int dim1, int dim2, std::string type, int threadID) {
    doOperation_PrivIndex_float_arr(index, array, result, dim1, dim2, 0, threadID, net, ss);
}

#if __SHAMIR__
// one-dimension private integer singular write
void SMC_Utils::smc_privindex_write(priv_int index, priv_int *array, int len_sig, int len_exp, int value, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write(MPZ_CAST(index), array, &value, dim, 1, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int index, priv_int *array, int len_sig, int len_exp, priv_int value, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {

    doOperation_PrivIndex_Write(MPZ_CAST(index), array, MPZ_CAST(value), dim, 1, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

// two-dimension private integer singular write
void SMC_Utils::smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, int value, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {

    doOperation_PrivIndex_Write_2d(MPZ_CAST(index), array, (int *)&value, dim1, dim2, 1, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, priv_int value, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write_2d(MPZ_CAST(index), array, MPZ_CAST(value), dim1, dim2, 1, out_cond, priv_cond, counter, threadID, 0, net, ss);
}
#endif

// one-dimension private float singular write
void SMC_Utils::smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, float value, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim, out_cond, priv_cond, counter, type, threadID, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, priv_int *value, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// two-dimension private float singular write
void SMC_Utils::smc_privindex_write(priv_int index, priv_int ***array, int len_sig, int len_exp, float value, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim1, dim2, out_cond, priv_cond, counter, type, threadID, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int index, priv_int ***array, int len_sig, int len_exp, priv_int *value, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim1, dim2, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// one-dimension private integer batch read
void SMC_Utils::smc_privindex_read(priv_int *indices, priv_int *arrays, priv_int *results, int dim, int size, std::string type, int threadID) {
    doOperation_PrivIndex_Read(indices, arrays, results, dim, size, threadID, 0, net, ss);
}

// two-dimension private integer batch read
void SMC_Utils::smc_privindex_read(priv_int *indices, priv_int **array, priv_int *results, int dim1, int dim2, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim1, dim2, size, type, threadID, net, ss);
}

// one-dimension private float batch read
void SMC_Utils::smc_privindex_read(priv_int *indices, priv_int **array, priv_int **results, int dim, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim, size, type, threadID, net, ss);
}

// two-dimension private float batch read
void SMC_Utils::smc_privindex_read(priv_int *indices, priv_int ***array, priv_int **results, int dim1, int dim2, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim1, dim2, size, type, threadID, net, ss);
}

// one-dimension private integer batch write
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int *array, int len_sig, int len_exp, int *values, int dim, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int *array, int len_sig, int len_exp, priv_int *values, int dim, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int **array, int len_sig, int len_exp, int *values, int dim1, int dim2, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// two-dimension private integer batch write
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int **array, int len_sig, int len_exp, priv_int *values, int dim1, int dim2, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// one-dimension private float batch write
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int **array, int len_sig, int len_exp, float *values, int dim, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int **array, int len_sig, int len_exp, priv_int **values, int dim, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// two-dimension private float batch write
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int ***array, int len_sig, int len_exp, float *values, int dim1, int dim2, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int ***array, int len_sig, int len_exp, priv_int **values, int dim1, int dim2, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, ss);
    ;
}

#if __SHAMIR__

// private pointer arithmetic only supported by shamirSS currently

priv_ptr SMC_Utils::smc_new_ptr(int level, int type) {
    return create_ptr(level, type);
}

priv_ptr *SMC_Utils::smc_new_ptr(int level, int type, int num) {
    return create_ptr(level, type, num);
}

/*************/
void SMC_Utils::smc_set_int_ptr(priv_ptr ptr, priv_int *var_loc, std::string type, int threadID) {
    set_ptr(ptr, var_loc, NULL, NULL, NULL, threadID);
}

void SMC_Utils::smc_set_float_ptr(priv_ptr ptr, priv_int **var_loc, std::string type, int threadID) {
    set_ptr(ptr, NULL, var_loc, NULL, NULL, threadID);
}

void SMC_Utils::smc_set_struct_ptr(priv_ptr ptr, void *var_loc, std::string type, int threadID) {
    set_ptr(ptr, NULL, NULL, var_loc, NULL, threadID);
}

void SMC_Utils::smc_set_ptr(priv_ptr assign_ptr, priv_ptr *ptr_loc, std::string type, int threadID) {
    set_ptr(assign_ptr, NULL, NULL, NULL, ptr_loc, threadID);
}

void SMC_Utils::smc_set_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, std::string type, int threadID) {
    set_ptr(assign_ptr, right_ptr, threadID);
}

void SMC_Utils::smc_set_ptr(priv_ptr ptr, int var_loc, std::string type, int threadID) {
}
/****************/
void SMC_Utils::smc_update_int_ptr(priv_ptr ptr, priv_int *var_loc, priv_int private_tag, int index, int threadID) {
    update_ptr(ptr, var_loc, NULL, NULL, NULL, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_update_float_ptr(priv_ptr ptr, priv_int **var_loc, priv_int private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, var_loc, NULL, NULL, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_update_struct_ptr(priv_ptr ptr, void *var_loc, priv_int private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, NULL, var_loc, NULL, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_update_ptr(priv_ptr ptr, priv_ptr *ptr_loc, priv_int private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, NULL, NULL, ptr_loc, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_update_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, priv_int private_tag, int index, int threadID) {
    update_ptr(assign_ptr, right_ptr, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_add_int_ptr(priv_ptr ptr, priv_int *var_loc, priv_int private_tag, int threadID) {
    add_ptr(ptr, var_loc, NULL, NULL, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_float_ptr(priv_ptr ptr, priv_int **var_loc, priv_int private_tag, int threadID) {
    add_ptr(ptr, NULL, var_loc, NULL, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_struct_ptr(priv_ptr ptr, void *var_loc, priv_int private_tag, int threadID) {
    add_ptr(ptr, NULL, NULL, var_loc, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_ptr(priv_ptr ptr, priv_ptr *ptr_loc, priv_int private_tag, int threadID) {
    add_ptr(ptr, NULL, NULL, NULL, ptr_loc, private_tag, threadID);
}

/*******************/
void SMC_Utils::smc_shrink_ptr(priv_ptr ptr, int current_index, int parent_index, int threadID) {
    shrink_ptr(ptr, current_index, parent_index, threadID);
}
/********************/
void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, priv_int result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID) {
    dereference_ptr_read_var(ptr, result, num_of_dereferences, threadID, net, ss);
}

void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, priv_int *result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID) {
    dereference_ptr_read_var(ptr, result, num_of_dereferences, threadID, net, ss);
}

void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, priv_ptr result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID) {
    dereference_ptr_read_ptr(ptr, result, num_of_dereferences, priv_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_int value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write_var(ptr, value, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_int_ptr(priv_ptr ptr, priv_int *value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, value, NULL, NULL, NULL, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_int *value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write_var(ptr, value, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_float_ptr(priv_ptr ptr, priv_int **value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, value, NULL, NULL, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_struct_ptr(priv_ptr ptr, void *value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, NULL, value, NULL, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_ptr *value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, NULL, NULL, value, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_ptr value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write_ptr(ptr, value, num_of_dereferences, private_cond, threadID, net, ss);
}
/*******************************************/
void SMC_Utils::smc_clear_ptr(priv_ptr *ptr) {
    clear_ptr(ptr);
}
void SMC_Utils::smc_free_ptr(priv_ptr *ptr) {
    destroy_ptr(ptr);
}

void SMC_Utils::smc_free_ptr(priv_ptr **ptrs, int num) {
    destroy_ptr(ptrs, num);
}

#endif

#if __SHAMIR__
// private float = private int
void SMC_Utils::smc_int2fl(priv_int value, priv_int *result, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, gamma, K, L, threadID, net, ss);
}
// private float = public int
void SMC_Utils::smc_int2fl(int value, priv_int *result, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, gamma, K, L, threadID, net, ss);
}
// private int = private int
void SMC_Utils::smc_int2int(priv_int value, priv_int result, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, gamma1, gamma2, threadID, net, ss);
}
// private int = public int
void SMC_Utils::smc_int2int(int value, priv_int result, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, gamma1, gamma2, threadID, net, ss);
}
// private int = private float
void SMC_Utils::smc_fl2int(priv_int *value, priv_int result, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, K, L, gamma, threadID, net, ss);
}
// private int = public float
void SMC_Utils::smc_fl2int(float value, priv_int result, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, K, L, gamma, threadID, net, ss);
}
// Public float casted to private float
void SMC_Utils::smc_fl2fl(float value, priv_int *result, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, K1, L1, K2, L2, threadID, net, ss);
}
// private float = public float
void SMC_Utils::smc_fl2fl(priv_int *value, priv_int *result, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, K1, L1, K2, L2, threadID, net, ss);
}
#endif
// private float* = private int* -> array
void SMC_Utils::smc_int2fl(priv_int *value, priv_int **result, int size, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, size, gamma, K, L, threadID, net, ss);
}
// private float* = public int* -> array
void SMC_Utils::smc_int2fl(int *value, priv_int **result, int size, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, size, gamma, K, L, threadID, net, ss);
}

// private int* = private int* -> array
void SMC_Utils::smc_int2int(priv_int *value, priv_int *result, int size, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, size, gamma1, gamma2, threadID, net, ss);
}
// private int* = public int* -> array
void SMC_Utils::smc_int2int(int *value, priv_int *result, int size, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, size, gamma1, gamma2, threadID, net, ss);
}

// private int* = private float* -> array
void SMC_Utils::smc_fl2int(priv_int **value, priv_int *result, int size, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, size, K, L, gamma, threadID, net, ss);
}
// private int* = public float* -> array
void SMC_Utils::smc_fl2int(float *value, priv_int *result, int size, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, size, K, L, gamma, threadID, net, ss);
}

// private float* = public float* -> array
void SMC_Utils::smc_fl2fl(priv_int **value, priv_int **result, int size, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, size, K1, L1, K2, L2, threadID, net, ss);
}
// Public float* casted to private float*
void SMC_Utils::smc_fl2fl(float *value, priv_int **result, int size, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, size, K1, L1, K2, L2, threadID, net, ss);
}

#if __SHAMIR__
/************************************ INTEGER BATCH ****************************************/
void SMC_Utils::smc_batch(priv_int *a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// used to compute 1-priv_cond in a batch stmt
void SMC_Utils::smc_batch(int a, priv_int *b, priv_int *result, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, int threadID) {
    ss_batch(a, b, result, out_cond, priv_cond, counter, index_array, size, op, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int *a, priv_int *b, priv_int *result, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, int threadID) {
    ss_batch(a, b, result, out_cond, priv_cond, counter, index_array, size, op, threadID, net, ss);
}

// first param: int array
// second param: int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(int *a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: int array
// second param: int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(int *a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: int array
// second param: one-dim private int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(int *a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: int array
// second param: one-dim private int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(int *a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: one-dim private int array
// second param: int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(priv_int *a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: one-dim private int array
// second param: int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(priv_int *a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: integer array
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(int *a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: integer array
// second param: two-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(int *a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: integer array
// assignment param: one-dim private int
void SMC_Utils::smc_batch(priv_int **a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: integer array
// assignment param: two-dim private int
void SMC_Utils::smc_batch(priv_int **a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: one-dim private int
// second param: two-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(priv_int *a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(priv_int **a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: one-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(priv_int *a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}
// first param: one-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(priv_int *a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(priv_int **a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: two-dim private int

void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

/*********************************************** FLOAT BATCH ****************************************************/
// public + private one-dimension float - arithmetic
void SMC_Utils::smc_batch(float *a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int **a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(float *a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int **a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// public to private assignments
void SMC_Utils::smc_batch(float *a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(int *a, int *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(float *a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(int *a, int *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// public + private two-dimension float - arithmetic
void SMC_Utils::smc_batch(float *a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int ***a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(float *a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int ***a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// public + private one-dimension float - comparison
void SMC_Utils::smc_batch(float *a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int **a, float *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// public + private two-dimension float - comparison
void SMC_Utils::smc_batch(float *a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int ***a, float *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(priv_int **a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}
// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(priv_int ***a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: two-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(priv_int ***a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(priv_int ***a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(priv_int **a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: one-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// two dimension float general computation
void SMC_Utils::smc_batch(priv_int ***a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// two-dimensional float comparison
void SMC_Utils::smc_batch(priv_int ***a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// one-dimensional float and two-dimensional float comparison
void SMC_Utils::smc_batch(priv_int **a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// two-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(priv_int ***a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// one-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}
#endif

void SMC_Utils::smc_batch_dot(priv_int **a, priv_int **b, int size, int array_size, int *index_array, priv_int *result, int threadID) {
    ss_batch_dot(a, b, size, array_size, index_array, result, threadID, net, ss);
}


/* conversion from public integer to private float*/
void SMC_Utils::smc_batch_int2fl(int *a, priv_int **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

/* conversion from private integer to private float */
// one-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(priv_int *a, priv_int **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2fl(int *a, priv_int ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

// one-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(priv_int *a, priv_int ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

// two-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(priv_int **a, priv_int **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

// two-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(priv_int **a, priv_int ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

/*FL2INT*/
void SMC_Utils::smc_batch_fl2int(float *a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(float *a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(priv_int **a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(priv_int **a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(priv_int ***a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(priv_int ***a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}
// INT2INT
void SMC_Utils::smc_batch_int2int(int *a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(int *a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(priv_int *a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(priv_int **a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(priv_int *a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(priv_int **a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

// FL2FL
void SMC_Utils::smc_batch_fl2fl(float *a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(float *a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(priv_int **a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(priv_int **a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(priv_int ***a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(priv_int ***a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

double SMC_Utils::time_diff(struct timeval *t1, struct timeval *t2) {
    double elapsed;
    if (t1->tv_usec > t2->tv_usec) {
        t2->tv_usec += 1000000;
        t2->tv_sec--;
    }
    elapsed = (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1000000.0;
    return elapsed;
}

std::string uint8PtrToString(const uint8_t *data, size_t length) {
    return std::string(data, data + length);
}

// seed_map - contains binary encodings of access sets T
// (as defined in Baccarini et al., "Multi-Party Replicated Secret Sharing over a Ring with Applications to Privacy-Preserving Machine Learning," 2023)
// 2*KEYSIZE - 16 bytes for
void SMC_Utils::seedSetup(std::vector<int> &seed_map, int peers, int threshold) {
    // this works now, since we are copying the data to a string, which is persistent in scope
    uint8_t RandomData_send[2 * KEYSIZE];
    uint8_t RandomData_recv[2 * KEYSIZE];
    printf("seedSetup\n");
#if __SHAMIR__
    std::vector<int> coefficients;
    // typedef int var_type; // this is here because I don't feel like refactoring shamirSS to use uints
#endif
#if __RSS__
    // typedef int var_type; // this is here because I don't feel like refactoring shamirSS to use uints
    // typedef uint var_type;
#endif

    std::vector<int> T_recv, send_map, recv_map; // declaring once to be reused
    std::vector<int> recv_map_original;
    // its fine to reuse vectors when inserting into the final mapping
    FILE *fp = fopen("/dev/urandom", "r"); // used for pulling good randomness for seeds
    try {
        for (auto &seed : seed_map) {

            send_map = extract_share_WITH_ACCESS(seed, peers, id);
            recv_map_original = extract_share_WITHOUT_ACCESS(seed, peers, id);       // equivalent to T_mine in the current iteration
            recv_map = extract_share_WITHOUT_ACCESS_new(seed, peers, id, threshold); // equivalent to T_mine in the current iteration

            if (fread(RandomData_send, 1, (2 * KEYSIZE), fp) != (2 * KEYSIZE))
                throw std::runtime_error("error reading random bytes from /dev/urandom. Which OS are you using?");

            assert(send_map.size() == recv_map.size());
            for (size_t i = 0; i < send_map.size(); i++) {
                // printf("sending to %i\n", send_map[i]);
                net.sendDataToPeer(static_cast<int>(send_map[i]), 2 * KEYSIZE, RandomData_send);
                // print_hexa_2(RandomData_send, 2 * KEYSIZE);
                // printf("recv from %i\n", recv_map[i]);
                net.getDataFromPeer(static_cast<int>(recv_map[i]), 2 * KEYSIZE, RandomData_recv);
                // print_hexa_2(RandomData_recv, 2 * KEYSIZE);

                // generating the share id T corresponding to the key I just recieved
                T_recv = extract_share_WITHOUT_ACCESS(seed, peers, recv_map[i]);
                std::vector<int> T_recv_test = extract_share_WITHOUT_ACCESS_new(seed, peers, recv_map[i], threshold);
                sort(T_recv.begin(), T_recv.end());
                sort(T_recv_test.begin(), T_recv_test.end());
#if __SHAMIR__
                coefficients = generateCoefficients(T_recv, threshold);
                shamir_seeds_coefs.insert(std::pair<std::string, std::vector<int>>(reinterpret_cast<char *>(RandomData_recv), coefficients));

#endif
#if __RSS__
                // cout << "inserting (original)" << T_recv << endl;
                // cout << "inserting (new)     " << T_recv_test << endl; // SHOULDNT BE USED
                // print_hexa_2(RandomData_recv, 2 * KEYSIZE);
                std::string result = uint8PtrToString(RandomData_recv, 2 * KEYSIZE);
                // rss_share_seeds.insert(std::pair<std::vector<int>, uint8_t *>(T_recv, RandomData_recv));
                rss_share_seeds.insert(std::pair<std::vector<int>, std::string>(T_recv, result));

#endif
            }
            // printf("\n");
            sort(recv_map.begin(), recv_map.end());                   // sorting now that we're done using it to know the order which we're recieving shares
            sort(recv_map_original.begin(), recv_map_original.end()); // sorting now that we're done using it to know the order which we're recieving shares
#if __SHAMIR__
            coefficients = generateCoefficients(recv_map, threshold);
            shamir_seeds_coefs.insert(std::pair<std::string, std::vector<int>>(reinterpret_cast<char *>(RandomData_send), coefficients));

#endif
#if __RSS__
            // cout << "inserting (recv_map_org)" << recv_map_original << endl;
            // cout << "inserting (recv_map_new)" << recv_map << endl;
            // print_hexa_2(RandomData_send, 2 * KEYSIZE);
            // rss_share_seeds.insert(std::pair<std::vector<int>, uint8_t *>(recv_map, RandomData_send));
            std::string result = uint8PtrToString(RandomData_send, 2 * KEYSIZE);
            rss_share_seeds.insert(std::pair<std::vector<int>, std::string>(recv_map_original, result));

            // printf("\n");
#endif
        }
        fclose(fp);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[seedSetup] " << ex.what() << "\n";
        exit(1);
    }

    // used for testing
    // int ctr = 0;
    // for (auto &[key, value] : rss_share_seeds) {
    //     std::cout << key << endl;
    //     ctr += 1;
    //     // print_hexa_2(value, 2*KEYSIZE);
    // }
    // std::cout << "num keys = " << ctr << std::endl;
}

// binary_rep = bianry encoding of share T generated from seed_map
// e.g. 0001111
// produces either the set of parties with access (set bits, 1s) or without access (unset bits, 0s)
// the LSB is IGNORED, since that corresponds to the party itself, and is ALWAYS SET
// id - my PID (1,...,n)
// result - UNsorted vector of party ID's with or without access
// ANB: w.r.t. sending keys for seeds, DONT CHANGE THE ORDER (yet)
// this explicitly informs us the exact order which shares will be sent/received from
std::vector<int> SMC_Utils::extract_share_WITH_ACCESS(int binary_rep, int peers, int id) {
    // iterate through bits from right to left
    std::vector<int> result;
    for (int i = 1; i < peers; i++) { // scans through remaining n-1 (LSB always set)
        if (GET_BIT(binary_rep, i)) {
            result.push_back(((id + i - 1) % peers + 1));
        }
    }
    return result;
}
// older version, gives incorrect results for some binary representations for >=7 parties
// replaced by "naive" approach in "new" function
std::vector<int> SMC_Utils::extract_share_WITHOUT_ACCESS(int binary_rep, int peers, int id) {
    // iterate through bits from left to right
    std::vector<int> result;
    for (int i = peers - 1; i > 0; i--) { // scans through remaining n-1 (LSB always set)
        if (!GET_BIT(binary_rep, i)) {
            result.push_back(((id + i - 1) % peers + 1));
        }
    }
    return result;
}

std::vector<int> SMC_Utils::extract_share_WITHOUT_ACCESS_new(int binary_rep, int peers, int id, int threshold) {
    // first getting the sendIDs
    int tmp = 0;
    int idx = 0;
    std::vector<int> result(threshold);
    for (int peer = 1; peer <= peers; peer++) {
        // std::cout << "peer: " << peer << std::endl;
        idx = 0;
        for (int i = 1; i < peers; i++) { // scans through remaining n-1 (LSB always set)
            // std::cout << "i : " << i << std::endl;
            tmp = ((peer + i - 1) % peers + 1);
            if (GET_BIT(binary_rep, i)) {
                if (tmp == id) {
                    // std::cout << "idx : " << idx << std::endl;
                    // std::cout << "peer: " << peer << std::endl;
                    result.at(idx) = peer;
                    // std::cout << "result: " << result << std::endl;
                    break;
                } else {
                    idx += 1;
                }
            }
        }
    }
    return result;
}

// taken from PRSS in seed/
void SMC_Utils::getCombinations(std::vector<int> &elements, int reqLen, std::vector<int> &pos, int depth, int margin, std::vector<std::vector<int>> &result) {
    if (depth >= reqLen) {
        std::vector<int> temp;
        for (unsigned int i = 0; i < pos.size(); i++)
            temp.push_back(elements.at(pos.at(i)));
        result.push_back(temp);
        return;
    }
    if ((elements.size() - margin) < (unsigned int)(reqLen - depth))
        return;
    for (unsigned int i = margin; i < elements.size(); i++) {
        pos.at(depth) = i;
        getCombinations(elements, reqLen, pos, depth + 1, i + 1, result);
    }
    return;
}

// used for Shamir secret sharing only, not applicable for RSS
// generates the polynomials described in  section 3.1 of Cramer et al. TCC'05 paper
// one polynomial for maximum unqualified set (T_set)
// T_set is generated from the current seed and the party id of who I'm recieving a key from (or myself if it's the key I generated)
// threshold is the degree of the polynomial
std::vector<int> SMC_Utils::generateCoefficients(std::vector<int> T_set, int threshold) {
    std::vector<int> coefficients = {1}; // the first coefficient is always 1

    transform(T_set.begin(), T_set.end(), T_set.begin(), std::bind(std::multiplies<int>(), std::placeholders::_1, -1)); // negating every element in T_set (only within the scope of this function since it's passed by value

    for (int j = 1; j <= threshold; j++) {
        std::vector<int> pos(j);
        std::vector<std::vector<int>> result;
        getCombinations(T_set, j, pos, 0, 0, result);
        int coef = 0;
        for (unsigned int m = 0; m < result.size(); m++) {
            std::vector<int> temp = result.at(m);
            int tmp = 1;
            for (unsigned int n = 0; n < temp.size(); n++) {
                tmp *= temp.at(n);
            }
            coef += tmp;
        }
        coefficients.push_back(coef);
    }
    return coefficients;
}

// std::vector<int> generateCoefficients(std::vector<int> T_set, int threshold);

#if __RSS__
uint SMC_Utils::getNumShares() {
    return ss->getNumShares();
}
#endif

