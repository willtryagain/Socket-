#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define min(a, b) a < b? a:b
#define SIZE 100000
#define SERVER_TCP_PORT 5000
#define true 1
typedef long long ll;


struct Data {
  int length;
  char data[SIZE + 1];
} rpdu, spdu;

char line[SIZE];

int read_file(int out_fd, ll size, int in_fd) {
  int fd;
  off_t chunk = 0;

  while (chunk < size) {
    size_t read_now = read(in_fd, &line, SIZE);
    if (read_now < 0) {
      perror("read");
      exit(1);
    }
    // printf("%s\n", line);
    if ((fd = write(out_fd, &line, read_now)) < 0) {
			perror("write");
			exit(1);
    }
    chunk += read_now;
  }
  // close(out_fd);
  return 1;
}

int main(int argc, char const *argv[]) {
  int in_fd, sd, new_sd;
  int opt = 1;
  int client_len;
  ll end;
  int fp;
  int port = SERVER_TCP_PORT;
  struct stat fstat;
  struct sockaddr_in server;
  struct sockaddr_in client;

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                 &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if (listen(sd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  client_len = sizeof(client);
  if ((new_sd = accept(sd, (struct sockaddr *)&client, &client_len)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }


  while (true) {
    if (read(new_sd, (char *)&rpdu, sizeof(rpdu)) < 0) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    if (!strcmp(rpdu.data, "")) {
      printf("%s,\n", rpdu.data); 
      continue;
    }
    if ((in_fd = open(rpdu.data, O_RDWR)) < 0) {
      perror("open");
      exit(EXIT_FAILURE);
    }

    if (stat(rpdu.data, &fstat)) {
      perror("stat");
      exit(1);
    }
    end = fstat.st_size;
    sprintf(line, "%lld", end);

    if (write(new_sd, &line, 20) < 0) {
      perror("write");
      return;
    }
    printf("readin\n");
    read_file(new_sd, end, in_fd);
    printf("readout\n");
  }


  // while (end > 0) {
  //   spdu.length = fread(spdu.data, sizeof(char), SIZE, fp);
  //   end -= spdu.length;
  //   if (write(sd, (char *)&spdu, sizeof(spdu)) < 0) {
  //     perror("write");
  //     exit(EXIT_FAILURE);
  //   }
  // }

  close(sd);
  return 0;
}
