#include <stdio.h>
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */
#include "lcgrand.c"

float expon(float mean);
float complexity[3] = {262.5, 399.5};
float temp;
main() {
	int i,j;
	/*for(i=0;i<=1000000;i++){
		for(j=0;j<=1;j++){
			temp = expon(complexity[j]);
			printf("%20.10f\n",temp);
		}
	}*/
	for(i=1;i<=10;++i){
		printf("%d\n",i);
	}
	
}

float expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */

    return -mean * log(lcgrand(1));
}
