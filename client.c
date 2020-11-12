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

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

#define min(a, b) a < b? a:b
#define SIZE 10000
#define SERVER_TCP_PORT 5000

typedef long long ll;


struct Data {
  int length;
  char data[SIZE + 1];
} rpdu, tpdu;


int read_file(char *file_name, ll size, int in_fd) {
  int fd, out_fd;
  long *buffer = (long*) malloc(size * sizeof(long));
  off_t chunk = 0;
  char line[SIZE];
  if ((out_fd = open(file_name, O_RDWR|O_CREAT)) < 0) {
    perror("open");
    exit(1);
  }
  while (chunk < size) {
    size_t read_now = read(in_fd, &line, SIZE);
    if (read_now < 0) {
      perror("read");
      exit(1);
    }
    printf("%s\n", line);
    if ((fd = write(out_fd, &line, read_now)) < 0) {
			perror("write");
			exit(1);
    }
    chunk += read_now;
  }
  close(out_fd);
  return 1;
}

int main(int argc, char const *argv[]) {
  char *host = "localhost";
  char *end_ptr;
  char line[SIZE];
  ll end;
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
  if (read(sd, &line, 20) < 0) {
    perror("read");
    exit(1);
  }
  end = strtoll(line, &end_ptr, 10);
  read_file(tpdu.data, end, sd);

  chmod(tpdu.data, S_IRUSR|S_IWUSR);
  // fclose(fp);
  close(sd);
  return 0;
}
