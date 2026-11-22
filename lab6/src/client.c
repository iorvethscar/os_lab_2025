#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
// // // / // / / //
#include <pthread.h>
#include "multmodulo.h"
// // / // // / / /

struct Server {
  char ip[255];
  int port;
};

// // // // / // / / // / /
struct ServerArgs {
  struct sockaddr_in address;   //куда подключаться
  uint64_t begin;               //какие числа считает этот сервер
  uint64_t end;
  uint64_t mod;
  uint64_t result;              //сюда поток положит ответ сервера
};
// // // / / // / / / //

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
// // // / // /
  errno = 0;                    //сбрасываем старую ошибку
// // / // / /
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;
// // // // / // / /
  if (end == str || *end != '\0')   //не число или мусор после числа
  {
    return false;
  }
// // // / / // /

  *val = i;
  return true;
}

// // // // / // / / // / / //
//работа с одним сервером, каждый в своем потоке
void *ServerThread(void *args)
{
  struct ServerArgs *sargs = (struct ServerArgs *)args;

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0)
  {
    fprintf(stderr, "Socket creation failed!\n");
    exit(1);
  }

  if (connect(sck, (struct sockaddr *)&sargs->address, sizeof(sargs->address)) < 0)
  {
    fprintf(stderr, "Connection failed\n");
    exit(1);
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &sargs->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &sargs->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &sargs->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0)
  {
    fprintf(stderr, "Send failed\n");
    exit(1);
  }

  char response[sizeof(uint64_t)];
  //MSG_WAITALL - ждем все 8 байт
  if (recv(sck, response, sizeof(response), MSG_WAITALL) != sizeof(response))
  {
    fprintf(stderr, "Recieve failed\n");
    exit(1);
  }
  memcpy(&sargs->result, response, sizeof(uint64_t));

  close(sck);
  return NULL;
}
// // // / // / / // / / //

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; //255 - макс длина имени файла в linux

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
// // // / // / / //
        if (!ConvertStringToUI64(optarg, &k))
        {
          printf("k must be a number\n");
          return 1;
        }
// // / // / / // /
        break;
      case 1:
// // // / / // / /
        if (!ConvertStringToUI64(optarg, &mod) || mod == 0)
        {
          printf("mod must be a number > 0\n");
          return 1;
        }
// // / // // / / /
        break;
      case 2:
// // // / // / / / //
        if (strlen(optarg) >= sizeof(servers))   //иначе вылезем за массив
        {
          printf("path to servers file is too long\n");
          return 1;
        }
        strcpy(servers, optarg);
// // / // / / // /
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

// // // // / // / / // / / // /
  //читаем сервера из файла, в каждой строке ip:port
  FILE *file = fopen(servers, "r");
  if (file == NULL)
  {
    printf("can't open %s\n", servers);
    return 1;
  }

  unsigned int servers_num = 0;
  struct Server *to = malloc(sizeof(struct Server) * 100);   //до 100 серверов
  while (servers_num < 100 &&
         fscanf(file, " %254[^:]:%d", to[servers_num].ip, &to[servers_num].port) == 2)
  {
    servers_num++;
  }
  fclose(file);

  if (servers_num == 0)
  {
    printf("no servers in %s\n", servers);
    return 1;
  }

  if (servers_num > k)          //серверов не больше чем чисел
  {
    servers_num = k;
  }

  struct ServerArgs args[servers_num];
  pthread_t threads[servers_num];
  uint64_t piece = k / servers_num;   //по сколько чисел на сервер
// // // / // / / // / / //

  for (int i = 0; i < servers_num; i++) {
    //gethostbyname тут, а не в потоках: она не потокобезопасна
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

// // // / // / / // /
    memset(&args[i].address, 0, sizeof(args[i].address));
    args[i].address.sin_family = AF_INET;
    args[i].address.sin_port = htons(to[i].port);
    args[i].address.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    args[i].begin = i * piece + 1;
    if (i == servers_num - 1)
    {
      args[i].end = k;                    //последний до конца
    }
    else
    {
      args[i].end = (i + 1) * piece;
    }
    args[i].mod = mod;
  }

  //все сервера работают одновременно - поток на каждый
  for (int i = 0; i < servers_num; i++)
  {
    if (pthread_create(&threads[i], NULL, ServerThread, (void *)&args[i]))
    {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  //ждем все потоки и перемножаем ответы
  uint64_t answer = 1 % mod;
  for (int i = 0; i < servers_num; i++)
  {
    pthread_join(threads[i], NULL);
    answer = MultModulo(answer, args[i].result, mod);
  }
  printf("answer: %llu\n", (unsigned long long)answer);
// // / // / / // / / /

  free(to);

  return 0;
}
