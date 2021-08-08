// Marcin Bieganek
#include "router.hpp"

struct in_addr network_ip_addr(struct in_addr ip_addr, uint8_t prefix_len) {
    struct in_addr network_ip_addr;
    uint32_t ip_addr_host_order = ntohl(ip_addr.s_addr);
    uint32_t mask = (~0U) << (32 - prefix_len);
    uint32_t network_ip_addr_host_order = ip_addr_host_order & mask;
    network_ip_addr.s_addr = htonl(network_ip_addr_host_order);
    return network_ip_addr;
}

struct in_addr broadcast_ip_addr(struct in_addr ip_addr, uint8_t prefix_len) {
    struct in_addr broadcast_ip_addr;
    uint32_t ip_addr_host_order = ntohl(ip_addr.s_addr);
    uint32_t mask = ~((~0U) << (32 - prefix_len));
    uint32_t broadcast_ip_addr_host_order = ip_addr_host_order | mask;
    broadcast_ip_addr.s_addr = htonl(broadcast_ip_addr_host_order);
    return broadcast_ip_addr;
}

bool ip_addr_in_network(struct in_addr ip_addr, struct in_addr network_ip_addr, uint8_t prefix_len) {
    struct in_addr broadcast = broadcast_ip_addr(network_ip_addr, prefix_len);
    uint32_t broadcast_host_order = ntohl(broadcast.s_addr);
    uint32_t network_host_order = ntohl(network_ip_addr.s_addr);
    uint32_t ip_addr_host_order = ntohl(ip_addr.s_addr);
    if (network_host_order <= ip_addr_host_order && ip_addr_host_order <= broadcast_host_order)
        return true;
    return false;
}