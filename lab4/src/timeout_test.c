#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

pid_t child_pid;   // номер помощника. Глобальная: чтобы обработчик её видел

// Будильник зазвонил -> убиваем помощника
void on_alarm(int sig) {
  char msg[] = "НАЧАЛЬНИК: время вышло, убиваю помощника\n";
  write(1, msg, sizeof(msg) - 1);
  kill(child_pid, SIGKILL);
}

int main(void) {
  child_pid = fork();

  if (child_pid == 0) {
    printf("ПОМОЩНИК: начинаю долгую работу (10 секунд)\n");
    fflush(stdout);
    sleep(1);                           // "очень медленная" работа
    printf("ПОМОЩНИК: закончил\n");      // не успеет напечатать
    return 0;
  }

  signal(SIGALRM, on_alarm);   // обработчик будильника
  alarm(3);                    // таймаут 3 секунды

  int status;
  while (1) {
    pid_t done = waitpid(child_pid, &status, WNOHANG);
    if (done == 0) {           // помощник ещё работает
      printf("НАЧАЛЬНИК: заглянула, ещё работает\n");
      fflush(stdout);
      sleep(1);
    } else {                   // помощник завершился
      break;
    }
  }
  alarm(0);                    // отменить будильник, если помощник успел раньше

  if (WIFSIGNALED(status)) {
    printf("НАЧАЛЬНИК: помощник убит сигналом %d\n", WTERMSIG(status));
  } else if (WIFEXITED(status)) {
    printf("НАЧАЛЬНИК: помощник закончил сам, код %d\n", WEXITSTATUS(status));
  }
  return 0;
}
