#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include<math.h>
#include<time.h>

int generator(int left, int right){
     int num = rand()%(right - left + 1);
     num+=left;
     return num;
}

struct cab{
    int id;
    int type;    
    int num_slots;
}caabs[1000];

struct rider{
    int id;
    int pref;
    int max_wait_time;
    int ride_time;
}riders[1000];

struct server{
    int id;
    int empty;
}servers[1000];

pthread_mutex_t mutex_cabs[1000], mutex_servers[1000];
pthread_t cabtids[1000], ridertids[1000], servertids[1000];
int M, N, K;

void accept_ride(int rider_id, int cab_id){
    caabs[cab_id].num_slots--;
    printf("Rider %d is travelling in cab %d. %d seats left\n",rider_id, cab_id, caabs[cab_id].num_slots);
    return;
}

void end_ride(int rider_id, int cab_id){
    int check = 0;
    while(!check){
        if(!pthread_mutex_trylock(&mutex_cabs[cab_id])){
            caabs[cab_id].num_slots++;
            check = 1;
        }
    }
    return;
}

int book_ride(int id){
    int check = 0;
    long double start = time(NULL);
    while(!check){
        int j = M;
        while(j--){
            long double curr = time(NULL);
            if((int)(curr-start)>riders[id].max_wait_time){
                printf("Rider %d has left at time %ds: Time Error\n", riders[id].id, (int)(curr-start));
                fflush(stdout);
                return 0;                
            }
            if(!pthread_mutex_trylock(&mutex_cabs[M-j]) && riders[id].pref == caabs[M-j].type && caabs[M-j].num_slots){
                accept_ride(id, M-j);
                pthread_mutex_unlock(&mutex_cabs[M-j]);
                sleep(riders[id].ride_time);
                end_ride(id, M-j);
                printf("Rider %d has reached his location in cab %d.\n", riders[id].id, caabs[M-j].id);
                fflush(stdout);
                pthread_mutex_unlock(&mutex_cabs[M-j]);
                return 1;
            }
            pthread_mutex_unlock(&mutex_cabs[M-j]);

        }
    }
    return 0;
}

void make_payment(int id){
    int check = 0;
    while(!check){
        int j = K;
        while(j--){
            if(!pthread_mutex_trylock(&mutex_servers[K-j]) && servers[K-j].empty){
                printf("Rider %d using server %d to make payment\n", id, servers[K-j].id);
                fflush(stdout);
                servers[K-j].empty = 0;
                pthread_mutex_unlock(&mutex_servers[K-j]);
                sleep(2);
                pthread_mutex_lock(&mutex_servers[K-j]);
                printf("Rider %d has paid through server %d\n",id, servers[K-j].id);
                fflush(stdout);
                servers[K-j].empty = 1;
                pthread_mutex_unlock(&mutex_servers[K-j]);
                check = 1;
                return;
            }
            pthread_mutex_unlock(&mutex_servers[K-j]);
        }
    }
    return;
}

void *rider_init(void *args){
    int id = *(int *)args;
    riders[id].max_wait_time = generator(40,70);
    riders[id].pref = generator(0,1);
    riders[id].ride_time = generator(5,15);
    printf("Rider %d has arrived. It has a ride time of %ds, will travel only by car type %d and will not wait for more than %ds\n", id, riders[id].ride_time, riders[id].pref, riders[id].max_wait_time);
    if(book_ride(id))
        make_payment(id);
    return NULL;
}

void *cab_init(void *args){
    int id = *(int *)args;
    caabs[id].type = generator(0,1);
    if(caabs[id].type)
        caabs[id].num_slots = 2;
    else
        caabs[id].num_slots = 1;
    printf("Cab of id %d of type %d is available\n",id, caabs[id].type);
}

void *server_init(void *args){
    int id = *(int *)args;
    servers[id].empty = 1;
}

int main(void){
    srand(time(NULL));
    printf("Enter number of cabs : ");
    scanf("%d", &M);
    printf("Enter number of riders : ");
    scanf("%d", &N);
    printf("Enter number of servers : ");
    scanf("%d", &K);
    int num_riders = K;
    if(!M || !N || !K){
        printf("Simulation ended\n");
        return 0;
    }
    
    int i = M;    
    while(i--){
        caabs[M-i].id = M-i;
        usleep(100);
        pthread_create(&cabtids[M-i], NULL, cab_init, (void *)&caabs[M-i].id);
    }
    i = N;    
    while(i--){
        riders[N-i].id = N-i;
        usleep(100);
        pthread_create(&ridertids[N-i], NULL, rider_init, (void *)&riders[N-i].id);
    }
    i = K;    
    while(i--){
        servers[K-i].id = K-i;
        usleep(100);
        pthread_create(&servertids[K-i], NULL, server_init, (void *)&servers[K-i].id);
    }
    i = N;
    while(i--)
        pthread_join(ridertids[N-i], NULL);
    printf("Simulation ended\n");
    return 0;
}

