// 2023011844 Kim HyeonJin
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define GREP_REQ     1
#define GREP_RES     2
#define GREP_END     3
#define GREP_END_ACK 4

typedef struct {
    int cmd;
    char options[100];
} REQ_PACKET;

typedef struct {
    int cmd;
    int result;
    char matched_lines[2048];
} RES_PACKET;

int main(int argc, char *argv[]) {
    if (argc != 3) { fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]); exit(1); }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    char input[200];
    while (1) {
        printf("Type [option] [keyword] [filename]: ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';

        REQ_PACKET req;
        memset(&req, 0, sizeof(req));

        if (strcmp(input, "quit") == 0) {
            req.cmd = htonl(GREP_END);
            send(sock, &req, sizeof(req), 0);
            printf("[Tx] GREP_END(%d)\n", GREP_END);

            RES_PACKET res;
            recv(sock, &res, sizeof(res), 0);
            res.cmd = ntohl(res.cmd);
            printf("[Rx] GREP_END_ACK(%d)\n", res.cmd);
            printf("Exit rGrep Client\n");
            break;
        }

        req.cmd = htonl(GREP_REQ);
        strncpy(req.options, input, sizeof(req.options) - 1);
        send(sock, &req, sizeof(req), 0);
        printf("[Tx] GREP_REQ(%d) options: %s\n", GREP_REQ, input);

        RES_PACKET res;
        memset(&res, 0, sizeof(res));
        recv(sock, &res, sizeof(res), 0);
        res.cmd    = ntohl(res.cmd);
        res.result = ntohl(res.result);

        printf("-----------------------------------\n");
        printf("[Rx] GREP_RES(%d), result: %d\n", res.cmd, res.result);
        printf("-----------------------------------\n");

        if (res.result == -1)
            printf("File not found!\n");
        else if (res.result == -2)
            printf("Invalid option\n");
        else if (res.result > 0)
            printf("%s", res.matched_lines);

        printf("-----------------------------------\n");
    }

    close(sock);
    return 0;
}
