#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

double a[18][16], b[16][18], c[18][18];

typedef struct thread_info thread_info_t;

struct thread_info 
{
    double a, b, c;
    int i, j;
};

void *compute_C_ij( void *arg)
{
    int k;
    thread_info_t *info1 = arg;
    double C_ij = 0.0;    

    for (k = 0; k < 16; k++)
    {
        C_ij  += (a[info1->i][k]*b[k][info1->j]);
    }

    info1->c=C_ij;
}

int main(void)
{
    pthread_t thread1[18][18];
    thread_info_t info1[18][18];
    int i = 0, j = 0;

    for(i = 0; i < 18; i++)
    {
        for(j = 0; j < 16; j++)
        {
            a[i][j] = (i+1) + (j+1);
            b[j][i] = (j+1) + 2*(i+1);            
        }
    }

    for(i = 0; i < 18; i++)
    {
        for(j = 0; j < 18; j++)
        {
            info1[i][j].i = i;
            info1[i][j].j = j;
            
            if (pthread_create(&thread1[i][j], NULL, &compute_C_ij,(void *) &info1[i][j]) > 0)
            {
                printf("Error in creating thread\n");
                exit(1);
            }
        }   
    }

    for (i = 0; i <= 17; i++)
    {
        for(j = 0; j <= 17; j++)
        {
            pthread_join(thread1[i][j], NULL);
        }
        c[i][i] = info1[i][i].c;
        printf("C = %lf\n", c[i][i]);
    }
}

