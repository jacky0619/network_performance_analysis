/* External definitions for single-server queueing system, fixed run length. */
#include <conio.h>
#include <stdio.h>
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */
#include "lcgrand.c"

#define MAX			10000000
#define BUSY      	1  /* Mnemonics for server's being busy */
#define IDLE      	0  /* and idle. */
#define TOP			1
#define BOTTOM		0

int next_event_type, sto_num_in_q, en_num_in_q, total_num, en_server_status, sto_server_status, num_discard, last_field;
float sto_utilization, sim_time, time_end;

int discard;//1就表示下一個來的是buttom 且要被丟棄  
float  time_next_event[5];
//float queue stoqueue[MAX], enqueue[101];    
struct queue{
	int tmp; //當encoder IDLE後丟的第一個field會記錄1 
	float com_time;
}; 
struct queue stoqueue[MAX], enqueue[102];

FILE   *outfile;

int capacity[5] = {20,40,60,80,100};
float mean[3] = {1/59.94, 1/120.0};
float complexity[3] = {262.5, 399.5};
float cenc = 15800.0, cstorage = 1600.0;


void  initialize(int index);
void  timing(void);
void  en_arrive(int cap_index, int time_index);
void  en_depart(void);
void  sto_depart(void);
void  report(void);
//void  update_time_avg_stats(void);
float expon(float mean);

main()  /* Main function. */
{	

    /* Open input and output files. */

    outfile = fopen("project1.out", "w");

	time_end = 480.0*60.0;//480.0*60.0
	int i,j,num = 1;
	int number=1;
	for(i=0;i<5;i++){
		for(j=0;j<2;j++){
			printf("%d\n", number++);
				
		    fprintf(outfile, "Video Capture Server Simulation round  %d\n\n",num++);
		    fprintf(outfile, "Mean interarrival time%14.3f seconds\n\n",mean[j]);
		    fprintf(outfile, "Mean service time%19.3f seconds\n\n", complexity[j]);
		    fprintf(outfile, "Capacity %27d\n\n", capacity[i]);
		    fprintf(outfile, "Length of the simulation%12.3f seconds\n\n", time_end);
	
		    /* Initialize the simulation. */
		    initialize(j);
		
		    /* Run the simulation until it terminates after an end-simulation event
		       (type 3) occurs. */
		    do {
		        /* Determine the next event. */
		        timing();
		
		        switch (next_event_type) {
		            case 1:
		                en_arrive(i,j); //丟這一輪 encode server容量的編號
		                break;
		            case 2:
		                en_depart();
		                break;
		            case 3:
		                sto_depart();
		                break;
		            case 4:
		                report();
		                break;
		                
		        }
		        //printf("---NUMBER: %d  %d\n",en_num_in_q,last_field);///////////////////////////////////////// 
		        
		    /* If the event just executed was not the end-simulation event (type 3),
		       continue simulating.  Otherwise, end the simulation. */
		    } while (next_event_type != 4);
		}
	}
    fclose(outfile);

    return 0;
}


void initialize(int index)  /* Initialization function. */
{
    /* Initialize the simulation clock. */

    sim_time = 0.0;

    /* Initialize the state variables. */

    en_server_status   = IDLE;
    sto_server_status  = IDLE;
    last_field         = BOTTOM;
    en_num_in_q        = 0;
    sto_num_in_q 	   = 0; 
    num_discard 	   = 0;
    total_num 		   = 0;
    discard 		   = 0; 
    sto_utilization    = 0.0;
	int i;
	for(i=0; i<MAX; i++){
		stoqueue[i].com_time=0.0;
		stoqueue[i].tmp=0;
	}
	for(i=0; i<102; i++){
		enqueue[i].com_time=0.0;
		enqueue[i].tmp=0;
	}

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration.  The end-
       simulation event (type 3) is scheduled for time time_end. */

    time_next_event[1] = sim_time + expon(mean[index]);
    time_next_event[2] = 1.0e+30;
    time_next_event[3] = 1.0e+30;
    time_next_event[4] = time_end;
}


void timing(void)  /* Timing function. */
{
    int   i;
    float min_time_next_event = 1.0e+29;

    next_event_type = 0;

    /* Determine the event type of the next event to occur. */

    for (i = 1; i <= 4; ++i)
        if (time_next_event[i] < min_time_next_event) {
            min_time_next_event = time_next_event[i];
            next_event_type     = i;
        }

    /* Check to see whether the event list is empty. */

    /*if (next_event_type == 0) {

         The event list is empty, so stop the simulation 

        fprintf(outfile, "\nEvent list empty at time %f", sim_time);
        exit(1);
    }*/

    /* The event list is not empty, so advance the simulation clock. */

    sim_time = min_time_next_event;
}


