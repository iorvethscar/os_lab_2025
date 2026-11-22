#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// SADDR - не константа, а сокращение для типа, поэтому остаётся макросом
#define SADDR struct sockaddr

// Использование: ./tcpserver --port 10050 --bufsize 100
int main(int argc, char *argv[]) {
  int port = -1;     // было #define SERV_PORT 10050
  int bufsize = -1;  // было #define BUFSIZE 100

  static struct option options[] = {{"port", required_argument, 0, 'p'},
                                    {"bufsize", required_argument, 0, 'b'},
                                    {0, 0, 0, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "", options, NULL)) != -1) {
    switch (c) {
      case 'p': port = atoi(optarg); break;
      case 'b': bufsize = atoi(optarg); break;
      default: return 1;
    }
  }
  if (port <= 0 || port > 65535 || bufsize <= 0) {
    printf("Usage: %s --port \"1..65535\" --bufsize \"num > 0\"\n", argv[0]);
    return 1;
  }

  const size_t kSize = sizeof(struct sockaddr_in);

  int lfd, cfd;
  int nread;
  char *buf = malloc(bufsize);
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(lfd, (SADDR *)&servaddr, kSize) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(lfd, 5) < 0) {
    perror("listen");
    exit(1);
  }

  while (1) {
    unsigned int clilen = kSize;

    if ((cfd = accept(lfd, (SADDR *)&cliaddr, &clilen)) < 0) {
      perror("accept");
      exit(1);
    }
    printf("connection established\n");
    fflush(stdout);

    while ((nread = read(cfd, buf, bufsize)) > 0) {
      write(1, buf, nread);
    }

    if (nread == -1) {
      perror("read");
      exit(1);
    }
    printf("connection closed\n");
    fflush(stdout);
    close(cfd);
  }
}
