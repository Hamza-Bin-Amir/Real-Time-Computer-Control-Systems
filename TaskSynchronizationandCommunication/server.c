// server.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>

const char *client_qname = "/client_queue";
const char *server_qname = "/server_queue";

void *server_thread(void *arg);
void *compute_prime(void *arg);

int main() {
    mqd_t client_queue, server_queue;
    pthread_t server_tid;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);

    client_queue = mq_open(client_qname, O_CREAT | O_RDWR, 0666, &attr);
    server_queue = mq_open(server_qname, O_CREAT | O_RDWR, 0666, &attr);

    pthread_create(&server_tid, NULL, server_thread, (void *)&client_queue);

    pthread_join(server_tid, NULL);

    mq_close(client_queue);
    mq_close(server_queue);
    mq_unlink(client_qname);
    mq_unlink(server_qname);

    return 0;
}

void *server_thread(void *arg) {
    mqd_t client_queue = *((mqd_t *)arg);
    mqd_t server_queue;
    int n, result;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);

    server_queue = mq_open(server_qname, O_CREAT | O_RDWR, 0666, &attr);

    while (1) {
        mq_receive(client_queue, (char *)&n, sizeof(int), NULL);

        if (n == 0) {
            break;
        }

        pthread_t compute_thread;
        pthread_create(&compute_thread, NULL, compute_prime, &n);
        pthread_join(compute_thread, (void **)&result);

        mq_send(server_queue, (char *)&result, sizeof(int), 0);
    }

    mq_send(server_queue, (char *)&n, sizeof(int), 0); // send the termination signal

    pthread_exit(NULL);
}

void *compute_prime(void *arg) {
    int candidate = 2;
    int n = *((int *)arg);

    while (1) {
        int factor;
        int is_prime = 1;

        for (factor = 2; factor < candidate; ++factor) {
            if (candidate % factor == 0) {
                is_prime = 0;
                break;
            }
        }

        if (is_prime) {
            if (--n == 0) {
                pthread_exit((void *)candidate);
            }
        }
        ++candidate;
    }

    return NULL;
}
