#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <time.h>

#define MAX_Random 500
#define NUM_THREADS 5

int reader_num=0;
int value=0;

pthread_mutex_t m=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c_writer=PTHREAD_COND_INITIALIZER;
pthread_cond_t c_reader=PTHREAD_COND_INITIALIZER;

void *writer(void* pArg);
void *reader(void* pArg);
void delay(int number_of_seconds);

int main(int argc,char *argv[])
{
    pthread_t tid1[5],tid2[5];
    int i;
    int tNum[NUM_THREADS];
    for ( i = 0; i < 5; i++)
    {
        tNum[i]=i;
        if(pthread_create(&tid1[i],NULL,writer,&tNum[i])!=0)
        {
            fprintf(stderr,"Unable to create writer thread\n");
            exit(1);
        }
    }
    
    for ( i = 0; i < 5; i++)
    {
        tNum[i]=i;
        if(pthread_create(&tid2[i],NULL,reader,&tNum[i])!=0)
        {
            fprintf(stderr,"Unable to create reader thread\n");
            exit(1);
        }
    }
    for (size_t i = 0; i < 5; i++)
    {
        pthread_join(tid1[i],NULL);
        pthread_join(tid2[i],NULL);
    }
    printf("parent quiting");
    return 0;

}

void *writer(void* pArg)
{
    int *p=(int*)pArg;
    int myNum=*p;
    delay(rand()%MAX_Random);
    fprintf(stderr,"writer %d pthread creat\n",myNum);
    pthread_mutex_lock(&m);
    while (1)
    {
        if (reader_num>0)
        {
            pthread_cond_wait(&c_writer,&m);
        }
        if(reader_num<0)
        {
            printf("reader_num error");
            exit(1);
        }
        if (reader_num==0)
        {
            value=rand()%255;
            break;
        }
    }
    pthread_mutex_unlock(&m);
    fprintf(stderr,"pthread writer id: %d readers:%d value:%d\n",myNum,reader_num,value);
    return 0;


}
void *reader(void* pArg)
{
    int local_variance;
    int *p=(int*)pArg;
    int myNum=*p;
    delay(rand()%MAX_Random);
    fprintf(stderr,"reader %d pthread creat\n",myNum);
    pthread_mutex_lock(&m);
    reader_num=reader_num+1;
    local_variance=value;
    pthread_mutex_unlock(&m);
    fprintf(stderr,"pthread reader id: %d readers:%d value:%d\n",myNum,reader_num,local_variance);
    pthread_mutex_lock(&m);
    reader_num=reader_num-1;
    if (reader_num==0)
    {
        pthread_cond_signal(&c_writer);
    }
    pthread_mutex_unlock(&m);
    return 0;
}
void delay(int number_of_seconds) 
{ 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds) 
        ; 
} 