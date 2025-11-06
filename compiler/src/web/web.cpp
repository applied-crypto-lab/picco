/*
This code will do the following stuff:
The mode flag in here is used to split the generating the input and running the server part on this code. 
1. Read the argument list that is given by the user
    a. There would be three important info here:
        ./../compiler/bin/picco-web -I -d 1 public_01.pem test_utility_conf test_util_shares_A
    b. Store the name test_util_shares_A and send it to the server on step 3. Cause the sever will create three files
        with this name

2. Read test_utility_conf and generate a JSON file based on it

3. Run the server and get the inputs
    - The rest will be hanled by the server
*/

#include <iostream>   // For std::cout, std::cerr, std::endl
#include <string>     // For std::string
#include <vector>     // For std::vector
#include <fstream>    // For std::ifstream, std::ofstream
#include <sstream>    // For std::stringstream
#include <cstring>    // For strtok, strcpy, sscanf, strcmp, strdup
#include <cstdlib>    // For atoi, atoll, exit, system, malloc, free
#include <cstdio>     // For FILE, fopen, fclose, fprintf, perror, printf
#include <iomanip>    // For std::setw, std::setfill (for formatting numbers)
#include <algorithm>  // For std::replace (if needed, but not directly used in final JSON escape)

// --- Struct Definitions ---
typedef struct {
    int technique;
    int bits;
    int peers;
    int threshold;
    int inputs;
    int outputs;
    long long modulus;
} Settings;

typedef struct {
    char name[100];
    char type[50];
    int input_party;
    int size1;
    int size2;
    int dimensions;
    int bit_len1;
    int bit_len2;
} Variable_list;

// --- Placeholder for GMP library functions ---
#ifndef __GMP_H__
struct __mpz_struct {
    void * _mp_d;
    int _mp_alloc;
    int _mp_size;
};
typedef __mpz_struct mpz_t[1];
extern "C" {
    void mpz_init(mpz_t /*z*/) {}
    char* mpz_get_str(char* /*str*/, int /*base*/, mpz_t /*op*/) { return strdup("0"); }
    void getPrime(mpz_t /*rop*/, int /*bits*/) {}
}
#define SHAMIR_SS 2 // Define SHAMIR_SS if not defined elsewhere
#else
// Assume <gmp.h> is included and functions are linked if __GMP_H__ is defined
#endif

// --- Helper function to read file content into a string ---
std::string read_file_content(const std::string& filepath) {

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::string content = buffer.str();

    // --- NEW: Remove trailing newline/carriage return from public key content ---
    // Remove the last character if it's a newline
    if (!content.empty() && content.back() == '\n') {
        content.pop_back();
    }
    // Remove a potential carriage return if it precedes the newline (for Windows newlines)
    if (!content.empty() && content.back() == '\r') {
        content.pop_back();
    }
    // --- END NEW ---

    return content;
}

