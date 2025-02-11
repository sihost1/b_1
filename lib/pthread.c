#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t log_mutex;

void log_message(const char *message) {
    pthread_mutex_lock(&log_mutex);
    FILE *logFile = fopen("log.txt", "a");
    if (logFile) {
        time_t now = time(NULL);
        fprintf(logFile, "[%s] %s\n", ctime(&now), message);
        fclose(logFile);
    }
    pthread_mutex_unlock(&log_mutex);
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void *log_task(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 5; i++) {
        char msg[50];
        sprintf(msg, "Thread %d: Fibonacci(10) = %d", id, fibonacci(10));
        log_message(msg);
        usleep(100000);
    }
    return NULL;
}

int main() {
    pthread_mutex_init(&log_mutex, NULL);

    pthread_t threads[3];
    int ids[3] = {1, 2, 3};

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, log_task, &ids[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&log_mutex);
    return 0;
}
