#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define SERVER_TCP_PORT 3000
#define BUFLEN 1000000

int echod(int);
void reaper(int);

int main(int argc, char **argv)
{
	int 	sd, new_sd, client_len, port;
	int opt = 1;
	struct	sockaddr_in server, client;

	switch(argc){
		case 1:
			port = SERVER_TCP_PORT;
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %d [port]\n", argv[0]);
			exit(1);
	}

	/* Create a stream socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
																								&opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
	{
			perror("setsockopt");
			exit(EXIT_FAILURE);
	}
	/* Bind an address to the socket */
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests */
	listen(sd, 5);

	(void) signal(SIGCHLD, reaper);
	client_len = sizeof(client);
	while (1) {
		new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
		if(new_sd < 0){
			fprintf(stderr, "Can't accept client \n");
			exit(1);
		}
		echod(new_sd);
	}

	while(0) {
		client_len = sizeof(client);
		new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
		if(new_sd < 0){
			fprintf(stderr, "Can't accept client \n");
			exit(1);
		}
		echod(new_sd);
		// switch (fork()) {
		// 	/* child */
		// 	case 0:
		// 		(void) close(sd);
		// 		exit(echod(new_sd));
		// 	/* parent */
		// 	default:
		// 		(void) close(new_sd);
		// 		break;
		// 	case -1:
		// 		fprintf(stderr, "fork: error\n");
		// }
	}
}

/* echod program */
int echod(int sd)
{
	long long int 	bytes_to_read, quit = 1, n;
	DIR		*d;
	FILE 	*fp;

	struct dirent *dir;
	struct stat fstat;
	struct PDU {
		char type;
		long long int length;
		char data[BUFLEN+5];
	} rpdu, spdu;

	while (quit) {
		read(sd, (char *)&rpdu, sizeof(rpdu)); // data from client
		printf("I read\n");
		switch(rpdu.type) {
			/* Download request */
			case 'D':
				fp = fopen(rpdu.data, "r");
				if (fp == NULL) {
					spdu.type = 'E'; // data unit type as error
					sprintf(spdu.data, "Error: File \"%s\" not found.\n", rpdu.data);
					spdu.length = strlen(spdu.data);
					write(sd, (char *)&spdu, sizeof(spdu));
					fprintf(stderr, "Error: File \"%s\" not found.\n", rpdu.data);
				} else {
					spdu.type = 'F';
					stat(rpdu.data, &fstat); // get file size
					bytes_to_read = fstat.st_size;
					printf("%lld\n", bytes_to_read);
					uint32_t un = htonl(bytes_to_read);
					send(sd, &un, sizeof(uint32_t), 0);
					while(bytes_to_read > 0) {
						spdu.length = fread(spdu.data, sizeof(char), BUFLEN, fp);
						bytes_to_read -= spdu.length;
						spdu.type = 'F';
						write(sd, (char *)&spdu, sizeof(spdu));
					}
					printf("Transfer of \"%s\" sucessful.\n", rpdu.data);
				}
				break;

			/* Upload request */
			case 'U':
				spdu.type = 'R';
				spdu.length = 0;
				write(sd, (char *)&spdu, sizeof(spdu)); // tell client ready
				fp = fopen(rpdu.data, "w"); // create file to write uploaded data
				printf("Starting transfer of \"%s\".\n", rpdu.data);
				do {
					read(sd, (char *)&rpdu, sizeof(rpdu));
					fwrite(rpdu.data, sizeof(char), rpdu.length, fp); // write data to file
				} while (rpdu.length == BUFLEN);
				fclose(fp);
				printf("Transfer sucessful.\n");
				break;

			/* Change directory */
			case 'P':
				if(opendir(rpdu.data)) { // check if directory is valid
					chdir(rpdu.data);
					printf("Directory changed to \"%s\"\n", rpdu.data);
					spdu.type = 'R';
				} else { // invalid directory
					spdu.type = 'E';
					strcpy(spdu.data, "Error: Invalid directory.\n");
					spdu.length = strlen(spdu.data);
				}
				write(sd, (char *)&spdu, sizeof(spdu)); // send reply to client
				break;

			/* List files */
			case 'L':
				d = opendir("./"); // open current directory
				spdu.data[0] = '\0';
				while ((dir = readdir(d)) != NULL) { // read contents
					strcat(spdu.data, dir->d_name);
					strcat(spdu.data, "\n");
					// printf("%s\n", dir->d_name);
				}
				closedir(d); // close directory
				spdu.length = strlen(spdu.data);
				// char str[100] = "text";
				// if( write(sd, &str, sizeof(str)) < 0) {
					// perror("write");
				// }
				// spdu.length = 5;
				printf("%d\n", spdu.length);
				// send list to client
				if (write(sd, (char *)&spdu, sizeof(spdu)) < 0) {
					perror("write list");
					exit(1);
				}
				// printf("%d\n", spdu.length);

				printf("List files.\n");
				break;

			/* Quit */
			case 'Q':
				quit = 0;
				break;
		}
	}
	close(sd); // close TCP connection
	return(0);
}

/* reaper */
void reaper(int sig)
{
	int	status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}
