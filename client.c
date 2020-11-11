
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

#define SERVER_TCP_PORT 3000
#define	BUFLEN 1000000
#define	SIZE 1000000

#define true 1


char args[11][50];

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

int main(int argc, char const *argv[])
{
	int count;
	char	*host = "localhost"; /* host to use if none supplied */
	long long int 	port = SERVER_TCP_PORT, bytes_to_read;
	int		sd, n, quit = 1; /* socket descriptor and socket type */
	FILE	*fp;
	char command[100];
	struct stat fstat;
	struct hostent *hp; /* pointer to host information entry */
	struct sockaddr_in server; /* an Internet endpoint address */
	struct PDU {
		char type;
		long long length;
		char data[BUFLEN+5];
	} rpdu, tpdu;



	/* Create a stream socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host))
		bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if (inet_aton(host, (struct in_addr *) &server.sin_addr)){
		fprintf(stderr, "Can't get server's address\n");
		exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't connect \n");
		exit(1);
	}

	while (true) {
		printf("Client> ");
	  fgets(command, 200, stdin);
	  get_args(command, &count);
		// printf("%d\n", count);
		if (strcmp(args[0], "get") == 0) {
			for (int i = 1; i < count; i++) {
				//get user message
				tpdu.length = strlen(args[i]);
				// tpdu.length = read(0, tpdu.data, BUFLEN-1); //
				strcpy(tpdu.data, args[i]);
				tpdu.type = 'D';
				write(sd, (char *)&tpdu, sizeof(tpdu));

				//file size obtain
				int val;
				read(sd, &val, sizeof(val));
				bytes_to_read = ntohl(val);
				printf("%d\n", bytes_to_read);

				int r1, r2;
				r1 = r2 = 1;
				read(sd, (char *)&rpdu, sizeof(rpdu));
				// printf("%c\n", rpdu.type);
				if (rpdu.type == 'F') {
					fp = fopen(tpdu.data, "w");
					// printf("in ifs\n");
					while (r2) {
						r2 = fwrite(rpdu.data, sizeof(char), rpdu.length, fp);
						if (r2 == 0)
							break;
						r1 = read(sd, (char *)&rpdu, sizeof(rpdu));
						if (r1 < 0) {
							perror("read");
							break;
						}
						printf("%f %\r", r1/bytes_to_read);
					}
					fclose(fp);
					printf("100 %\n");
					printf("Transfer sucessful.\n");

				} else
					fprintf(stderr, "error %s", rpdu.data);
				// printf("%s\n", args[i]);
			}
		}
		else if (strcmp(args[0], "exit") == 0) {
			exit(EXIT_SUCCESS);
		}
	  else
			printf("command not recognized\n");

	}
	close(sd);
	return(0);
}
