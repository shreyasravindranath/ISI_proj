#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define Ts 1
#define min(a,b) (abs(a) > abs(b) ? b : a)
#define ITER 20
#define NUM_OF_THREADS 5

int count = 1;
int wakeme_up = 1;
int generation = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t BarrierQueue = PTHREAD_COND_INITIALIZER;


typedef struct packet_data
{
	/*common*/
	int packet_type;
	float max_E;
	float min_E;
	float max_P;
	float min_P;
	
	/*Battery and Bakery*/
	int T_end;
	
	/*Bakery*/
	int T_run;
	int check_bit;
	
	
}Packet;


/*Bucket variables*/
float Ei_Bucket_present;
float Pi_Bucket;
float Ki_Bucket;
float P_Bucket_reserved;
float Ei_Bucket_past = 5;

/*Battery variables*/
float Ei_Battery_present;
float Pi_Battery;
float Ki_Battery;
float P_Battery_Forced;
float Ei_Battery_past = 5;

/*Bakery variables*/
float Ei_Bakery_present;
float Pi_Bakery;
float Ki_Bakery;
int run_flag;
float P_Bakery_Forced;
float Ei_Bakery_past = 10;

float Total_P_Forced,P_Battery_send,P_Bakery_send, P_Bucket_send;
float Power_required_battery, Power_required_bakery;	
float Remaining_Power;

int P_dispatch;


/*Function Declarations*/
void * bucket_func (void *);
void * battery_func (void *);
void * bakery_func (void *);
void * calc_func ();
void new_Packet(Packet *pkt, int);
void new_Param(Packet *pkt);
void Packet_Details(Packet *pkt, int i);
void Bucket_Energy(Packet *pack);
void Bucket_Agility(Packet *pack);
void Bucket_Reserve(Packet *pack);
void Battery_Energy(Packet *pack);
void Battery_Agility(Packet *pack, int k);
void Battery_Forced(Packet *pack);
void Bakery_Energy(Packet *pack);
void Bakery_Agility(Packet *pack, int k);
void Bakery_Forced(Packet *pack);
void Barrier_Sync();
void Compute_Power();
int rand_num_gen();