// --- Parsing Functions (UPDATED `parse_variables` with your specific dimension logic) ---
void parse_variables(const char *input[], int count, Variable_list vars[], int *var_count) {
    *var_count = 0;
    for (int i = 0; i < count; i++) {
        char temp[256];
        strncpy(temp, input[i], sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';

        if (temp[0] != 'I') {
            continue;
        }

        // Use a mutable copy for strtok
        char* line_copy = strdup(temp); // Duplicate the string as strtok modifies it
        if (!line_copy) {
            std::cerr << "Error: Memory allocation failed for line_copy." << std::endl;
            exit(1);
        }

        char *token = strtok(line_copy, ",");
        if (token == NULL) { free(line_copy); continue; }
        token = strtok(NULL, ",");

        if (token == NULL) { free(line_copy); continue; }
        strncpy(vars[*var_count].name, token, sizeof(vars[*var_count].name) - 1);
        vars[*var_count].name[sizeof(vars[*var_count].name) - 1] = '\0';

        token = strtok(NULL, ",");
        if (token == NULL) { free(line_copy); continue; }
        strncpy(vars[*var_count].type, token, sizeof(vars[*var_count].type) - 1);
        vars[*var_count].type[sizeof(vars[*var_count].type) - 1] = '\0';

        token = strtok(NULL, ",");
        if (token == NULL) { free(line_copy); continue; }
        vars[*var_count].input_party = atoi(token);

        std::vector<int> values_vec;
        while ((token = strtok(NULL, ",")) != NULL) {
            values_vec.push_back(atoi(token));
        }

        int num_bitlens = (strcmp(vars[*var_count].type, "float") == 0) ? 2 : 1;
        int value_count = values_vec.size();
        int num_sizes = value_count - num_bitlens;

        // --- Original logic to set size1 and size2 ---
        if (num_sizes <= 0) {
            vars[*var_count].size1 = 0;
            vars[*var_count].size2 = 0;
        } else if (num_sizes == 1) {
            vars[*var_count].size1 = values_vec[0];
            vars[*var_count].size2 = 0;
        } else if (num_sizes >= 2) {
            vars[*var_count].size1 = values_vec[0];
            vars[*var_count].size2 = values_vec[1];
        }

        // Setting dimensions
        vars[*var_count].dimensions = 0;

        if (vars[*var_count].size1 == 1 && vars[*var_count].size2 == 0) { // No sizes exists (scalar)
            vars[*var_count].dimensions = 0;
        } else if (vars[*var_count].size2 == 0) { // Size 1 exists (1D array)
            vars[*var_count].dimensions = 1;
        } else if (vars[*var_count].size1 != 0 && vars[*var_count].size2 != 0) { // Both sizes exists (2D array)
            vars[*var_count].dimensions = 2;
        } else {
            vars[*var_count].dimensions = 0;
        }

        if (strcmp(vars[*var_count].type, "float") == 0) {
            vars[*var_count].bit_len1 = (value_count > 1) ? values_vec[value_count - 2] : 0;
            vars[*var_count].bit_len2 = (value_count > 0) ? values_vec[value_count - 1] : 0;
        } else {
            vars[*var_count].bit_len1 = (value_count > 0) ? values_vec[value_count - 1] : 0;
            vars[*var_count].bit_len2 = 0;
        }

        (*var_count)++;
        free(line_copy); // Free the duplicated string
    }
}

void parse_settings(const char *var_list_arr[], int size, Settings *settings) {
    settings->technique = 0;
    settings->bits = 0;
    settings->peers = 0;
    settings->threshold = 0;
    settings->inputs = 0;
    settings->outputs = 0;
    settings->modulus = 0;

    for (int i = 0; i < size; i++) {
        if (var_list_arr[i][0] == 'I' || var_list_arr[i][0] == 'O')
            break;

        char key[50];
        char value_str[50];

        // Use a mutable copy for strtok
        char* line_copy = strdup(var_list_arr[i]);
        if (!line_copy) {
            std::cerr << "Error: Memory allocation failed for line_copy in parse_settings." << std::endl;
            exit(1);
        }

        if (sscanf(line_copy, "%49[^:]:%49s", key, value_str) == 2) {
            if (strcmp(key, "technique") == 0) settings->technique = atoi(value_str);
            else if (strcmp(key, "bits") == 0) settings->bits = atoi(value_str);
            else if (strcmp(key, "peers") == 0) settings->peers = atoi(value_str);
            else if (strcmp(key, "threshold") == 0) settings->threshold = atoi(value_str);
            else if (strcmp(key, "inputs") == 0) settings->inputs = atoi(value_str);
            else if (strcmp(key, "outputs") == 0) settings->outputs = atoi(value_str);
            else if (strcmp(key, "modulus") == 0) settings->modulus = atoll(value_str);
        }
        free(line_copy); // Free the duplicated string
    }
}

// Function to run the Python server with share_base_name, input_config_json, and passcode_file
void run_python_server(const std::string& share_name,
                       const std::string& input_config_json,
                       const std::string& passcode_file, const std::string& host,
                       int port) {
    // Construct command with all three arguments
    // Redirect stdout to server.log and stderr to stdout, then run in background
    std::string command = "python3 ../compiler/src/web/server.py " + share_name + " " + input_config_json + " " + passcode_file + " " + host + " " +
                          std::to_string(port) + " > server.log 2>&1 &";

    // Execute the command
    int result = system(command.c_str());

    // Check exit status of the 'system' call itself (not the background process)
    if (result == -1) {
        std::cerr << "Error: Failed to execute system command for Python server." << std::endl;
    } else {
        std::cout << "Python server started in the background. Check server.log for its output and access logs." << std::endl;
    }
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "Error: missing flag (-G or -S)" << std::endl;
        return 1;
    }

    // Set the mode 
        // 1. Generate the server mode (-G) → Generate the JSON file but do not run the server.
        // 2. Run the server mode (-S) → Skip JSON generation and only run the server.
    int mode;
    if (!strcmp(argv[1], "-G") || !strcmp(argv[1], "-g"))
        mode = 1;
    if (!strcmp(argv[1], "-S") || !strcmp(argv[1], "-s"))
        mode = 2;

    // If case 1: Usage: picco-web -G <utility_config> <input_config_json> <public_key_file1> <public_key_file2> <public_key_file3> ...
    // If case 2: picco-web -S <host> <port>  <input_config_json> <passcode_file> <share_base_name>

    if (mode == 1) {
        if (argc < 7) { // Requires at least 7 arguments (executable + 6 parameters) -> considering that the smallest number of parties allowed is 3
            std::cerr << "Incorrect input parameters." << std::endl;
            std::cerr << "Usage: picco-web -G <utility_config> <input_config_json> <public_key_file1> <public_key_file2> <public_key_file3> ..." << std::endl;
            return 1;
        }

        // Extract input filenames from arguments
        std::string utility_config_path = argv[2];        // Config file with peers, bits, etc.
        std::string input_config_json = argv[3];          // Name of the JSON file to write

        // Public keys start from argv[8] to argv[argc - 1]
        std::vector<std::string> public_key_filepaths;
        for (int i = 4; i < argc; ++i) {
            public_key_filepaths.push_back(argv[i]);
        }

        // Read configuration lines from utility config file
        std::vector<std::string> config_lines;
        std::ifstream config_file(utility_config_path);
        if (!config_file.is_open()) {
            std::cerr << "Error: Could not open utility configuration file: " << utility_config_path << std::endl;
            return 1;
        }
        std::string line;
        while (std::getline(config_file, line)) {
            config_lines.push_back(line); // Store each line from config file
        }
        config_file.close();

        // Convert config lines to C-style string array
        std::vector<const char*> c_style_config_lines;
        for (const auto& s : config_lines) {
            c_style_config_lines.push_back(s.c_str());
        }

        // Parse configuration to extract settings like peers, bit length, threshold, etc)
        Settings settings;
        parse_settings(c_style_config_lines.data(), c_style_config_lines.size(), &settings);

        // Validate peer count
        if (settings.peers <= 0) {
            std::cerr << "Error: 'peers' setting not found or is invalid (<= 0) in " << utility_config_path << std::endl;
            return 1;
        }

        // Check the arguments and print a message if the number of args does not match (for the exact match)
        // Validate number of public key files matches settings.peers
        if ((int)public_key_filepaths.size() != settings.peers) {
            std::cerr << "Error: Number of public key files provided (" << public_key_filepaths.size()
                    << ") does not match 'peers' setting (" << settings.peers << ")." << std::endl;
            return 1;
        }

        // Parse declared variables from config file
        Variable_list vars[100];
        int var_count = 0;
        parse_variables(c_style_config_lines.data(), c_style_config_lines.size(), vars, &var_count);

        // Open the JSON file using the name provided by the user (argv[2]) 
        FILE *json_file = fopen(input_config_json.c_str(), "w"); // << updated to use user input
        if (!json_file) {
            perror(("Failed to open " + input_config_json).c_str());
            return 1;
        }

        fprintf(json_file, "{\n");
        fprintf(json_file, "  \"settings\": [\n");
        fprintf(json_file, "    {\n");
        fprintf(json_file, "      \"technique\": %d,\n", settings.technique);
        fprintf(json_file, "      \"bits\": %d,\n", settings.bits);
        fprintf(json_file, "      \"peers\": %d,\n", settings.peers);
        fprintf(json_file, "      \"threshold\": %d,\n", settings.threshold);
        fprintf(json_file, "      \"inputs\": %d,\n", settings.inputs);
        fprintf(json_file, "      \"outputs\": %d,\n", settings.outputs);
        fprintf(json_file, "      \"modulus\": %lld\n", settings.modulus);
        fprintf(json_file, "    }\n");
        fprintf(json_file, "  ],\n");

        // --- "peers" array with public key content (UPDATED JSON escaping) ---
        fprintf(json_file, "  \"peers\": [\n");
        for (size_t i = 0; i < public_key_filepaths.size(); ++i) {
            int peer_number = i + 1; // Peers are 1-indexed

            std::string public_key_content = read_file_content(public_key_filepaths[i]);

            // JSON escaping for string values: backslashes, double quotes, newlines, carriage returns
            std::string escaped_content = public_key_content;
            size_t pos = 0;

            // Escape backslashes first
            while ((pos = escaped_content.find('\\', pos)) != std::string::npos) {
                escaped_content.insert(pos, "\\");
                pos += 2;
            }

            // Escape double quotes
            pos = 0;
            while ((pos = escaped_content.find('\"', pos)) != std::string::npos) {
                escaped_content.insert(pos, "\\");
                pos += 2;
            }

            // Escape newlines
            pos = 0;
            while ((pos = escaped_content.find('\n', pos)) != std::string::npos) {
                escaped_content.replace(pos, 1, "\\n");
                pos += 2;
            }

            // Escape carriage returns
            pos = 0;
            while ((pos = escaped_content.find('\r', pos)) != std::string::npos) {
                escaped_content.replace(pos, 1, "\\r");
                pos += 2;
            }

            // Write peer object
            fprintf(json_file, "    {\n");
            fprintf(json_file, "      \"peer\": %d,\n", peer_number);
            fprintf(json_file, "      \"public_key\": \"%s\"\n", escaped_content.c_str());
            fprintf(json_file, "    }%s\n", (i == public_key_filepaths.size() - 1) ? "" : ",");
        }
        fprintf(json_file, "  ],\n");

        // Write the "variables" section
        fprintf(json_file, "  \"variables\": [\n");
        for (int i = 0; i < var_count; i++) {
            fprintf(json_file, "    {\n");
            fprintf(json_file, "      \"name\": \"%s\",\n", vars[i].name);
            fprintf(json_file, "      \"display_name\": \"%s\",\n", vars[i].name);
            fprintf(json_file, "      \"type\": \"%s\",\n", vars[i].type);
            fprintf(json_file, "      \"bit_len1\": %d,\n", vars[i].bit_len1);
            fprintf(json_file, "      \"bit_len2\": %d,\n", vars[i].bit_len2);
            fprintf(json_file, "      \"size1\": %d,\n", vars[i].size1);
            fprintf(json_file, "      \"size2\": %d,\n", vars[i].size2);
            fprintf(json_file, "      \"dimension\": %d,\n", vars[i].dimensions);
            fprintf(json_file, "      \"input_party\": %d\n", vars[i].input_party);
            fprintf(json_file, "    }%s\n", (i == var_count - 1) ? "" : ",");
        }
        fprintf(json_file, "  ]\n");
        fprintf(json_file, "}\n");

        // Close JSON file
        fclose(json_file);

        printf("JSON file '%s' has been created successfully.\n", input_config_json.c_str());

    } else if (mode == 2) {
        if (argc < 7) { // Requires at least 7 arguments (executable + 6 parameters) 
            std::cerr << "Incorrect input parameters." << std::endl;
            std::cerr << "Usage: picco-web -S <host> <port>  <input_config_json> <passcode_file> <share_base_name>" << std::endl;
            return 1;
        }

        // Extract input filenames from arguments
        std::string host = argv[2];
        int port = std::stoi(argv[3]);
        std::string input_config_json = argv[4];          // Name of the JSON file to write
        std::string passcode_file = argv[5];              // File to send to server
        std::string share_base_name = argv[6];            // Base name to send to server

        // --- Run the Python Server ---
        run_python_server(share_base_name, input_config_json, passcode_file, host, port);
    }

    return 0;
}
