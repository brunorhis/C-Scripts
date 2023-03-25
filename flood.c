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
    // Extrai os argumentos passados na linha de comando
    char *dst_ip = argv[1]; // endereço IP do destino
    int dst_port = atoi(argv[2]); // porta do destino
    int num_packets = atoi(argv[3]); // número de pacotes a serem enviados (0 = infinito)
    int packet_size = sizeof(struct iphdr) + sizeof(struct tcphdr);
    char packet[MAX_PACKET_SIZE];
    memset(packet, 0, MAX_PACKET_SIZE);

    // Configura o endereço de destino do pacote
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET; // IPv4
    dest_addr.sin_port = htons(dst_port); // porta do destino
    if (inet_pton(AF_INET, dst_ip, &dest_addr.sin_addr) != 1) {
        // Verifica se o endereço IP é válido
        printf("Invalid IP address: %s\n", dst_ip);
        exit(EXIT_FAILURE);
    }

    // Configura o cabeçalho do pacote TCP
    struct tcphdr tcp;
    memset(&tcp, 0, sizeof(tcp));
    tcp.source = htons(12345); // porta de origem do pacote (qualquer porta livre)
    tcp.dest = htons(dst_port); // porta de destino do pacote
    tcp.seq = htonl(rand() % 900000000 + 100000000); // número de sequência aleatório
    tcp.doff = sizeof(tcp) / 4; // tamanho do cabeçalho TCP em palavras de 32 bits
    tcp.syn = 1; // flag SYN (solicitação de conexão)

    // Laço principal de envio dos pacotes
    while (num_packets == 0 || num_packets > 0) {
        // Monta o pacote completo (cabeçalho IP + cabeçalho TCP)
        memcpy(packet, &dest_addr.sin_addr, sizeof(dest_addr.sin_addr));
        memcpy(packet + sizeof(dest_addr.sin_addr), &tcp, sizeof(tcp));

        // Envia o pacote usando o comando hping3
        char command[1024];
        sprintf(command, "sudo hping3 -c 1 -S -p %d -d %d %s", dst_port, packet_size, dst_ip);
        if (system(command) == -1) {
            // Verifica se houve erro ao enviar o pacote
            printf("Failed to send packet: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("Packet sent to %s:%d\n", dst_ip, dst_port);

        // Atualiza o número de pacotes restantes, se for o caso
        if (num_packets > 0) {
            num_packets--;
        }

        // Aguarda um intervalo antes de enviar o próximo pacote
        usleep(200000); // 200 milissegundos
    }

    return 0;
}
