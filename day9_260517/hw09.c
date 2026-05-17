// 2023011844 Kim Hyeon JIn
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define BUF_SIZE    150
#define NAME_SIZE   50
#define MSG_SIZE    100

int recv_sock;
struct ip_mreq mreq;

void sigterm_handler(int sig) {
    printf("\nSIGTERM: Multicast Receiver terminate!\n");
    setsockopt(recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    printf("Multicast drop membership and Exit\n");
    close(recv_sock);
    printf("[Child] recv_sock closed\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <GroupIP> <PORT> <Name>\n", argv[0]);
        return 1;
    }

    char *group_ip = argv[1];
    int   port     = atoi(argv[2]);
    char  name[NAME_SIZE];
    strncpy(name, argv[3], NAME_SIZE - 1);
    name[NAME_SIZE - 1] = '\0';

    /* ── recv_sock: UDP Socket, SO_REUSEADDR, bind, IP_ADD_MEMBERSHIP ── */
    recv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_sock < 0) { perror("recv_sock"); exit(1); }

    int opt = 1;
    setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in recv_addr;
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family      = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    recv_addr.sin_port        = htons(port);

    if (bind(recv_sock, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("bind"); exit(1);
    }

    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(group_ip);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

    /* ── send_sock: UDP 소켓, IP_MULTICAST_TTL=64 ── */
    int send_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (send_sock < 0) { perror("send_sock"); exit(1); }

    int ttl = 64;
    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    struct sockaddr_in send_addr;
    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family      = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr(group_ip);
    send_addr.sin_port        = htons(port);

    /* ── fork ── */
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }

    if (pid == 0) {
        /* ── 자식 프로세스: Receiver ── */
        close(send_sock);

        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigterm_handler;
        sigaction(SIGTERM, &sa, NULL);

        char buf[BUF_SIZE];
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);

        while (1) {
            memset(buf, 0, BUF_SIZE);
            int n = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0,
                             (struct sockaddr *)&from, &from_len);
            if (n < 0) break;
            buf[n] = '\0';
            printf("%s\n", buf);
            fflush(stdout);
        }

        /* 부모가 소켓 닫으면 여기 도달 가능, 정상 종료 */
        close(recv_sock);
        exit(0);

    } else {
        /* ── 부모 프로세스: Sender ── */
        char input[MSG_SIZE + 1];
        char buf[BUF_SIZE];

        while (1) {
            memset(input, 0, sizeof(input));
            if (fgets(input, sizeof(input), stdin) == NULL) break;

            /* 개행 제거 */
            input[strcspn(input, "\n")] = '\0';

            /* 종료 조건 */
            if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0) {
                /* 자식에게 SIGTERM 전송 */
                kill(pid, SIGTERM);

                /* 자식도 recvfrom에서 빠져나오도록 종료 메시지 전송 */
                sendto(send_sock, "EXIT", 4, 0,
                       (struct sockaddr *)&send_addr, sizeof(send_addr));

                /* 자식 종료 대기 */
                waitpid(pid, NULL, 0);

                close(send_sock);
                printf("[Parent] send_sock closed\n");
                break;
            }

            /* 시간 정보 획득 */
            time_t now = time(NULL);
            struct tm *t = localtime(&now);

            /* 전송 메시지 조합: "HH:MM:SS [이름] 메시지" */
            memset(buf, 0, BUF_SIZE);
            snprintf(buf, BUF_SIZE, "%02d:%02d:%02d [%s] %s",
                     t->tm_hour, t->tm_min, t->tm_sec,
                     name, input);

            sendto(send_sock, buf, strlen(buf), 0,
                   (struct sockaddr *)&send_addr, sizeof(send_addr));
        }
    }

    return 0;
}
