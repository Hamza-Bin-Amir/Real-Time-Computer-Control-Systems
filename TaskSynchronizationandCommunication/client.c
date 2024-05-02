// client.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>

const char *client_qname = "/client_queue";
const char *server_qname = "/server_queue";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *client_prompt_thread(void *arg);
void *client_print_thread(void *arg);

int main() {
    mqd_t client_queue, server_queue;
    pthread_t prompt_thread, print_thread;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);

    client_queue = mq_open(client_qname, O_CREAT | O_RDWR, 0666, &attr);
    server_queue = mq_open(server_qname, O_CREAT | O_RDWR, 0666, &attr);

    pthread_create(&prompt_thread, NULL, client_prompt_thread, (void *)&client_queue);
    pthread_create(&print_thread, NULL, client_print_thread, (void *)&server_queue);

    pthread_join(prompt_thread, NULL);
    pthread_join(print_thread, NULL);

    mq_close(client_queue);
    mq_close(server_queue);
    mq_unlink(client_qname);
    mq_unlink(server_qname);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}

void *client_prompt_thread(void *arg) {
    mqd_t client_queue = *((mqd_t *)arg);
    int input;

    while (1) {
        printf("Enter an integer (0 to exit): ");
        fflush(stdout); // Flush the output buffer to ensure immediate printing
        scanf("%d", &input);

        pthread_mutex_lock(&mutex);
        mq_send(client_queue, (char *)&input, sizeof(int), 0);
        pthread_cond_wait(&cond, &mutex); // Wait for the signal from the print thread
        pthread_mutex_unlock(&mutex);

        if (input == 0) {
            break;
        }
    }

    pthread_exit(NULL);
}

void *client_print_thread(void *arg) {
    mqd_t server_queue = *((mqd_t *)arg);
    int result;

    while (1) {
        mq_receive(server_queue, (char *)&result, sizeof(int), NULL);

        printf("Result: %d\n", result);
        fflush(stdout); // Flush the output buffer to ensure immediate printing

        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond); // Signal the prompt thread to continue
        pthread_mutex_unlock(&mutex);

        if (result == 0) {
            break;
        }
    }

    pthread_exit(NULL);
}
