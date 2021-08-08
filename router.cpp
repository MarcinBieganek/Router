// Marcin Bieganek
#include "router.hpp"

extern int Inet_pton(int af, const char *addr, void *buf);
extern const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size);
extern int Socket(int domain, int type, int protocol);
extern int Setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
extern int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
extern ssize_t Sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
extern int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
extern ssize_t Recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
extern int Close(int fd);
extern int Gettimeofday(struct timeval *tv, struct timezone *tz);

extern void update_time_left(struct timeval *start, struct timeval *end, struct timeval *tv);

extern struct in_addr network_ip_addr(struct in_addr ip_addr, uint8_t prefix_len);
extern struct in_addr broadcast_ip_addr(struct in_addr ip_addr, uint8_t prefix_len);

extern void prepare_udp_datagram(struct UDP_data *data, uint8_t datagram[]);
extern void receive_udp_datagram(struct UDP_data *data, uint8_t datagram[]);

extern void create_new_distv_elem(struct vector_elem *new_elem, uint32_t dist, uint8_t prefix_len, 
                            struct in_addr network, struct in_addr via_ip_addr);
extern uint32_t find_current_dist(vector<struct vector_elem> dist_vect, struct in_addr ip_addr);
extern int find_index_for_ip_addr(vector<struct vector_elem> dist_vect, struct in_addr ip_addr);
extern bool is_packet_from_me(vector<struct vector_elem> dist_vect, struct in_addr sender_ip);
extern void set_unreachable(vector<struct vector_elem> *dist_vect, int direct_index, vector<struct direct_elem> *direct_vect);
extern void set_reachable(vector<struct vector_elem> *dist_vect, int direct_index, vector<struct direct_elem> *direct_vect);
extern bool network_in_direct(vector<struct direct_elem> direct_vect, struct in_addr network);

uint32_t add_without_overflow(uint32_t a, uint32_t b) {
    if (a == INF || b == INF)
        return INF;
    return (a+b);
}

void read_data(int n, vector<struct vector_elem> *dist_vect, vector<struct direct_elem> *direct_vect) {
    for (int i = 0; i < n; i++) {
        vector_elem new_distv_elem;
        direct_elem new_direct_elem;
        char ip_addr_str[20];
        uint32_t prefix_len;
        uint32_t distance;

        scanf("\n%[^/]/%u distance %u", ip_addr_str, &prefix_len, &distance);
        Inet_pton(AF_INET, ip_addr_str, &new_distv_elem.ip_addr);

        new_distv_elem.prefix_len = (uint8_t)prefix_len;
        new_distv_elem.dist = distance;
        new_distv_elem.indirectly = false;
        new_distv_elem.network = network_ip_addr(new_distv_elem.ip_addr, new_distv_elem.prefix_len);
        new_distv_elem.broadcast = broadcast_ip_addr(new_distv_elem.ip_addr, new_distv_elem.prefix_len);
        new_distv_elem.rounds_since_last_info = 0;
        new_distv_elem.rounds_with_dist_eq_inf = 0;
        new_distv_elem.send_info_about = true;

        new_direct_elem.ip_addr = new_distv_elem.ip_addr;
        new_direct_elem.network = new_distv_elem.network;
        new_direct_elem.broadcast = new_distv_elem.broadcast;
        new_direct_elem.prefix_len = (uint8_t)prefix_len;
        new_direct_elem.dist = distance;

        (*dist_vect).push_back(new_distv_elem);
        (*direct_vect).push_back(new_direct_elem);
    }
}

void print_dist_vect(vector<struct vector_elem> dist_vect) {
    printf("\n================== dist table ==================\n");
    for (size_t i = 0; i < dist_vect.size(); i++) {
        char network_str[20];
        Inet_ntop(AF_INET, &(dist_vect[i].network), network_str, sizeof(network_str));
        printf("%2ld. %s/%d ", i, network_str, dist_vect[i].prefix_len);
        if (dist_vect[i].dist == INF) {
            printf("unreachable ");
        } else {
            printf("distance %d ", dist_vect[i].dist);
        }
        if (dist_vect[i].indirectly) {
            char via_ip_addr_str[20];
            Inet_ntop(AF_INET, &(dist_vect[i].via_ip_addr), via_ip_addr_str, sizeof(via_ip_addr_str));
            printf("via %s\n", via_ip_addr_str);
        } else {
            printf("connected directly\n");
        }
    }
    printf("================================================\n\n");
}

