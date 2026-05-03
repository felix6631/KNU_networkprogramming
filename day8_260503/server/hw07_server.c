// 2023011844 Kim HyeonJin
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9000
#define BUF_SIZE 1024
#define SENDER   1
#define RECEIVER 2
#define MSG_SEND 1
#define TERMINATE 2

typedef struct {
    int cmd;
    char buf[BUF_SIZE + 1];
} PACKET;

int main(void) {
    int serv_fd, clnt_fd;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_len = sizeof(clnt_addr);

    fd_set reads, temps;
    struct timeval timeout;
    int fd_max, role_buf;

    int clients[FD_SETSIZE];
    int client_roles[FD_SETSIZE];
    int client_count = 0;

    memset(clients, -1, sizeof(clients));
    memset(client_roles, 0, sizeof(client_roles));

    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(PORT);

    bind(serv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serv_fd, 5);

    FD_ZERO(&reads);
    FD_SET(serv_fd, &reads);
    fd_max = serv_fd;

    printf("[Server] Waiting for 3 clients (1 Sender + 2 Receivers)...\n");

    while (1) {
        temps = reads;
        timeout.tv_sec  = 3;
        timeout.tv_usec = 0;

        int result = select(fd_max + 1, &temps, NULL, NULL, &timeout);
        if (result == 0) {
            printf("[Server] select() timeout\n");
            continue;
        }

        for (int i = 0; i <= fd_max; i++) {
            if (!FD_ISSET(i, &temps)) continue;

            if (i == serv_fd) {
                clnt_fd = accept(serv_fd, (struct sockaddr *)&clnt_addr, &clnt_len);
                // 역할 수신
                recv(clnt_fd, &role_buf, sizeof(int), 0);

                FD_SET(clnt_fd, &reads);
                if (clnt_fd > fd_max) fd_max = clnt_fd;

                clients[clnt_fd]      = clnt_fd;
                client_roles[clnt_fd] = role_buf;
                client_count++;

                printf("[Server] Client fd=%d connected as %s\n",
                       clnt_fd, role_buf == SENDER ? "SENDER" : "RECEIVER");
            } else {
                // 데이터 수신
                PACKET pkt;
                int n = recv(i, &pkt, sizeof(PACKET), 0);

                if (n <= 0) {
                    // 클라이언트 종료 (Sender FIN)
                    printf("[Server] Client fd=%d disconnected\n", i);
                    FD_CLR(i, &reads);
                    clients[i]      = -1;
                    client_roles[i] = 0;
                    client_count--;
                    close(i);

                    // 모든 클라이언트 없으면 서버 종료
                    if (client_count == 0) {
                        printf("[Server] All clients disconnected. Shutting down.\n");
                        close(serv_fd);
                        return 0;
                    }
                    continue;
                }

                if (pkt.cmd == MSG_SEND) {
                    // Receiver 2개에게 포워딩
                    for (int j = 0; j <= fd_max; j++) {
                        if (clients[j] == -1) continue;
                        if (client_roles[j] == RECEIVER) {
                            int sent = send(j, &pkt, sizeof(PACKET), 0);
                            printf("[Server] Forwarded to fd=%d, bytes=%d\n", j, sent);
                        }
                    }
                }
            }
        }

        // Sender가 없어지면 Receiver들에게 TERMINATE 전송
        int has_sender = 0;
        for (int i = 0; i <= fd_max; i++) {
            if (clients[i] != -1 && client_roles[i] == SENDER) {
                has_sender = 1;
                break;
            }
        }

        if (!has_sender && client_count > 0) {
            PACKET term;
            term.cmd = TERMINATE;
            memset(term.buf, 0, sizeof(term.buf));

            for (int i = 0; i <= fd_max; i++) {
                if (clients[i] != -1 && client_roles[i] == RECEIVER) {
                    send(i, &term, sizeof(PACKET), 0);
                    printf("[Server] Sent TERMINATE to fd=%d\n", i);
                }
            }
        }
    }

    return 0;
}
