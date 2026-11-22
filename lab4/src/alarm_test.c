#include <stdio.h>
#include <unistd.h>
#include <signal.h>

// Обработчик: эту функцию вызовет система, когда придёт SIGALRM
void on_alarm(int sig) {
  char msg[] = "БУДИЛЬНИК ЗАЗВОНИЛ!\n";
  write(1, msg, sizeof(msg) - 1);
}

int main(void) {
 // signal(SIGALRM, on_alarm);   // "когда придёт SIGALRM, вызови on_alarm"
  alarm(3);                    // "через 3 секунды пришли мне SIGALRM"

  for (int i = 1; i <= 6; i++) {
    printf("секунда %d, занимаюсь своими делами\n", i);
    fflush(stdout);
    sleep(1);
  }
  printf("конец программы\n");
  return 0;
}
