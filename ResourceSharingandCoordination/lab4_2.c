#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NTHREADS 2
#define STRING_LENGTH 10

int string_index = 0;
char string_to_print[] = "0123456789";
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *func_A(void *arg) {
    int tmp_index;
    while (1) {
        pthread_mutex_lock(&mutex);
        while (string_index % 2 != 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        tmp_index = string_index;
        if (tmp_index < STRING_LENGTH) {
            printf("A%c ", string_to_print[tmp_index]);
            fflush(stdout);
            usleep(100000); // sleep 100 ms
        }
        string_index++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

void *func_B(void *arg) {
    int tmp_index;
    while (1) {
        pthread_mutex_lock(&mutex);
        while (string_index % 2 == 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        tmp_index = string_index;
        if (tmp_index < STRING_LENGTH) {
            printf("B%c ", string_to_print[tmp_index]);
            fflush(stdout);
            usleep(100000); // sleep 100 ms
        }
        string_index++;
        if (string_index == STRING_LENGTH) {
            printf("\n"); // Insert newline after printing '9'
        }
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

int main(void) {
    pthread_t threads[NTHREADS];
    int k;

    // Start threads
    for (k = 0; k < NTHREADS; k++) {
        if (k == 0) {
            printf("Starting Thread A\n");
            if (pthread_create(&threads[k], NULL, &func_A, NULL) != 0) {
                printf("Error creating thread A\n");
                exit(-1);
            }
        } else {
            printf("Starting Thread B\n");
            if (pthread_create(&threads[k], NULL, &func_B, NULL) != 0) {
                printf("Error creating thread B\n");
                exit(-1);
            }
        }
    }

    // Wait for threads indefinitely
    while(1) {
        usleep(1000000); // sleep for 1 second
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond); // Signal the condition variable
        string_index = 0; // Reset string index
        pthread_mutex_unlock(&mutex);
    }

    for (k = 0; k < NTHREADS; k++) {
        pthread_join(threads[k], NULL);
    }

    return 0;
}
