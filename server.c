#include "definitions.h"

int main(int argc, char *argv[])
{
	Packet pkt_data[3];
	pthread_t bucket, battery, bakery, calculate;
	void *rc_bucket, *rc_battery, *rc_bakery, *rc_calc;
	int i;
	for(i=0;i<3;i++)
	{	
		new_Packet(&pkt_data[i],i);
		new_Param(&pkt_data[i]);
	}

	int err_bucket = pthread_create(&bucket, NULL, bucket_func, (void *) &pkt_data[0]);
	int err_battery = pthread_create(&battery, NULL, battery_func, (void *) &pkt_data[1]);
	int err_bakery = pthread_create(&bakery, NULL, bakery_func, (void *) &pkt_data[2]);
	int err_calc = pthread_create(&calculate, NULL, calc_func, NULL);
	
	int bucket_join = pthread_join(bucket,&rc_bucket);
	// printf("Thread 1 joins %d \n", (int) rc_bucket);
	int battery_join = pthread_join(battery,&rc_battery);
	// printf("Thread 2 joins %d \n", (int) rc_battery);
	int bakery_join = pthread_join(bakery,&rc_bakery);
	// printf("Thread 3 joins %d \n", (int) rc_bakery);
	int calc_join = pthread_join(calculate,&rc_calc);
	// printf("Thread 4 joins %d \n", (int) rc_calc);
	
	return 0;
}

//Packet Setup Code
void new_Packet(Packet *pkt,int type)
{
	pkt->packet_type = type+1;
	pkt->max_E = 0.0;
	pkt->min_E = 0.0;
	pkt->max_P = 0.0;
	pkt->min_P = 0.0;
	pkt->T_end = 0;
	pkt->T_run = 0;
	pkt->check_bit = 0;
}

void new_Param(Packet *packet)
{
	//Initialise base parameters before iterative handling
	if(packet->packet_type == 1){
	
		packet->max_E = 10.0;
		packet->min_E = -2.0;
		packet->max_P = 6.0;
		packet->min_P = -2.0;	
	
	}
	if(packet->packet_type == 2){
	
		packet->max_E = 20.0;
		packet->min_E = 0.0;
		packet->max_P = 6.0;
		packet->T_end = 6.5;
		packet->min_P = 0.0;	
	
	}
	if(packet->packet_type == 3){
	
		packet->max_E = 20.0;
		packet->min_E = 0.0;
		packet->max_P = 6.0;
		packet->min_P = 0.0;
		packet->T_end = 6.5;
		packet->T_run = 4;	
		packet->check_bit = 0;
	
	}
	
}

void Packet_Details(Packet *pkt, int i){

	printf("Packet Payload\n");
	printf("--------------------------\n");
	printf("Type : %d\n", pkt->packet_type);
	printf("Iteration Number: %d\n", i);
	printf("Dispatched Power: %d\n",P_dispatch);
	printf("P(i) Bucket: %f\n", Pi_Bucket);
	printf("P(i) Battery: %f\n", Pi_Battery);
	printf("P(i) Bakery: %f\n", Pi_Bakery);
	// printf("E(i) Bucket: %f\n", Ei_Bucket_present);
	// printf("E(i) Battery: %f\n", Ei_Battery_present);
	// printf("E(i) Bakery: %f\n", Ei_Bakery_present);
	
	// printf("K(i) Bucket: %f\n", Ki_Bucket);
	// printf("K(i) Battery: %f\n", Ki_Battery);
	// printf("K(i) Bakery: %f\n", Ki_Bakery);
	// printf("E(i)_OLD Bucket: %f\n", Ei_Bucket_past);
	// printf("E(i)_OLD Battery: %f\n", Ei_Battery_past);
	// printf("E(i)_OLD Bakery: %f\n", Ei_Bakery_past);
	printf("--------------------------\n");
}

int rand_num_gen(){

  return (int)(rand() % 10);
}


//Thread Code
void * bucket_func(void *packet)
{
	int i;
	for(i=0;i<ITER;i++)
	{
		Bucket_Energy((Packet *)packet);	
		Bucket_Agility((Packet *)packet);
		Bucket_Reserve((Packet *)packet);
		// printf("BUCKET: ITER %d\n",i);
		Packet_Details((Packet *)packet,i);
		Barrier_Sync();
	}
	return (void *)1;

}

void * battery_func(void *packet)
{
	int i;
	for(i=0;i<ITER;i++)
	{
		Battery_Energy((Packet *)packet);	
		Battery_Agility((Packet *)packet,i);
		Battery_Forced((Packet *)packet);
		// printf("BATTERY: ITER %d\n",i);
		Packet_Details((Packet *)packet,i);
		Barrier_Sync();
	}
	
	return (void *)2;
}

void * bakery_func(void *packet)
{
	int i;
	for(i=0;i<ITER;i++)
	{
		Bakery_Energy((Packet *)packet);	
		Bakery_Agility((Packet *)packet,i);
		Bakery_Forced((Packet *)packet);
		// printf("BAKERY: ITER %d\n",i);
		Packet_Details((Packet *)packet,i);
		Barrier_Sync();	
		
	}
	return (void *)3;
}

