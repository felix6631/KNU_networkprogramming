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

void do_grep(const char *opt, const char *keyword, const char *filename, RES_PACKET *res) {
    if (strcmp(opt, "-n") != 0 && strcmp(opt, "-v") != 0 && strcmp(opt, "-i") != 0) {
        printf("Invalid option: %s\n", opt);
        res->result = -2;
        return;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("File not found: %s\n", filename);
        res->result = -1;
        return;
    }

    char line[512];
    int lineno = 0, count = 0;
    res->matched_lines[0] = '\0';

    while (fgets(line, sizeof(line), fp)) {
        lineno++;
        line[strcspn(line, "\n")] = '\0';

        int match = 0;
        if (strcmp(opt, "-n") == 0)
            match = strstr(line, keyword) != NULL;
        else if (strcmp(opt, "-v") == 0)
            match = strstr(line, keyword) == NULL;
        else if (strcmp(opt, "-i") == 0)
            match = strcasestr(line, keyword) != NULL;

        if (match) {
            char buf[600];
            snprintf(buf, sizeof(buf), "%3d: %s\n", lineno, line);
            strncat(res->matched_lines, buf, sizeof(res->matched_lines) - strlen(res->matched_lines) - 1);
            count++;
        }
    }
    fclose(fp);
    res->result = count;
}

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "Usage: %s <port>\n", argv[0]); exit(1); }

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(serv_sock, 5);

    int clnt_sock = accept(serv_sock, NULL, NULL);

    while (1) {
        REQ_PACKET req;
        if (recv(clnt_sock, &req, sizeof(req), 0) <= 0) break;
        req.cmd = ntohl(req.cmd);

        RES_PACKET res;
        memset(&res, 0, sizeof(res));

        if (req.cmd == GREP_REQ) {
            char opts_copy[100];
            strncpy(opts_copy, req.options, sizeof(opts_copy));
            char *option   = strtok(opts_copy, " ");
            char *keyword  = strtok(NULL, " ");
            char *filename = strtok(NULL, " ");

            printf("[Rx] GREP_REQ(%d), options: %s\n", req.cmd, req.options);

            do_grep(option, keyword, filename, &res);
            res.cmd = htonl(GREP_RES);
            int net_result = htonl(res.result);

            printf("-----------------------------------\n");
            printf("[Tx] GREP_RES(%d), result: %d\n", GREP_RES, res.result);
            printf("-----------------------------------\n");
            if (res.result > 0)
                printf("%s", res.matched_lines);
            printf("-----------------------------------\n");

            res.result = net_result;
            send(clnt_sock, &res, sizeof(res), 0);

        } else if (req.cmd == GREP_END) {
            printf("[Rx] GREP_END(%d)\n", req.cmd);
            res.cmd = htonl(GREP_END_ACK);
            send(clnt_sock, &res, sizeof(res), 0);
            printf("[Tx] GREP_END_ACK(%d)\n", GREP_END_ACK);
            printf("Exit rGrep Server\n");
            break;
        }
    }

    close(clnt_sock);
    close(serv_sock);
    return 0;
}
