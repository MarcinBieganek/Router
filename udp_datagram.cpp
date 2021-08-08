// Marcin Bieganek 
#include "router.hpp"

void prepare_udp_datagram(struct UDP_data *data, uint8_t datagram[]) {
    // adres sieci
    uint8_t *data_bytes = (uint8_t *)data;
    uint8_t *dest = datagram;
    memcpy(dest, data_bytes, sizeof(data->network.s_addr));
    // dlugosc prefiksu
    data_bytes = (uint8_t *)data + offsetof(struct UDP_data, prefix_len);
    dest += sizeof(data->network.s_addr);
    memcpy(dest, data_bytes, sizeof(data->prefix_len));
    // odleglosc
    uint32_t local_dist = htonl(data->dist);
    dest += sizeof(data->prefix_len);
    memcpy(dest, &local_dist, sizeof(data->dist));
}

void receive_udp_datagram(struct UDP_data *data, uint8_t datagram[]) {
    // adres sieci
    uint8_t *datagram_bytes = datagram;
    uint8_t *dest = (uint8_t *)data;
    memcpy(dest, datagram_bytes, sizeof(data->network.s_addr));
    // dlugosc prefiksu
    datagram_bytes += sizeof(data->network.s_addr);
    dest = (uint8_t *)data + offsetof(struct UDP_data, prefix_len);
    memcpy(dest, datagram_bytes, sizeof(data->prefix_len));
    // odleglosc
    uint32_t local_dist = 0;
    datagram_bytes += sizeof(data->prefix_len);
    dest = (uint8_t *)data + offsetof(struct UDP_data, dist);
    memcpy(&local_dist, datagram_bytes, sizeof(data->dist));
    local_dist = ntohl(local_dist);
    memcpy(dest, &local_dist, sizeof(data->dist));
}