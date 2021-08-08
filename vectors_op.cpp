// Marcin Bieganek 
#include "router.hpp"

void create_new_distv_elem(struct vector_elem *new_elem, uint32_t dist, uint8_t prefix_len, 
                            struct in_addr network, struct in_addr via_ip_addr) {
    new_elem->indirectly = true;
    new_elem->dist = dist;
    new_elem->prefix_len = prefix_len;
    new_elem->network = network;
    new_elem->broadcast = broadcast_ip_addr(new_elem->network, new_elem->prefix_len);
    new_elem->via_ip_addr = via_ip_addr;
    new_elem->rounds_since_last_info = 0;
    new_elem->rounds_with_dist_eq_inf = 0;
    new_elem->send_info_about = true;
}

uint32_t find_current_dist(vector<struct vector_elem> dist_vect, struct in_addr ip_addr) {
    uint32_t dist = INF;
    for (size_t i = 0; i < dist_vect.size(); i++)
        if (ip_addr_in_network(ip_addr, dist_vect[i].network, dist_vect[i].prefix_len))
            dist = dist_vect[i].dist;
    return dist;
}
// Szuka wpisu o sieci do ktorej nalezy podany adres, zwraca indeks lub -1, gdy nie ma
// odpowiadajacego wpisu.
int find_index_for_ip_addr(vector<struct vector_elem> dist_vect, struct in_addr ip_addr) {
    int index = -1;
    for (size_t i = 0; i < dist_vect.size(); i++)
        if (ip_addr_in_network(ip_addr, dist_vect[i].network, dist_vect[i].prefix_len))
            index = i;
    return index;
}

bool is_packet_from_me(vector<struct vector_elem> dist_vect, struct in_addr sender_ip) {
    for (size_t i = 0; i < dist_vect.size(); i++)
        if (dist_vect[i].indirectly == false)
            if (dist_vect[i].ip_addr.s_addr == sender_ip.s_addr)
                return true;
    return false;
}

void set_unreachable(vector<struct vector_elem> *dist_vect, int direct_index, vector<struct direct_elem> *direct_vect) {
    // aktualizacja w wektorze odlegosci
    for (size_t k = 0; k <(*dist_vect).size(); k++) {
        // aktualizacja odleglosci do tej sieci
        if ((*dist_vect)[k].network.s_addr == (*direct_vect)[direct_index].network.s_addr
            && (*dist_vect)[k].prefix_len == (*direct_vect)[direct_index].prefix_len) {
            (*dist_vect)[k].dist = INF;
        }
        // aktualizacja odleglosci do sieci do ktorych sciezka prowadzi przez ta siec
        if ((*dist_vect)[k].indirectly) {
            if (ip_addr_in_network((*dist_vect)[k].via_ip_addr, (*direct_vect)[direct_index].network, (*direct_vect)[direct_index].prefix_len))
                (*dist_vect)[k].dist = INF;
        }
    }
}

void set_reachable(vector<struct vector_elem> *dist_vect, int direct_index, vector<struct direct_elem> *direct_vect) {
    int dist_index = find_index_for_ip_addr((*dist_vect), (*direct_vect)[direct_index].network);
    //printf("SET REACHABLE = %d\n", dist_index);
    if (dist_index >= 0) {
        (*dist_vect)[dist_index].indirectly = false;
        (*dist_vect)[dist_index].dist = (*direct_vect)[direct_index].dist;
        (*dist_vect)[dist_index].send_info_about = true;
        (*dist_vect)[dist_index].rounds_since_last_info = 0;
        (*dist_vect)[dist_index].rounds_with_dist_eq_inf = 0;
    }
}

bool network_in_direct(vector<struct direct_elem> direct_vect, struct in_addr network) {
    for (size_t i = 0; i < direct_vect.size(); i++)
        if (direct_vect[i].network.s_addr == network.s_addr)
            return true;
    return false;
}