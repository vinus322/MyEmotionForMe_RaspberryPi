#include<stdlib.h>
#include<stdio.h>
#include<wiringPi.h>

#define LED 1 // BCM_GPIO 23




void ledSet (int onoff)
{

  pinMode (LED, OUTPUT);

  if(onoff==1)	digitalWrite (LED, 1) ; // On
  else		digitalWrite (LED, 0) ; // Off
 
}