void send_to_neighbors(int sockfd, vector<struct vector_elem> *dist_vect, vector<struct direct_elem> *direct_vect) {
    // do kazdego sasiada wysylamy wszystkie elementy wektora odleglosci
    for (size_t i = 0; i < (*direct_vect).size(); i++) {
        for (size_t j = 0; j < (*dist_vect).size(); j++) {
            if ((*dist_vect)[j].send_info_about) {
                // adresowanie
                struct sockaddr_in recipent;
                bzero(&recipent, sizeof(recipent));
                recipent.sin_family      = AF_INET;
                recipent.sin_port        = htons(PORT);
                recipent.sin_addr.s_addr = (*direct_vect)[i].broadcast.s_addr;
                // przygotowanie danych
                struct UDP_data udp_data;
                udp_data.network = (*dist_vect)[j].network;
                udp_data.prefix_len = (*dist_vect)[j].prefix_len;
                udp_data.dist = (*dist_vect)[j].dist;
                // formatowanie danych
                uint8_t datagram[9];
                prepare_udp_datagram(&udp_data, datagram);
                ssize_t datagram_len = sizeof(datagram);
                // wysyłanie
                int res = sendto(sockfd, &datagram, datagram_len, 0, (struct sockaddr*)&recipent, sizeof(recipent));
                if (res < 0) {
                    // niepowodzenie w wysłaniu do sasiada
                    set_unreachable(dist_vect, i, direct_vect);
                } else {
                    // udalo sie, wiec sasiad musi byc osiagalny
                    set_reachable(dist_vect, i, direct_vect);
                }  
            }
        }
    }
}

int receive_msg(int sockfd, vector<struct vector_elem> *dist_vect) {
    struct sockaddr_in sender;
    socklen_t          sender_len = sizeof(sender);
    u_int8_t           buffer[IP_MAXPACKET];
    Recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);

    if (is_packet_from_me((*dist_vect), sender.sin_addr))
        return 0;

    // otrzymalismy wiadomosc od nadawcy
    // zerujemy licznik rund od ostatniej wiadomosci
    for (size_t i = 0; i < (*dist_vect).size(); i++) {
        if ((*dist_vect)[i].indirectly) {
            if ((*dist_vect)[i].via_ip_addr.s_addr == sender.sin_addr.s_addr)
                (*dist_vect)[i].rounds_since_last_info = 0;
        }
    }

    struct UDP_data datagram;
    receive_udp_datagram(&datagram, buffer);

    // odczytanie odleglosci od nadawcy
    uint32_t dist_to_sender = find_current_dist((*dist_vect), sender.sin_addr);

    // analiza pod kątem zmiany wektora
    int index = find_index_for_ip_addr((*dist_vect), datagram.network);

    if (index == -1) { // przyszla informacja o nowej sieci
        uint32_t new_dist = add_without_overflow(datagram.dist, dist_to_sender);
        // dodawanie nowego wpisu
        vector_elem new_distv_elem;
        create_new_distv_elem(&new_distv_elem, new_dist, datagram.prefix_len, datagram.network, sender.sin_addr);

        (*dist_vect).push_back(new_distv_elem);
    } else {           // przyszla informacja o zapisanej juz sieci
        uint32_t old_dist = (*dist_vect)[index].dist;
        uint32_t new_dist = add_without_overflow(datagram.dist, dist_to_sender);
        // aktualizujemy wpis?
        // gdy lepsza odleglosc
        if (new_dist < old_dist) {
            (*dist_vect)[index].indirectly = true;
            (*dist_vect)[index].dist = new_dist;
            (*dist_vect)[index].via_ip_addr = sender.sin_addr;
        } else if ((*dist_vect)[index].indirectly) {
            // lub gdy informacja jest od obecnego posrednika (via_ip_addr)
            if ((*dist_vect)[index].via_ip_addr.s_addr == sender.sin_addr.s_addr) {
                (*dist_vect)[index].dist = new_dist;
            }
        }
    }

    return 0;
}

