#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

struct FactorialArgs
{
  uint64_t begin;
  uint64_t end;
};

uint64_t mod = 0;
uint64_t result = 1;  //общий результат для всех потоков
pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

void *ThreadFactorial(void *arg)
{
  struct FactorialArgs *args = (struct FactorialArgs *)arg;

  //локальное произведение
  uint64_t local = 1 % mod;
  for (uint64_t i = args->begin; i <= args->end; i++)
  {
    local = (local * (i % mod)) % mod;
  }

  //критическая секция
  pthread_mutex_lock(&result_mutex);
  result = (result * local) % mod;
  pthread_mutex_unlock(&result_mutex);

  return NULL;
}

int main(int argc, char **argv)
{
  long long k = -1;
  long long pnum = -1;
  long long mod_arg = -1;

  static struct option options[] = {{"pnum", required_argument, 0, 'p'},
                                    {"mod", required_argument, 0, 'm'},
                                    {0, 0, 0, 0}};
  int c;
  while ((c = getopt_long(argc, argv, "k:", options, NULL)) != -1)
  {
    switch (c)
    {
      case 'k': k = atoll(optarg); break;
      case 'p': pnum = atoll(optarg); break;
      case 'm': mod_arg = atoll(optarg); break;
      default: return 1;
    }
  }

  if (k < 0 || pnum <= 0 || mod_arg <= 0)
  {
    printf("Usage: %s -k \"num >= 0\" --pnum=\"num > 0\" --mod=\"num > 0\"\n",
           argv[0]);
    return 1;
  }
  if (mod_arg > UINT32_MAX)
  {
    printf("mod must be less than %u\n", UINT32_MAX);
    return 1;
  }

  mod = (uint64_t)mod_arg;
  result = 1 % mod;

  if (pnum > k) pnum = k;   //потоков не больше, чем чисел

  pthread_t *threads = malloc(sizeof(pthread_t) * pnum);
  struct FactorialArgs *args = malloc(sizeof(struct FactorialArgs) * pnum);

  uint64_t piece  = (pnum > 0) ? k / pnum : 0;
  for (long long i = 0; i < pnum; i++) {
    args[i].begin = i * piece + 1;
    args[i].end = (i == pnum - 1) ? (uint64_t)k : (i + 1) * piece;
    if (pthread_create(&threads[i], NULL, ThreadFactorial, &args[i]) != 0)
    {
      perror("pthread_create");
      return 1;
    }
  }

  for (long long i = 0; i < pnum; i++)
  {
    pthread_join(threads[i], NULL);
  }

  printf("%lld! mod %llu = %llu\n", k, (unsigned long long)mod,
         (unsigned long long)result);

  free(threads);
  free(args);
  return 0;
}
