#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutex_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_b = PTHREAD_MUTEX_INITIALIZER;

void *Thread1(void *arg)
{
  pthread_mutex_lock(&mutex_a);
  printf("Thread1 захватил мьютекс a, пытаюсь b\n");
  sleep(1);
  pthread_mutex_lock(&mutex_b);    //здесь виснет навсегда
  printf("Thread1 захватил b\n");     //никогда не напечатается
  pthread_mutex_unlock(&mutex_b);
  pthread_mutex_unlock(&mutex_a);
  return NULL;
}

void *Thread2(void *arg)
{
  pthread_mutex_lock(&mutex_b);
  printf("Thread2 захватил мьютекс b, пытаюсь a\n");
  sleep(1);
  pthread_mutex_lock(&mutex_a);
  printf("Thread2 захватил a\n");
  pthread_mutex_unlock(&mutex_a);
  pthread_mutex_unlock(&mutex_b);
  return NULL;
}

int main(void)
{
  pthread_t t1, t2;
  pthread_create(&t1, NULL, Thread1, NULL);
  pthread_create(&t2, NULL, Thread2, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  printf("программа отработала\n"); //не увидим?
  return 0;
}