void en_arrive(int cap_index, int time_index)  /* Arrival event function. */
{
	/*1.先排成下一個arrival到 time_next_event 
	  2.先判斷來的是BOTTOM還是TOP 
	  3.再判斷queue是否滿，要不要丟棄 
	  4.判斷encode server是否 busy*/
	
	/* Schedule next arrival. */
    time_next_event[1] = sim_time + expon(mean[time_index]);
    total_num++;
    
	if(last_field == TOP) //從上一個來的field判斷接下來要來的是哪一個 
		last_field = BOTTOM;
	else
		last_field = TOP;
		
	if(en_num_in_q >= capacity[cap_index]){ //queue已滿 丟掉這個field 
		if(last_field == TOP ){
			num_discard++;
			discard = 1;
		}
		else if(last_field == BOTTOM && discard==1){//代表這個BOTTOM的TOP已被丟棄 
			num_discard++;
			discard = 0;
		}
		else{//來的是BOTTOM就把原本queue裡的TOP移除 
			num_discard+=2;
			en_num_in_q--;
		}
	}
	else if(en_num_in_q < capacity[cap_index] && discard==1){
		num_discard++;
		discard = 0;
	}
	else{ //queue沒有滿，判斷server是否busy 
		++en_num_in_q;
		enqueue[en_num_in_q].com_time =  expon(complexity[time_index]);
				
	    if (en_server_status == IDLE){ //server不busy 直接進去服務 
	
	        /* Server is idle, so arriving customer has a delay of zero.  (The
	           following two statements are for program clarity and do not affect
	           the results of the simulation.) */
	
	
	        en_server_status = BUSY;
	        /* Schedule a departure (service completion). */
	
	        time_next_event[2] = sim_time + enqueue[en_num_in_q].com_time/cenc; //排程要完成服務的時間 
			sto_num_in_q++;
			stoqueue[sto_num_in_q].com_time = enqueue[en_num_in_q].com_time;
			stoqueue[sto_num_in_q].tmp = 1;//代表已經在storage queue加過了 
			en_num_in_q--;
	    }

	}
	
}


void en_depart(void)  /* Departure event function. */
{
	/**********          encode_server的departure                *************/

    /* Check to see whether the queue is empty. */
	float temp;//紀錄 
    if (en_num_in_q == 0){

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        en_server_status      = IDLE;
        time_next_event[2] = 1.0e+30;
    }

    else if(en_num_in_q >= 1){ //encode queue裡還有東西 
		int   i;
        /* The queue is nonempty, so decrement the number of customers in
           queue. */
        temp = enqueue[1].com_time;//暫存從encode 服務完的 complexity //////////////////////////////////////////////////////
        time_next_event[2] = sim_time + expon(temp)/cenc;//排程下一次的完成服務時間 
        en_num_in_q--;
        
        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= en_num_in_q; ++i) //Queue中一個人離開，全部往前移 
            enqueue[i].com_time = enqueue[i + 1].com_time;
            
    }
    
    /**********          storage_server 的 arrival                *************/
    /*1.先判斷要不要加進來
	  2.再判斷是否大於2  
	  3.再看storage server是否busy*/
    
    if(stoqueue[sto_num_in_q].tmp == 1){//不用加 
    	stoqueue[sto_num_in_q].tmp = 0; 
	}
	else if(stoqueue[sto_num_in_q].tmp == 0){
		sto_num_in_q++;
		stoqueue[sto_num_in_q].com_time = temp; //紀錄從encode server 移除之前的complexity

	}
	
	if(sto_num_in_q >= 2){
		if (sto_server_status == IDLE) {
			sto_server_status = BUSY;
			
			//printf("%20.10f %20.10f\n",stoqueue[1].com_time, stoqueue[2].com_time);////////////
			
			time_next_event[3] = sim_time + 0.1*(stoqueue[1].com_time + stoqueue[2].com_time) / cstorage; //排程要完成服務的時間 
			sto_utilization += 0.1*(stoqueue[1].com_time + stoqueue[2].com_time) / cstorage;
			
			/* 先把2個從storage queue中移除*/
			sto_num_in_q = sto_num_in_q - 2;
			int i;
			for (i = 1; i <= sto_num_in_q; ++i) //Queue中一個人離開，全部往前移 
	            stoqueue[i].com_time = stoqueue[i + 2].com_time;
		}
	}
    	
	
}

void sto_depart(void){
	 
	if(sto_num_in_q < 2){ //不到兩個
		sto_server_status =  IDLE;
		time_next_event[3] = 1.0e+30;
	}
	else if(sto_num_in_q >= 2){
		//printf("%20.10f %20.10f\n",stoqueue[1].com_time, stoqueue[2].com_time);
		sto_utilization += 0.1*(stoqueue[1].com_time + stoqueue[2].com_time) / cstorage;
		time_next_event[3] = sim_time + 0.1*(stoqueue[1].com_time + stoqueue[2].com_time) / cstorage;
		sto_num_in_q = sto_num_in_q - 2;
		int i;
		for (i = 1; i <= sto_num_in_q; ++i) //Queue中一個人離開，全部往前移 
            stoqueue[i].com_time = stoqueue[i + 2].com_time;
	}
	
}

void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */
    float fraction = (float)num_discard / (float)total_num;
	fprintf(outfile, "Server fraction of frames discard %19.10f %%\n\n",fraction * 100.0);
	//fprintf(outfile, "Discard: %d , Total: %d\n\n",num_discard,total_num);////////////////////
    fprintf(outfile, "Server utilization%35.10f %%\n\n",sto_utilization / time_end * 100.0);
    fprintf(outfile, "------------------------------------------------------------------------------\n");
}


/*void update_time_avg_stats(void)  
{
    float time_since_last_event;



    time_since_last_event = sim_time - time_last_event;
    time_last_event       = sim_time;



    area_server_status += sto_server_status * time_since_last_event;
}*/


float expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */

    return -mean * log(lcgrand(1));
}

