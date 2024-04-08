#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

std::vector<std::vector<int>> generateInputSendRecvMap(std::vector<int> input_parties, vector<int> T_star);

/*
 * version of Input where t parties are inputting values into the computation
 * this implementation is **independent** of the number of parties in the computation
 * if pid is NOT an input party, then the argument (T *input) is (void/null/whatever)
 * numInputParties = input_parties.size()
 * input dimensiopns : [size]
 * result dimensions : [numInputParties][numShares][size]
 * This is a different/better representation than what is used in edaBit, since it allows us to test individual outputs, while keeping the structure well-formed
 * e.g.,
 * we, buy default, set T* to be the canonnically "first" share that the input party possesses (and is thus the "computed" share)
 * which is dictated by T_map (defined in RepSecretShare)
 * example input_parties  = {1, 2, 3} (for 7pc B2A)
 * we assume input_parties is in increasing order
 * we also assume that the mapping that is (now) an argument is defined inside whatever protocol is calling Input^*, and is *well-formed*
 * meaning no *race conditions*, where two parties are sending to the same destination party in the same round (and similarly for receiving)
 */
template <typename T>
void Rss_Input_p_star(T ***result, T *input, std::vector<int> input_parties, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // uint numInputParties = input_parties.size();
    uint numShares = ss->getNumShares();
    uint bytes = (ring_size + 7) >> 3;
    int pid = ss->getID();
    std::vector<std::vector<int>> send_recv_map = ss->generateInputSendRecvMap(input_parties);
    // printf("---\nsend_recv_map\n");
    // std::cout << "send : "<< send_recv_map[0] << std::endl;
    // std::cout << "recv : "<< send_recv_map[1] << std::endl;

    // allocated based on the size of recv_map()
    T **recvbuf = new T *[send_recv_map[1].size()];
    for (size_t i = 0; i < send_recv_map[1].size(); i++) {
        recvbuf[i] = new T[size];
        memset(recvbuf[i], 0, sizeof(T) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[bytes * size];
    }
    // instead of generating T_star, just get the index in T_map where it exists (different for each party)
    // iterating through every input party
    int T_star_index;
    int p_star_index = 0;
    int my_index = 0;
    int my_T_star_index = 0;
    for (auto p_star : input_parties) {
        T_star_index = ss->generateT_star_index(p_star);
        // iterating through every set T in mapping T_map_mpc (every share, exclusing T^*)
        // calling the PRGs whenever applicable
        for (uint s = 0; s < numShares; s++) {
            if (s != T_star_index and !(std::find(ss->T_map_mpc[s].begin(), ss->T_map_mpc[s].end(), p_star) != ss->T_map_mpc[s].end())) {
                ss->prg_getrandom(s, bytes, size, buffer[s]);
                for (size_t i = 0; i < size; i++) {
                    memcpy(result[p_star_index][s] + i, buffer[s] + i * bytes, bytes); // copying bytes amount of randomness into result[p_star][s][i]
                }
            }
        }

        // only to be performed by the current input party (p_star) we are dealing with in this iteration
        // p_star will compute the last share T^*
        // THIS IS CONTINGENT ON result BEING ZEROED !!!!!!
        if (p_star == pid) {
            my_index = p_star_index;        // storing the index of myself
            my_T_star_index = T_star_index; // storing the index of my T_star
            // very small number of if statements are being executed, no degradation to performance
            for (uint s = 0; s < numShares; s++) {
                if (s != T_star_index) {
                    for (size_t i = 0; i < size; i++) {
                        result[my_index][T_star_index][i] -= result[my_index][s][i];
                    }
                } else {
                    // only happens once, p_star adding the value which is being inputted to share T_star
                    for (size_t i = 0; i < size; i++) {
                        result[my_index][T_star_index][i] += input[i];
                    }
                }
            }
        }
        p_star_index += 1;
        // no need to modify send buffer -> can just pass the array of T_star shares pid needs to send into the send-recv function (just as in edabit)
        // However, will still need to extract the shares received into the correct locations
    }
    // sending everything in the result[my_index][my_T_star_index]
    nodeNet.SendAndGetDataFromPeer(result[my_index][my_T_star_index], recvbuf, size, ring_size, send_recv_map);
    /*
    the order which data is received into recvbuf corresponds to the recv component of send_recv_map
    e.g. supposed input_parties = {1,2,3}
    5 receives from parties 2 and 1 (in that order)
    p5 will iterate through recv_map, then through input_parties
    if a match is found (e.g., we first find "2" at index 1 in input_parties), this informs us that everything in recv_buff at position[1] is from party 2
     */
    // iterate through recv_map, look for match in input_parties
    p_star_index = 0;
    for (auto p_star : input_parties) {
        // used to determine correct index of T_star (different for each party)
        T_star_index = ss->generateT_star_index(p_star);
        for (size_t recv_id_idx = 0; recv_id_idx < send_recv_map[1].size(); recv_id_idx++) {
            if (p_star == send_recv_map[1][recv_id_idx]) {
                // getting the index of the share T_star
                T_star_index = ss->generateT_star_index(p_star);
                memcpy(result[p_star_index][T_star_index], recvbuf[recv_id_idx], sizeof(T) * size);
            }
        }
        p_star_index += 1;
    }

    for (size_t i = 0; i < send_recv_map[1].size(); i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;
    for (size_t i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
}

#endif // _INPUT_HPP_