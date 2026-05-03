//2023011844 Kim HyeonJin 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT     9000
#define BUF_SIZE 1024
#define SENDER   1
#define RECEIVER 2
#define MSG_SEND  1
#define TERMINATE 2

typedef struct {
    int cmd;
    char buf[BUF_SIZE + 1];
} PACKET;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    // 역할 선택
    int role;
    printf("Select role (1=Sender, 2=Receiver): ");
    scanf("%d", &role);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port        = htons(PORT);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    send(sock, &role, sizeof(int), 0);
    printf("[Client] Connected as %s (fd=%d)\n", role == SENDER ? "SENDER" : "RECEIVER", sock);

    fd_set reads, temps;
    struct timeval timeout;
    FD_ZERO(&reads);

    if (role == SENDER) {
        FILE *fp = fopen("rfc1180.txt", "r");
        if (!fp) { perror("fopen"); return 1; }

        int file_fd = fileno(fp);
        FD_SET(file_fd, &reads);
        int fd_max = file_fd;

        while (1) {
            temps = reads;
            timeout.tv_sec  = 3;
            timeout.tv_usec = 0;

            int result = select(fd_max + 1, &temps, NULL, NULL, &timeout);
            if (result == 0) {
                printf("[Sender] select() timeout\n");
                continue;
            }

            if (FD_ISSET(file_fd, &temps)) {
                PACKET pkt;
                memset(&pkt, 0, sizeof(pkt));
                pkt.cmd = MSG_SEND;

                int n = fread(pkt.buf, 1, BUF_SIZE, fp);
                if (n <= 0) {
                    // 파일 전송 완료
                    FD_CLR(file_fd, &reads);
                    fclose(fp);
                    printf("[Sender] File transfer complete. Closing.\n");
                    close(sock);
                    break;
                }

                printf("[Sender] Read %d bytes, sending...\n", n);
                send(sock, &pkt, sizeof(PACKET), 0);
                sleep(1);
            }
        }

    } else {
        // RECEIVER
        FD_SET(sock, &reads);
        int fd_max = sock;

        while (1) {
            temps = reads;
            timeout.tv_sec  = 3;
            timeout.tv_usec = 0;

            int result = select(fd_max + 1, &temps, NULL, NULL, &timeout);
            if (result == 0) {
                printf("[Receiver] select() timeout\n");
                continue;
            }

            if (FD_ISSET(sock, &temps)) {
                PACKET pkt;
                int n = recv(sock, &pkt, sizeof(PACKET), 0);
                if (n <= 0) {
                    printf("[Receiver] Connection closed.\n");
                    FD_CLR(sock, &reads);
                    break;
                }

                if (pkt.cmd == MSG_SEND) {
                    printf("%s", pkt.buf);
                    fflush(stdout);
                } else if (pkt.cmd == TERMINATE) {
                    printf("\n[Receiver] TERMINATE received. Closing.\n");
                    FD_CLR(sock, &reads);
                    close(sock);
                    break;
                }
            }
        }
    }

    return 0;
}
