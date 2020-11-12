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
#define SIZE 100000
#define SERVER_TCP_PORT 5000
#define true 1
typedef long long ll;
#define TIME 30


struct Data {
  int length;
  char data[SIZE + 1];
} rpdu, tpdu;

char command[100], args[11][50], time_left[SIZE];

void str_init_() {
	strcpy(time_left, "[");
	for (ll i = 1; i <= TIME; ++i)
		time_left[i] = '*';
	time_left[TIME+1] = ']';
	time_left[TIME+2] = '\0';
}

void change(float p) {
	str_init_();
	ll len = (TIME*p)/100;
	for (ll i = 1; i <= len; ++i)
		time_left[i] = '#';
}

void get_args(char *command, int *count) {
	int cnt = 0;
	char *word = strtok(command, " ");
	while (word != NULL) {
		// printf("%s\n", word);
		strcpy(args[cnt++], word);
		word = strtok(NULL, " ");
	}
	int n = strlen(args[cnt-1]);
	args[cnt-1][n-1] = 0;
	*count = cnt;
	// printf("%d\n", *count);
}

int read_file(char *file_name, ll size, int in_fd) {
  int fd, out_fd;
  long *buffer = (long*) malloc(size * sizeof(long));
  off_t chunk = 0;
  float p;
  char line[SIZE];
  if ((out_fd = creat(file_name, 0777)) < 0) {
    perror("creat");
    exit(1);
  }
  //cursor to start
  while (chunk < size) {
    write(1, "\r", strlen("\r"));
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
    // printf("%lf\n", (double)chunk/size);
    //display progress
    p = ((double)chunk/size) * 100;
    sprintf(line, "%.2f%% ", p);
    change(p);
    strcat(line, time_left);
    if (write(1, line, strlen(line)) < 0) {
      perror("console");
      return;
    }
  }
  // close(out_fd);
  return 1;
}

int main(int argc, char const *argv[]) {
  int count = 0;
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
//diff c/1M.txt s/1M.txt
  while (true) {
    printf("Client>");
	  fgets(command, 200, stdin);
	  get_args(command, &count);
    if (strcmp(args[0], "get") == 0) {
			for (int i = 1; i < count; i++) {
      //file name
        tpdu.length = strlen(args[i]);
        strcpy(tpdu.data, args[i]);

        write(sd, (char *)&tpdu, sizeof(tpdu));
        if (read(sd, &line, 20) < 0) {
          perror("read");
          exit(1);
        }
        end = strtoll(line, &end_ptr, 10);
        read_file(tpdu.data, end, sd);

        chmod(tpdu.data, 0777);
        printf("\nTransfer sucessful.\n");
      }
    }
    else if (!strcmp(args[0], "exit"))
			exit(EXIT_SUCCESS);
	  else
			printf("command not recognized\n");
    // printf("iteration\n");
  }
  // fclose(fp);
  close(sd);
  return 0;
}
