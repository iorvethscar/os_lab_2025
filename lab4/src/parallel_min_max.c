#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"
// // // // / // // /
#include <signal.h>

pid_t *child_pids = NULL;
int child_count = 0;

void KillAllChildren(int sig)
{
  for (int i = 0; i < child_count; i++)
  {
    kill(child_pids[i], SIGKILL);
  }
}
// // // // // / ///

int main(int argc, char **argv)
{
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  // // // // //
  int timeout = -1;
  // // // // //

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
					/// // / // / / / / 
                                      {"timeout", required_argument, 0, 0},
					// // // / // / / / 
				      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed must be > 0 \n");
              return 1;
             }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size must be > 0\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
	     if (pnum <= 0) {
              printf("pnum must be > 0\n");
              return 1;
             }
            break;
          case 3:
            with_files = true;
            break;
// // // // // // / // / / / // / / / / /
	  case 4:
	    timeout = atoi(optarg);
	    if (timeout <= 0) {
	      printf("timeout must be > 0\n");
	      return 1;
             }
	    break;
          default: //опечатка была defalut
            printf("Index %d is out of options\n", option_index);
        }
// // // // // // / // / / / /
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;
  int pipefd[2];
  if (!with_files) {
    if (pipe(pipefd) == -1)
    {
      perror("pipe");
      return 1;
    }
  }

  int piece = array_size / pnum;
// // // // // // // // //
  child_pids = malloc(sizeof(pid_t) * pnum);
// // // // // // // //
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
// // // // // // // // // // 
      if (child_pid > 0) {
        child_pids[child_count] = child_pid;   // родитель запоминает номер
        child_count += 1;
      }
// // // // // // // // /// // // 
      if (child_pid == 0) {
        // child process
	unsigned int begin = i * piece;
	unsigned int end;
        if (i == pnum - 1)
	{
          end = array_size;
        }
	else
	{
          end = (i + 1) * piece;
        }
	struct MinMax local = GetMinMax(array, begin, end);

        if (with_files) {
          // use files here
	  char filename[64];
          sprintf(filename, "min_max_%d.txt", i);
          FILE *f = fopen(filename, "w");
          if (f == NULL) {
            perror("fopen");
            return 1;
          }
          fprintf(f, "%d %d\n", local.min, local.max);
          fclose(f);
        } else {
          // use pipe here
	  close(pipefd[0]);
          write(pipefd[1], &local, sizeof(local));
          close(pipefd[1]);
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

// /// // // // / // // //// //
  if (!with_files)
  {
    close(pipefd[1]);
  }

  if (timeout > 0)
  {
    signal(SIGALRM, KillAllChildren);
    alarm(timeout);
  }

  int killed = 0;
  while (active_child_processes > 0)
  {
    int status;
    pid_t done = waitpid(-1, &status, WNOHANG);
    if (done > 0) // кто-то из детей завершился
    {
      if (WIFSIGNALED(status))
      {
        killed += 1;                 // его убили сигналом (сволочи)
      }
      active_child_processes -= 1;
    }
    else if (done == 0) // никто еще не завершился
    {
      usleep(1000);  // подождать 1 мс и заглянуть снова
    }
    else
    { // детей не осталось
      break;
    }
  }

  alarm(0);// все дети закончили, будилтник не нужен

  if (killed > 0)
  {
    printf("По таймауту убито %d детей\n", killed);
  }
// // /// /// /// /// // /// ///


  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++)
  {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files)
    {
      char filename[64];
      sprintf(filename, "min_max_%d.txt", i);
      FILE *f = fopen(filename, "r");
// // // // // // //
      if (f == NULL)
      {
        continue;
      }
// // // /// // /// /
      fscanf(f, "%d %d", &min, &max);
      fclose(f);
      remove(filename);
    }
    else
    {
      struct MinMax local;
///////////////////////////////
      int bytes = read(pipefd[0], &local, sizeof(local));
      if (bytes != sizeof(local))
      {
        break;
      }
// / // // / / // /// 
      min = local.min;
      max = local.max;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  if (!with_files) 
  {
    close(pipefd[0]);
  }
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
// // / // / / // / 
  free(child_pids);
// // / / / // / / /
  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