void on_round_end(vector<struct vector_elem> *dist_vect, vector<struct direct_elem> *direct_vect) {
    for (size_t i = 0; i < (*dist_vect).size(); i++) {
        if ((*dist_vect)[i].indirectly) {
            // zwiekszamy licznik rund we wpisach posrednich
            (*dist_vect)[i].rounds_since_last_info += 1;
            // sprawdzamy, czy nie przekroczyl on granicznej wartosci (utrata polaczenia)
            if ((*dist_vect)[i].rounds_since_last_info > MAX_ROUNDS_WITHOUT_INFO)
                (*dist_vect)[i].dist = INF;
        }
        // sprawdzamy, czy dystans nie przekroczyl maksymalnej wartosci (uznajemy go za inf)
        if ((*dist_vect)[i].dist > MAX_DIST)
            (*dist_vect)[i].dist = INF;
        // aktualizujemy informacje o liczbie rund z odlegloscia = inf
        if ((*dist_vect)[i].dist == INF)
            (*dist_vect)[i].rounds_with_dist_eq_inf += 1;
        else
            (*dist_vect)[i].rounds_with_dist_eq_inf = 0;
        // sprawdzamy, czy nie przekroczyla ona granicznej wartosci
        if ((*dist_vect)[i].rounds_with_dist_eq_inf > MAX_ROUNDS_WITH_INF) {
            if ((*dist_vect)[i].indirectly) 
                // (usuniecie wpisu posredniego)
                (*dist_vect).erase((*dist_vect).begin() + i);
            else
                // (koniec wysylania informacji o wpisie bezposrednim)
                (*dist_vect)[i].send_info_about = false;
        }
        // sprawdzamy, czy siec ze sciezka posrednia, ktora ma dist = INF
        // nie jest w sieciach bezposrednich
        if ((*dist_vect)[i].dist == INF && (*dist_vect)[i].indirectly) {
            if (network_in_direct((*direct_vect), (*dist_vect)[i].network)) {
                (*dist_vect)[i].indirectly = false;
            }
        }

    }
}

int main() {
    int n;
    scanf("%d", &n);

    vector<struct vector_elem> dist_vect;
    dist_vect.reserve(n);
    vector<struct direct_elem> direct_vect;
    direct_vect.reserve(n);

    read_data(n, &dist_vect, &direct_vect);
    
    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    
    int broadcastPermission = 1;
    Setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission));

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(sockfd, (struct sockaddr*)&server_address, sizeof(server_address));

    for (;;) {
        // wypisujemy
        print_dist_vect(dist_vect);
        // wysyłamy
        send_to_neighbors(sockfd, &dist_vect, &direct_vect);
        //odbieramy
        fd_set descriptors;
        FD_ZERO(&descriptors);
        FD_SET(sockfd, &descriptors);

        struct timeval round_time; round_time.tv_sec = ROUND_TIME; round_time.tv_usec = 0;

        while (true) {
            // przygotuj czas
            struct timeval start;
            Gettimeofday(&start, NULL);
            struct timeval tv; tv.tv_sec = round_time.tv_sec; tv.tv_usec = round_time.tv_usec;

            int ready = Select(sockfd+1, &descriptors, NULL, NULL, &tv);
            // zaakutalizuj czas
            struct timeval end;
            Gettimeofday(&end, NULL);
            update_time_left(&start, &end, &round_time);
            if (ready <= 0) { 
                break; // timeout for current round
            } else {
                receive_msg(sockfd, &dist_vect);
            }
        }
        // na koniec rundy:
        on_round_end(&dist_vect, &direct_vect);
    }

    Close(sockfd);

    return EXIT_SUCCESS;
}