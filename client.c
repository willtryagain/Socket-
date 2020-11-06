#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define SIZE 1000
#define PORT 8000
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

// void send_file_name(char *file_name) {
// 		if (send(sock, file_name, strlen(file_name), 0) == -1) {
// 			perror("send");
// 			exit(EXIT_FAILURE);
// 		}
// 		//get progress
// }

int main(int argc, char const *argv[]) {
	int count = 0;
	char command[SIZE];
	char args[11][SIZE];
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket creation");
		exit(EXIT_FAILURE);
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( PORT );

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("Invalid address");
		exit(EXIT_FAILURE);
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect");
		exit(EXIT_FAILURE);
	}



	while (true) {
	  printf("Client> ");
	  fgets(command, SIZE, stdin);
	  get_args(command, &count, args);

		if (strcmp(args[0], "get") == 0) {
			for (size_t i = 1; i < count; i++) {
				// send_file_name(args[i]);
				printf("%s\n", args[i]);
			}
		}
		else if (strcmp(args[0], "exit") == 0) {
			exit(EXIT_SUCCESS);
		} else {
			printf("command not recognized\n");
		}
	  //see whether exit or get
	  //request
	  //display progress
	  //print receieved
	}
	return 0;
}
