#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// SADDR и SLEN - не константы, а сокращения для типа и его размера,
// поэтому остаются макросами
#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

// Использование: ./udpserver --port 20001 --bufsize 1024
int main(int argc, char *argv[]) {
  int port = -1;     // было #define SERV_PORT 20001
  int bufsize = -1;  // было #define BUFSIZE 1024

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

  int sockfd, n;
  // +1 под завершающий '\0': в оригинале mesg[n] = 0 при n == BUFSIZE
  // писал за границу массива
  char *mesg = malloc(bufsize + 1);
  char ipadr[16];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  memset(&servaddr, 0, SLEN);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(sockfd, (SADDR *)&servaddr, SLEN) < 0) {
    perror("bind problem");
    exit(1);
  }
  printf("SERVER starts...\n");
  fflush(stdout);

  while (1) {
    unsigned int len = SLEN;

    if ((n = recvfrom(sockfd, mesg, bufsize, 0, (SADDR *)&cliaddr, &len)) < 0) {
      perror("recvfrom");
      exit(1);
    }
    mesg[n] = 0;

    printf("REQUEST %s      FROM %s : %d\n", mesg,
           inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, ipadr, 16),
           ntohs(cliaddr.sin_port));
    fflush(stdout);

    if (sendto(sockfd, mesg, n, 0, (SADDR *)&cliaddr, len) < 0) {
      perror("sendto");
      exit(1);
    }
  }
}
