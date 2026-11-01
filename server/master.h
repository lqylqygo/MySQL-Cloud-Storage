#ifndef MY__MASTER_H__
#define MY__MASTER_H__
#include <my_header.h>

void send_fd(int socket, int fd_to_send);
int recv_fd(int socket);
void masterLoop(int listenfd, int *workers_fd, int worker_num);
#endif