void *calc_func()
{
	int i;
	for(i=0;i<ITER;i++)
	{
		pthread_mutex_lock(&m);
		while(wakeme_up)
			pthread_cond_wait(&cv,&m);
		// printf("CALC: I AM AWAKE. LETS COMPUTE\n");
		pthread_mutex_unlock(&m);
		Compute_Power();
		Barrier_Sync();
		wakeme_up = 1;
		// printf("CALC: GOING TO SLEEP\n");
	}
	return (void *)4;
}

void Barrier_Sync()
{
	int my_generation;
	pthread_mutex_lock(&m);
	if(count++ < NUM_OF_THREADS-2)                     
	{
		// printf("COUNT #1: %d\n",count);
		my_generation = generation;             
		while(my_generation == generation)
			pthread_cond_wait(&BarrierQueue, &m);
	}
	else
	{
		// printf("COUNT VAL %d\n",count);
		if(count++ == NUM_OF_THREADS-1)
		{
			wakeme_up = 0;
			pthread_cond_broadcast(&cv);
			int my_generation = generation;      
			while(my_generation == generation)
				pthread_cond_wait(&BarrierQueue, &m);
			
		}
		else
		{
			count=1;
			generation++;
			pthread_cond_broadcast(&BarrierQueue);
		}
	}
	pthread_mutex_unlock(&m);
}




/* Algorithm Related Functions */

void Compute_Power(){

	P_dispatch = rand_num_gen();
	Total_P_Forced = P_Battery_Forced + P_Bakery_Forced;
	if(Total_P_Forced > P_dispatch)
	{
		P_Battery_send = P_Battery_Forced ;
		P_Bakery_send = P_Bakery_Forced;
	}
	else
	{	
		Power_required_battery=Ei_Battery_present/Ts;
		Power_required_bakery=Ei_Bakery_present/Ts;

		if(Ki_Bakery > Ki_Battery)
		{
			
			if(Power_required_battery >= P_dispatch)
			{
				P_Battery_send=P_dispatch;
				P_Bakery_send=0;
			}
			else
			{
				P_Battery_send=Power_required_battery;
				P_Bakery_send=P_dispatch-P_Battery_send;
			}			
		
		}
		else
		{
			if(Power_required_bakery >= P_dispatch)
			{
				P_Bakery_send=P_dispatch;
				P_Battery_send=0;
			}
			else
			{
				P_Bakery_send=Power_required_bakery;
				P_Battery_send=P_dispatch-P_Bakery_send;
			}

		}		

	}
	P_Bucket_send = min(P_Bucket_reserved, (P_dispatch - P_Battery_send - P_Bakery_send));
	Pi_Bucket = P_Bucket_send;
	Pi_Battery = P_Battery_send;
	Pi_Bakery = P_Bakery_send;
	Remaining_Power= P_dispatch - P_Bakery_send - P_Battery_send - P_Bucket_send; 
}

/*Bucket Functions*/

void Bucket_Energy(Packet *pack)
{
	if((Pi_Bucket >= pack->min_P) && (Pi_Bucket <= pack->max_P))
	{
		if((Ei_Bucket_past >= pack->min_E) &&(Ei_Bucket_past <= pack->max_E))
		{
			Ei_Bucket_present = Ei_Bucket_past + Ts * Pi_Bucket;
		} 
	} 	
}

void Bucket_Agility(Packet *pack)
{
	
	Ki_Bucket = (pack->max_E-Ei_Bucket_present)/(Ts*pack->max_P);
		
}

void Bucket_Reserve(Packet *pack)
{
	P_Bucket_reserved = min(pack->max_P, ((pack->max_E-Ei_Bucket_present)/Ts));
}

/*Battery functions*/

void Battery_Energy(Packet *pack)
{
	if((Pi_Battery >=0) &&(Pi_Battery <= pack->max_P))
	{
		if((Ei_Battery_past >=0) &&(Ei_Battery_past <= pack->max_E))
		{
			Ei_Battery_present = Ei_Battery_past + Ts * Pi_Battery;
		} 
	} 	
}

void Battery_Agility(Packet *pack, int k)
{
	
	Ki_Battery = pack->T_end - k - ((pack->max_E-Ei_Battery_present)/(Ts*pack->max_P));
		
}

void Battery_Forced(Packet *pack)
{
	
	if( Ki_Battery > 1 )
	{
			P_Battery_Forced=0;
	}
	else if( (Ki_Battery<=1) && (Ki_Battery>0) )
	{
			P_Battery_Forced = pack->max_P*(1-Ki_Battery);
	}
	else if( Ki_Battery == 0 )
	{
			P_Battery_Forced = pack->max_P;
			
	}
		
}

/*Bakery Functions*/

void Bakery_Energy(Packet *pack)
{
	if((Ei_Bakery_past >= 0) && (Ei_Bakery_past <= pack->max_E))
	{
			Ei_Bakery_present = Ei_Bakery_past + Ts * (pack->max_P * pack->check_bit);
	} 
	 	
}

void Bakery_Agility(Packet *pack, int k)
{		
	Ki_Bakery = pack->T_end - pack->T_run - k;	
}

void Bakery_Forced(Packet *pack)
{
	if(Ki_Bakery>1)
	{
		P_Bakery_Forced=0;
	}
	else if(Ki_Bakery==0)
	{
		P_Bakery_Forced= pack->max_P;
	}
		
}

