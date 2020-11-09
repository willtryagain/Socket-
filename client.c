#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#define SIZE 1000
#define HOST 8000
#define LISTENPORT 5000
#define true 1

void get_args(char *command, int *count, char args[][SIZE]) {
	int cnt = 0;
	char *word = strtok(command, " ");
	while (word != NULL) {
		printf("%s\n", word);
		strcpy(args[cnt++], word);
		word = strtok(NULL, " ");
	}
	int n = strlen(args[cnt-1]);
	args[cnt-1][n-1] = 0;
	*count = cnt;
}

int send_int(int num, int fd) {
    int32_t conv = htonl(num);
    char *data = (char*)&conv;
    int left = sizeof(conv);
    int rc;
    do {
        rc = write(fd, data, left);
        if (rc < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                // use select() or epoll() to wait for the socket to be writable again
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
    return 0;
}

void write_file(int socket_fd, char *file_name) {
	ssize_t n;
	char buffer[SIZE] = {0};
	FILE *fp = fopen(file_name, "wb");
	if (fp == NULL) {
		printf("fopen\n");
		exit(EXIT_FAILURE);
	}
	while ((n = recv(socket_fd, buffer, SIZE, 0)) > 0) {
		if (fwrite(buffer, sizeof(char), n, fp) != n) {
			perror("fwrite");
			exit(EXIT_FAILURE);
		}
		memset(buffer, 0, SIZE);
	}
}

// void send_file_name(char *file_name) {
// 		if (send(sock, file_name, strlen(file_name), 0) == -1) {
// 			perror("send");
// 			exit(EXIT_FAILURE);
// 		}
// 		//get progress
// }

int main(int32_t argc, char const *argv[]) {
	struct sockaddr_in client_addr, server_addr;
	socklen_t addr_len;
	int socket_fd, connect_fd;
	char file_name[SIZE] = {0};
	char buffer[SIZE];
	struct sockaddr_in serv_addr;
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(HOST);

	if (bind(socket_fd, (const struct sockaddr *) &server_addr,
																	sizeof(server_addr)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(socket_fd, LISTENPORT) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	addr_len = sizeof(client_addr);
	if ((connect_fd = accept(socket_fd, (struct sockaddr *) &client_addr,
																											 &addr_len)) < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}
	close(socket_fd);

	int conv = htonl(argc);
	printf("%d\n", conv);
	write(connect_fd, &conv, sizeof(conv));

	for (size_t i = 1; i < argc; i++) {

	}
	if (recv(connect_fd, file_name, SIZE, 0) < 0) {
		perror("recv");
		exit(EXIT_FAILURE);
	}
	// printf("%s\n", file_name);
	write_file(connect_fd, file_name);
	// if (strcmp(argv[1], "get") == 0) {
	// 	for (size_t i = 2; i < argc; i++) {
	// 		// send_file_name(args[i]);
	// 		printf("%s\n", argv[i]);
	// 	}
	// }
	// else if (strcmp(argv[0], "exit") == 0) {
	// 	exit(EXIT_SUCCESS);
	// } else {
	// 	printf("command not recognized\n");
	// }
	  //see whether exit or get
	  //request
	  //display progress
	  //print receieved
	// }
	return 0;
}
