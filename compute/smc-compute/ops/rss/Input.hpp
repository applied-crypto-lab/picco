#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

// version of Input where t parties are inputting values into the computation
// this implementation is **independent** of the number of parties in the computation
// if pid is NOT an input party, then input is (void)
// input dimensiopns : [size]
// result dimensions : [numShares][input_parties.size()][size] (this matches the convention used in RSS_GenerateRandomShares_*pc)
// we, buy default, set T* to be the canonnically "first" share that the input party possesses (and is thus the "computed" share)
// which is dictated by T_map (defined in RepSecretShare)
// example input_parties  = {1, 2, 3} (for 7pc B2A)
template <typename T>
void Rss_Input_p_star(T ***result, T *input, std::vector<uint> input_parties, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numInputParties = input_parties.size();
    uint numShares = ss->getNumShares();
    uint bytes = (ring_size + 7) >> 3;
    int pid = ss->getID();

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < numInputParties; i++) {
            // sanitizing destination
            // potentially not needed?, given the context of which this is called (inside B2A, where we created the destination variables themselves)
            memset(result[s][i], 0, sizeof(T) * size);
        }
    }

    T **recvbuf = new T *[numInputParties];
    for (size_t i = 0; i < numInputParties; i++) {
        recvbuf[i] = new T[size];
        memset(recvbuf[i], 0, sizeof(T) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[bytes * size];
    }

    int T_star_index;

    // instead of generating T_star, just get the index in T_map where it exists (different for each party)
    // iterating through every input party
    for (auto pidp : input_parties) {

        T_star_index = ss->generateT_star_index(p_star);
        // iterating through every set T in mapping T_map_mpc (every share, exclusing T^*)
        // calling the PRGs whenever applicable
        for (uint s = 0; s < numShares; s++) {
            if (s != T_star_index and (std::find(ss->T_map_mpc[s].begin(), ss->T_map_mpc[s].end(), p_star) != ss->T_map_mpc[s].end())) {
                ss->prg_getrandom(s, bytes, size, buffer[s]);
                for (size_t i = 0; i < size; i++) {
                    memcpy(result[s][p_star] + i, buffer[s] + i * bytes, bytes); // copying bytes amount of randomness into result[s][p_star][i]
                }
            }
        }

        // only to be performed by the current input party (p_star) we are dealing with in this iteration
        // p_star will compute the last share T^*
        // THIS IS CONTINGENT ON result BEING ZEROED !!!!!!
        if (p_star == pid) {
            // very small number of if statements are being executed, no degradation to performance
            for (uint s = 0; s < numShares; s++) {
                if (s != T_star_index) {
                    for (size_t i = 0; i < size; i++) {
                        result[T_star_index][p_star][i] -= result[s][p_star][i];
                    }
                } else { 
                    // only happens once, p_star adding the value which is being inputted to share T_star
                    for (size_t i = 0; i < size; i++) {
                        result[T_star_index][p_star][i] += input[i];
                    }
                }
            }
        }
        // no need to modify send buffer -> can just pass the array of T_star shares pid needs to send into the send-recv function (just as in edabit)
        // However, will still need to extract the shares received into the correct locations
    }

        T_star_index = ss->generateT_star_index(pid);

    nodeNet.SendAndGetDataFromPeer(result[p_index], recvbuf, new_size, ring_size, ss->eda_map_mpc );
    for (size_t i = 0; i < numInputParties; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;
    for (size_t i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
}

#endif // _INPUT_HPP_