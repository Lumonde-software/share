#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void die(char *s) {
    perror(s);
    exit(1);
}

int main(int argc, char **argv) {
    int s, ss;
    if (argc == 2) {
        int port = atoi(argv[1]);

        ss = socket(PF_INET, SOCK_STREAM, 0);
        if (ss == -1) die("socket");

        struct sockaddr_in addr;           /* 最終的にbindに渡すアドレス情報 */
        addr.sin_family = AF_INET;         /* このアドレスはIPv4アドレスです */
        addr.sin_port = htons(port);       /* このポート番号で待ち受けしたいです */
        addr.sin_addr.s_addr = INADDR_ANY; /* どのアドレスでも待ち受けしたいです */
        int ret = bind(ss, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1) die("bind");
        socklen_t addr_len = sizeof(addr);

        listen(ss, 10);

        struct sockaddr_in client_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        s = accept(ss, (struct sockaddr *)&client_addr, &len);
        // die("exit");
        if (s == -1) die("accept");

        FILE *rec = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
        if (rec == NULL) die("rec");
        FILE *play = popen("play -t raw -b 16 -c 1 -e s -r 44100 -", "w");
        if (play == NULL) die("play");

        unsigned char recv_data[1000], send_data[1000];

        while (1) {
            int m = recv(s, &recv_data, 1000, 0);
            if (m == -1) die("recv");
            if (m == 0) break;

            if (fwrite(&recv_data, 1, m, play) == -1) die("write");

            int n = fread(&send_data, 1, 1000, rec);
            if (n == -1) die("fread");
            if (n == 0) break;

            if (send(s, &send_data, n, 0) == -1) die("send");
        }
    } else if (argc == 3) {
        char *IP = argv[1];
        int port = atoi(argv[2]);

        s = socket(PF_INET, SOCK_STREAM, 0);
        if (s == -1) die("socket");

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;             /* IPv4のアドレスです */
        int t = inet_aton(IP, &addr.sin_addr); /* IPアドレスはこれです */
        if (t == 0) die("inet_aton");
        addr.sin_port = htons(port);                                  /* ポート番号はこれです */
        int ret = connect(s, (struct sockaddr *)&addr, sizeof(addr)); /* connect */
        if (ret == -1) die("connect");

        FILE *rec = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
        if (rec == NULL) die("rec");
        FILE *play = popen("play -t raw -b 16 -c 1 -e s -r 44100 -", "w");
        if (play == NULL) die("play");

        unsigned char recv_data, send_data;

        while (1) {
            int n = fread(&send_data, 1, 1000, rec);
            if (n == -1) die("fread");
            if (n == 0) break;

            if (send(s, &send_data, n, 0) == -1) die("send");

            int m = recv(s, &recv_data, 1000, 0);
            if (m == -1) die("recv");
            if (m == 0) break;

            if (fwrite(&recv_data, 1, m, play) == -1) die("write");
        }
    } else {
        die("./tel port or ./tel addr port");
    }

    close(s);
}