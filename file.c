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
char path[SIZE], temp[SIZE], name[SIZE], time_left[SIZE];

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

void write_file(int fd) {
  float p = 0;
  int out_fd;
  if ((out_fd = creat(path, S_IRUSR|S_IWUSR|O_TRUNC)) < 0) {
    perror("creat");
    return;
  }

  ll len = min(end, SIZE-1);
	if (len == 0)
		return;



}
