#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>
#include <vector>
#include <limits.h>

using namespace std;

int Inet_pton(int af, const char *addr, void *buf);
const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size);
int Socket(int domain, int type, int protocol);
int Setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t Sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
int Close(int fd);
int Gettimeofday(struct timeval *tv, struct timezone *tz);

#define ROUND_TIME 15 // in sec
#define INF UINT_MAX
#define PORT 54321
#define MAX_ROUNDS_WITHOUT_INFO 4
#define MAX_DIST 20
#define MAX_ROUNDS_WITH_INF 4

struct vector_elem {
    struct in_addr ip_addr;
    struct in_addr network;
    struct in_addr broadcast;
    uint8_t prefix_len;
    uint32_t dist;
    bool indirectly;
    struct in_addr via_ip_addr; // used for indirect
    int rounds_since_last_info; // used for indirect
    int rounds_with_dist_eq_inf;
    bool send_info_about;       // used for direct, when rounds_with_dist_eq_inf > MAX_ROUNDS_WITH_INF
};

struct direct_elem {
    struct in_addr ip_addr;
    struct in_addr network;
    struct in_addr broadcast;
    uint8_t prefix_len;
    uint32_t dist;
};

struct UDP_data {
    struct in_addr network;
    uint8_t prefix_len;
    uint32_t dist;
};

void update_time_left(struct timeval *start, struct timeval *end, struct timeval *tv);

struct in_addr network_ip_addr(struct in_addr ip_addr, uint8_t prefix_len);
struct in_addr broadcast_ip_addr(struct in_addr ip_addr, uint8_t prefix_len);
bool ip_addr_in_network(struct in_addr ip_addr, struct in_addr network_ip_addr, uint8_t prefix_len);

void prepare_udp_datagram(struct UDP_data *data, uint8_t datagram[]);
void receive_udp_datagram(struct UDP_data *data, uint8_t datagram[]);

void create_new_distv_elem(struct vector_elem *new_elem, uint32_t dist, uint8_t prefix_len, 
                            struct in_addr network, struct in_addr via_ip_addr);
uint32_t find_current_dist(vector<struct vector_elem> dist_vect, struct in_addr ip_addr);
int find_index_for_ip_addr(vector<struct vector_elem> dist_vect, struct in_addr ip_addr);
bool is_packet_from_me(vector<struct vector_elem> dist_vect, struct in_addr sender_ip);
void set_unreachable(vector<struct vector_elem> *dist_vect, int direct_index, vector<struct direct_elem> *direct_vect);
void set_reachable(vector<struct vector_elem> *dist_vect, int direct_index, vector<struct direct_elem> *direct_vect);
bool network_in_direct(vector<struct direct_elem> direct_vect, struct in_addr network);

void read_data(int n, vector<struct vector_elem> *dist_vect, vector<struct direct_elem> *direct_vect);
void print_dist_vect(vector<struct vector_elem> dist_vect);
void send_to_neighbors(int sockfd, vector<struct vector_elem> *dist_vect);
int receive_msg(int sockfd, vector<struct vector_elem> *dist_vect);
void on_round_end(vector<struct vector_elem> *dist_vect, vector<struct direct_elem> *direct_vect);