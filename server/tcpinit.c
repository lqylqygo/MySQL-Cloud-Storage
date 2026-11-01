#include"tcpinit.h"
int tcp_init(const char *ip, int port)
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(server_fd, -1, "socket failed");  int bind_ret=bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    ERROR_CHECK(bind_ret, -1, "bind failed");
    int listen_ret=listen(server_fd, 10);
    ERROR_CHECK(listen_ret, -1, "listen failed");
    return server_fd;
}