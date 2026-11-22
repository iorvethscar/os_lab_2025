#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
  int result = fork();

  if (result < 0)
  {
    perror("fork");
    return 1;
  }

  if (result == 0)
  {
    printf("Ребенок %d: я закончил\n", getpid());
    exit(0);
  }

  printf("Родитель %d: 20 секунд не вызываю wait, ребенок стал зомби\n", getpid());
  fflush(stdout);
  sleep(20);

  wait(NULL);
  printf("РОдитель: вызвал wait,зомби теперь нет. Жду ещё 20 секунд\n");
  fflush(stdout);
  sleep(20);
  return 0;
}
