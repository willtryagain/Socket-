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
} rpdu, tpdu;


void write_file(int in_fd, int out_fd) {
  int fd;
  char *end_ptr;
  char line[SIZE];
  //assuming 20 digits
  if ((fd = read(in_fd, &line, 20)) < 0) {
    perror("read");
    return;
  }
  ll end = strtoll(line, &end_ptr, 10);
  printf("%lld\n", end);
  end = 8;
  ll len = min(end, SIZE-1);

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
  char *host = "localhost";
  int port = SERVER_TCP_PORT;
  int in_fd, out_fd, sd;
  FILE *fp;
  struct sockaddr_in server; // internet endpoint address
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&server, '0', sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
    perror("inet_pton");
    exit(EXIT_FAILURE);
  }

  if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  //file name
  tpdu.length = read(0, tpdu.data, SIZE-1); // get user message
  tpdu.data[tpdu.length-1] = '\0';
  write(sd, (char *)&tpdu, sizeof(tpdu));
  if ((out_fd = open("file.txt", O_RDWR|O_CREAT)) < 0) {
    perror("open");
    exit(1);
  }
  write_file(sd, out_fd);


  // fclose(fp);
  close(sd);
  return 0;
}
