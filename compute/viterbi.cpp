// g++ viterbi.cpp -o viterbi
// ./viterbi

#include <iostream>
using namespace std;

const int N = 3; // Number of states
const int MM = 32; // Number of observation symbols
const int T = 32; // Length of the observation sequence

void viterbi_algorithm(float a[N][N], float bb[N][MM], int x[T], float pi[N]) {
    // Initialization
    float V[T][N];
    int B[T][N];

    // Initialize Viterbi matrix
    for (int i = 0; i < N; i++) {
        V[0][i] = pi[i] * bb[i][x[0]];
        B[0][i] = 0;
    }

    // Recursion
    for (int t = 1; t < T; t++) {
        for (int j = 0; j < N; j++) {
            float max_prob = 0.0;
            int max_index = 0;
            for (int i = 0; i < N; i++) {
                float prob = V[t - 1][i] * a[i][j] * bb[j][x[t]];
                if (prob > max_prob) {
                    max_prob = prob;
                    max_index = i;
                }
            }
            V[t][j] = max_prob;
            B[t][j] = max_index;
        }
    }

    // Termination
    float max_final_prob = 0.0;
    int max_final_index = 0;
    for (int i = 0; i < N; i++) {
        if (V[T - 1][i] > max_final_prob) {
            max_final_prob = V[T - 1][i];
            max_final_index = i;
        }
    }

    // Backtrack to find the most likely sequence
    int max_sequence[T];
    max_sequence[T - 1] = max_final_index;
    for (int t = T - 2; t >= 0; t--) {
        max_sequence[t] = B[t + 1][max_sequence[t + 1]];
    }

    // Output the max probability and the correct index of the max sequence
    cout << "Max Probability: " << max_final_prob << endl;
    cout << "Correct Index of Max Sequence: ";
    for (int t = 0; t < T; t++) {
        cout << max_sequence[t] << " ";
    }
    cout << endl;
}

int main() {
    // Define transition probabilities
    float a[N][N] = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};

    // Define emission probabilities
    float bb[N][MM] = {
        {0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.1, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.1, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.1, 0.4, 0.5, 0.6}, 
        {0.7, 0.2, 0.3, 0.4, 0.1, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.1, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.1, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2}, 
        {0.3, 0.4, 0.1, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.1, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.1, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4}
    }; 

    // Define observation sequence
    int x[T] = {5, 10, 15, 20, 25, 0, 1, 2, 3, 4, 11, 12, 13, 14, 21, 22, 23, 24, 26, 27, 28, 29, 30, 31, 6, 7, 8, 9, 16, 17, 18, 19};

    // Define initial probabilities
    float pi[N] = {0.1, 0.2, 0.3}; 

    // Run Viterbi algorithm
    viterbi_algorithm(a, bb, x, pi);

    return 0;
}
