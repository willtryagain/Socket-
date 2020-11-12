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
#define SIZE 10000
#define SERVER_TCP_PORT 5000

typedef long long ll;


struct Data {
  int length;
  char data[SIZE + 1];
} rpdu, spdu;

void  write_file(int out_fd, int in_fd) {
  int fd;
  ll end, len;
  char line[SIZE];
  struct stat fstat;

  stat(rpdu.data, &fstat);
  end = fstat.st_size;
  sprintf(line, "%lld", end);

  if ((fd = write(out_fd, &line, 20)) < 0) {
    perror("write");
    return;
  }
  printf("%d\n", end);

  len = min(end, SIZE - 1);
  // spdu.length = fread(spdu.data, sizeof(char), SIZE, fp);
  // end -= spdu.length;

  for (ll i = 1; i*len <= end; i++) {
    if ((fd = read(in_fd, &line, len)) < 0) {
			perror("read");
			return;
		}

    if ((fd = write(out_fd, &line, len)) < 0) {
			perror("write");
			return;
		}
  }

  if (end%len > 0) {
    if ((fd = read(in_fd, &line, end%len)) < 0) {
			perror("read");
			return;
		}

    if ((fd = write(out_fd, &line, end%len)) < 0) {
			perror("write");
			return;
    }
  }
}


int main(int argc, char const *argv[]) {
  int in_fd, sd, new_sd;
  int opt = 1;
  int client_len;
  int fp;
  int port = SERVER_TCP_PORT;
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

  if (read(new_sd, (char *)&rpdu, sizeof(rpdu)) < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  if ((in_fd = open(rpdu.data, O_RDONLY)) < 0) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  write_file(new_sd, in_fd);
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
