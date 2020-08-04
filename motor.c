#include<stdio.h>
#include<wiringPi.h>

int T =4;

int setGPIO[12]={8,9,7,0,2,3,12,13,14, 21, 22, 23};
int dir[5][12]={

	{0,1,1, 0,1,1, 0,1,1, 0,1,1},
	{1,1,0, 0,1,1, 1,1,0, 1,1,0},

	{1,1,0, 0,1,1, 0,1,1, 1,1,0},

	{1,1,0, 0,1,1, 1,1,0, 1,1,0},
	
	{0,0,0,0,0,0,0,0,0,0,0,0}
};



void initGPIO(){
	int i;
	for(i=0; i<12; i+=3){
		pinMode(setGPIO[i], INPUT);
		pinMode(setGPIO[i+1], OUTPUT);
		pinMode(setGPIO[i+2], OUTPUT);
	}

}


void goDir(int d){
	int i;
	for( i=0; i<12; i++){
		digitalWrite(setGPIO[i], dir[d][i]);
	}
}


void motorSet(){
//	initGPIO();
	goDir(T);
}



