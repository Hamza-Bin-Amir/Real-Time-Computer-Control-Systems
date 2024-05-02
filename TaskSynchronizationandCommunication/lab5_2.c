#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <errno.h>

struct Msg {char str[20]; unsigned int priority; };
int errno;
int messages_sent = 0;
int messages_received = 0;
const char *qname = "/myque";  
sem_t sem_receiver;

void *Receiver(void *arg)  
{
   mqd_t rx;
   struct mq_attr my_attrs;
   ssize_t n;
   char inBuf[50];
   unsigned int priority;

   sleep(2);

   rx = mq_open(qname, O_RDONLY);  

   if (rx < 0 ) perror("Receiver mq_open:");  

   mq_getattr(rx, &my_attrs);     

   while (messages_received < 10) {
      sem_wait(&sem_receiver); // Wait for the signal from Sender
      n = mq_receive(rx, inBuf, sizeof(inBuf), &priority);
      inBuf[n] = '\0';
      if (n < 0) perror("Receiver mq_receive:");  
      printf("Receiver: Received message = %s. Priority = %u\n", inBuf, priority);
      messages_received++;
   }

   mq_close(rx);   
   pthread_exit(NULL);
}
   
void *Sender(void *arg)  
{
   int retcode, i;
   mqd_t tx;
   struct Msg myMsg;
   unsigned int msg_priority[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};  

   sleep(2);

   tx = mq_open(qname, O_WRONLY);  

   if (tx < 0) perror("Sender mq_open:");  

   for (i = 0; i < 10; i++) {
      sprintf(myMsg.str, "This is message %d", i+1);  
      myMsg.priority = msg_priority[i];    
      printf("Sender: Message sent = %s. Priority = %u\n", myMsg.str, myMsg.priority);
      retcode = mq_send(tx, myMsg.str, (size_t)strlen(myMsg.str), myMsg.priority);
      if (retcode < 0) perror("Sender mq_send:");  
      sem_post(&sem_receiver); // Signal Receiver that a message is sent
      messages_sent++;
      sleep(1); // Introduce delay between sending messages
   }

   mq_close(tx);   
   pthread_exit(NULL);
}

int main(void *arg)
{
   mqd_t trx;
   mode_t mode;
   int oflags;
   struct mq_attr my_attrs;
   pthread_t Sender_thread, Receiver_thread;

   my_attrs.mq_maxmsg = 10;      
   my_attrs.mq_msgsize = 50;     

   oflags = O_CREAT | O_RDWR ;   
   
   mode = S_IRUSR | S_IWUSR;     

   trx = mq_open(qname, oflags, mode, &my_attrs);  

   if (trx < 0) perror("Main mq_open:");  

   sem_init(&sem_receiver, 0, 0); // Initialize semaphore

   printf("Creating Sender thread\n");
   if (pthread_create(&Sender_thread, NULL, &Sender, NULL) != 0) {
       printf("Error creating Sender thread.\n");
       exit(-1);
   }

   printf("Creating Receiver thread\n");
   if (pthread_create(&Receiver_thread, NULL, &Receiver, NULL) != 0) {
       printf("Error creating Receiver thread.\n");
       exit(-1);
   }

   pthread_join(Sender_thread, NULL);
   pthread_join(Receiver_thread, NULL);

   mq_unlink(qname);     
   printf("All messages sent and received. Exiting...\n");
   exit(0);
}


