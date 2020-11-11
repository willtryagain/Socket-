#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#define min(a, b) a < b? a:b
#define SIZE 1000000
#define TIME 30

typedef long long ll;

struct stat stat_buf;
char output[SIZE], temp[SIZE], name[SIZE], time_left[SIZE];

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

void write_file(int in_fd) {
  float p = 0;
  int out_fd, fd;
  if ((out_fd = creat(output, S_IRUSR|S_IWUSR|O_TRUNC)) < 0) {
    perror("creat");
    return;
  }

  ll len = min(end, SIZE-1);
	if (len == 0)
		return;

  for (ll i = 1; i*len <= end; ++i) {
  	//cursor to start
	write(1, "\r", strlen("\r"));
	  
  	//read string
	if ((fd = read(in_fd, &temp, len)) < 0) {
		perror("read");
		return;
	}
	if ((fd = write(in_fd, &temp, len)) < 0) {
		perror("write");
		return;
	}
	//display progress
	p = (float)i*len*100/end;
	sprintf(temp, "%.2f%% ", p);
	change(p);
	strcat(temp, time_left);
	fd = write(1, temp, strlen(temp));
	if ((fd = write(1, temp, strlen(temp))) < 0) {
		perror("write");
		return;
	}	  
  }
  if (end%len > 0) {
	  //cursor to start
	write(1, "\r", strlen("\r"));
	  
  	//read string
	if ((fd = read(in_fd, &temp, end%len)) < 0) {
		perror("read");
		return;
	}
	if ((fd = write(in_fd, &temp, end%len)) < 0) {
		perror("write");
		return;
	}
	//display progress
	p = 100;
	sprintf(temp, "%.2f%% ", p);
	change(p);
	strcat(temp, time_left);
	fd = write(1, temp, strlen(temp));
	if ((fd = write(1, temp, strlen(temp))) < 0) {
		perror("write");
		return;
	}	  
  }

}

int main() {
	char input[] = "input.txt";
	char output[] = "output.txt";
	int fd = open(input, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return 0;
	}
	write_reverse(fd);
	
	return 0;
}	
