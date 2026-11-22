#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <getopt.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// SADDR и SLEN - не константы, а сокращения для типа и его размера,
// поэтому остаются макросами
#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

// Использование: ./udpclient --ip 127.0.0.1 --port 20001 --bufsize 1024
int main(int argc, char **argv) {
  char *ip = NULL;
  int port = -1;     // было #define SERV_PORT 20001
  int bufsize = -1;  // было #define BUFSIZE 1024

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

  int sockfd, n;
  char *sendline = malloc(bufsize);
  char *recvline = malloc(bufsize + 1);
  struct sockaddr_in servaddr;

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  // inet_pton возвращает 0 для некорректной строки и -1 при ошибке,
  // поэтому проверяем <= 0 (в оригинале было < 0)
  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
    fprintf(stderr, "bad address: %s\n", ip);
    exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  write(1, "Enter string\n", 13);

  while ((n = read(0, sendline, bufsize)) > 0) {
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, SLEN) == -1) {
      perror("sendto problem");
      exit(1);
    }

    int m = recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL);
    if (m == -1) {
      perror("recvfrom problem");
      exit(1);
    }
    recvline[m] = '\0';  // в оригинале строка не завершалась нулём

    printf("REPLY FROM SERVER= %s\n", recvline);
  }
  free(sendline);
  free(recvline);
  close(sockfd);
}
