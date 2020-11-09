#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#define SIZE 1024
#define HOST 8000
#define true 1

char src[] = "127.0.0.1";

void exit_function(char *buffer) {
  if (strcmp(buffer, "exit") == 0)
    exit(EXIT_SUCCESS);
}

int receive_int(int *num, int fd)
{
    int32_t ret;
    char *data = (char*)&ret;
    int left = sizeof(ret);
    int rc;
    do {
        rc = read(fd, data, left);
        if (rc <= 0) { /* instead of ret */
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                // use select() or epoll() to wait for the socket to be readable again
            }
            else if (errno != EINTR) {
                return -1;
            }
        }
        else {
            data += rc;
            left -= rc;
        }
    }
    while (left > 0);
    *num = ntohl(ret);
    return 0;
}

void send_file(int socket_fd, char *file_name) {
  FILE *fp = fopen(file_name, "rb");
  int n;
  ssize_t total = 0;
  char line[SIZE] = {0};
  if (fp == NULL) {
    printf("cannot open file\n");
    exit(EXIT_FAILURE);
  }
  while ((n = fread(line, sizeof(char), SIZE, fp)) > 0) {
    total += n;
    if (n != SIZE && ferror(fp)) {
      perror("ferror");
      exit(EXIT_FAILURE);
    }
    if (send(socket_fd, line, n, 0) < 0) {
      perror("send");
      exit(EXIT_FAILURE);
    }
    memset(line, 0, SIZE);
  }
  printf("num bytes = %ld\n", total);
}

int main(int argc, char const *argv[]) {
  int socket_fd;
  char *file_name;
  char buffer[SIZE];
  FILE *fp;
  struct sockaddr_in address;
  // printf("%d %s\n", argc, argv[1]);
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket");
      exit(EXIT_FAILURE);
  }
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(HOST);
  if (inet_pton(AF_INET, src, &address.sin_addr) < 0) {
    perror("inet_pton");
    exit(EXIT_FAILURE);
  }
  if (connect(socket_fd, (const struct sockaddr *) &address,
              sizeof(address)) < 0) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  if (argc > 1) {
    if (strcmp(argv[0], "get") == 0) {

    } else {
        // error
        exit(EXIT_FAILURE);
      }
  }

  int num = -1;
  read(socket_fd, &num, sizeof(num));
  printf("%d\n", ntohl(num));

  //send number of files
  // for (size_t i = 1; i < argc; i++) {
  //
    file_name = basename("file.txt");
  //   printf("file %s\n", file_name);
    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, file_name, strlen(file_name));
    // send file name
    if (send(socket_fd,buffer, SIZE, 0) < 0) {
      perror("send");
      exit(EXIT_FAILURE);
    }
  //   send_file(socket_fd, file_name);
  //
  // }

  return 0;
}
// while (true) {
//   //receive command
//   //for each file
//   //if present
//   //send progress and file
//   //else
//   //ERROR
// }
