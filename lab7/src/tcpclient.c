#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// SADDR и SIZE - не константы, а сокращения для типа и его размера,
// поэтому остаются макросами
#define SADDR struct sockaddr
#define SIZE sizeof(struct sockaddr_in)

// Использование: ./tcpclient --ip 127.0.0.1 --port 10050 --bufsize 100
int main(int argc, char *argv[]) {
  char *ip = NULL;
  int port = -1;
  int bufsize = -1;  // было #define BUFSIZE 100

  static struct option options[] = {{"ip", required_argument, 0, 'i'},
                                    {"port", required_argument, 0, 'p'},
                                    {"bufsize", required_argument, 0, 'b'},
                                    {0, 0, 0, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "", options, NULL)) != -1) {
    switch (c) {
      case 'i': ip = optarg; break;
      case 'p': port = atoi(optarg); break;
      case 'b': bufsize = atoi(optarg); break;
      default: return 1;
    }
  }
  if (ip == NULL || port <= 0 || port > 65535 || bufsize <= 0) {
    printf("Usage: %s --ip \"addr\" --port \"1..65535\" --bufsize \"num > 0\"\n",
           argv[0]);
    return 1;
  }

  int fd;
  int nread;
  char *buf = malloc(bufsize);
  struct sockaddr_in servaddr;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket creating");
    exit(1);
  }

  memset(&servaddr, 0, SIZE);
  servaddr.sin_family = AF_INET;

  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
    fprintf(stderr, "bad address: %s\n", ip);
    exit(1);
  }

  servaddr.sin_port = htons(port);

  if (connect(fd, (SADDR *)&servaddr, SIZE) < 0) {
    perror("connect");
    exit(1);
  }

  write(1, "Input message to send\n", 22);
  while ((nread = read(0, buf, bufsize)) > 0) {
    if (write(fd, buf, nread) < 0) {
      perror("write");
      exit(1);
    }
  }

  free(buf);
  close(fd);
  exit(0);
}
