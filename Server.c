#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include<wiringPi.h>


#define TCP_PORT 5100


static void *clnt_connection(void *arg);
int sendData(int fd, FILE* fp, char *ct, char *file_name);
void sendOk(FILE* fp);
void sendError(FILE* fp);
 
pthread_mutex_t music_lock;
pthread_mutex_t motor_lock;

static pthread_t ptMusic, ptMotor;

extern void ledSet(int onoff);
extern void motorSet();
extern int playMusic(char *fileName);
extern int Play;
extern int MusicQuit;
extern int initGPIO();
extern int T;

char musicName[50];

int ledControl(int onoff){
	printf("ledControl! %d !!\n", onoff);
	if (onoff == 0){
		printf("led on\n");
		ledSet(onoff);
	}
	else{
		printf("ledOff~\n");
		ledSet(onoff);
	}
	return 0;
}

void *MusicControl(){
	printf("IN MUSIC FUNC %s\n", musicName);	
	if(pthread_mutex_trylock(&music_lock)!=EBUSY){
		playMusic(musicName);
		pthread_mutex_unlock(&music_lock);	
	}else{
		MusicQuit =1;
		delay(1000);
		pthread_join(ptMusic, 0);
		pthread_mutex_trylock(&music_lock);
		playMusic(musicName);
		pthread_mutex_unlock(&music_lock);
		
	}
	return NULL;
}

void *MotorControl(){
	printf("motor dir = %d\n", T);
	if(pthread_mutex_trylock(&motor_lock)!=EBUSY){
		motorSet();
		pthread_mutex_unlock(&motor_lock);
	}
	return NULL;
}
 
int main(int argc, char **argv)
{
	wiringPiSetup ();
	initGPIO();
	int serv_sock;
	pthread_t thread;
	struct sockaddr_in serv_addr, clnt_addr;
	unsigned int clnt_addr_size;
 
	pthread_mutex_init(&music_lock, NULL);
	pthread_mutex_init(&motor_lock, NULL);
	
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1) {
		perror("socket( )");
		return -1;
	}
 
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(TCP_PORT);

	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		perror("bind( )");
		return -1;
	}
 
	if (listen(serv_sock, 10) == -1) {
		perror("listen( )");
		return -1;
	}
 
	while (1) {
		int clnt_sock;
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		printf("Client IP : %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
		pthread_create(&thread, NULL, clnt_connection, &clnt_sock);
		pthread_join(thread, 0);
	};
 
	return 0;
}
 
void *clnt_connection(void *arg)
{
	int clnt_sock = *((int*)arg), clnt_fd;
	FILE *clnt_read, *clnt_write;
	char reg_line[BUFSIZ], reg_buf[BUFSIZ];
	char method[10], ct[BUFSIZ], type[BUFSIZ];
	char file_name[256], file_buf[256];
	char* type_buf;
	int i = 0, j = 0, len = 0;
 
	clnt_read = fdopen(clnt_sock, "r");
	//clnt_write = fdopen(dup(clnt_sock), "w");
	clnt_fd = clnt_sock;
 
	fgets(reg_line, BUFSIZ, clnt_read);
	fputs(reg_line, stdout);
 
	printf("\n");
	
	char optLine[50];
	char optStr[4][20];
	char opt[20], var[20];
	char* tok;
	int count = 0;

	strcpy(optLine, reg_line);

	tok = strtok(optLine, "&");
	
	while (tok != NULL) {
		strcpy(optStr[count++], tok);
		tok = strtok(NULL, "&");
	};
 
	for (i = 0; i < count; i++) {
		printf("optStr[i] : %s\n",optStr[i]);
		strcpy(opt, strtok(optStr[i], "="));
		strcpy(var, strtok(NULL, "="));
		
		printf("opt:%s ", opt);
		printf("var:%s \n", var);

		printf("%s = %s\n", opt, var);
		if (!strcmp(opt, "led")) { 
			if(!strcmp(var, "On")) ledControl(1);
			else ledControl(0);
		}

		if(!strcmp(opt,"music")){
			
			if(!strcmp(var, "play")){
				Play = 1;
				continue;
			}
			else if(!strcmp(var, "stop")){
				Play = 0;
				continue;
			}
			Play =1;
			musicName[0]='\0';
			strcat(musicName,"music/");
			strcat(musicName, var);
			strcat(musicName, ".wav");

			pthread_create(&ptMusic,NULL,MusicControl,NULL);
			printf("check!!\n");
		}
		if(!strcmp(opt, "motor")){
			T = var[0]-'0'-1;
			pthread_create(&ptMotor,NULL,MotorControl,NULL);
			
		}
 	}
	
	
	//strcpy(file_buf, file_name);
	//sendData(clnt_fd, clnt_write, ct, file_name);
 
	fclose(clnt_read);                                          
	//fclose(clnt_write);
 
	pthread_exit(0);                                            
	return (void*)NULL;
}
 
int sendData(int fd, FILE* fp, char *ct, char *file_name)
{
	/* 클라이언트로 보낼 성공에 대한 응답 메시지 */
	char protocol[] = "HTTP/1.1 200 OK\r\n";
	char server[] = "Server:Netscape-Enterprise/6.0\r\n";
	char cnt_type[] = "Content-Type:text/html\r\n";
	char end[] = "\r\n";                                      /* 헤더의 끝은 항상 \r\n */
	char buf[BUFSIZ];
	int len;
 
	fputs(protocol, fp);
	fputs(server, fp);
	fputs(cnt_type, fp);
	fputs(end, fp);
	fflush(fp);
 

 
	fd = open(file_name, O_RDWR);               /* 파일을 오픈한다. */
 
	do {
		len = read(fd, buf, BUFSIZ);                     /* 파일을 읽어서 클라이언트로 보낸다. */
		fwrite(buf, len, sizeof(char), fp);
	} while (len == BUFSIZ);
 
	fflush(fp);
	close(fd);                                                     /* 파일을 닫는다. */
 
	return 0;
}
 
void sendOk(FILE* fp)
{
	/* 클라이언트에 보낼 성공에 대한 HTTP 응답 메시지 */
	char protocol[] = "HTTP/1.1 200 OK\r\n";
	char server[] = "Server: Netscape-Enterprise/6.0\r\n\r\n";
 
	fputs(protocol, fp);
	fputs(server, fp);
	fflush(fp);
}
 
void sendError(FILE* fp)
{
	/* 클라이언트로 보낼 실패에 대한 HTTP 응답 메시지 */
	char protocol[] = "HTTP/1.1 400 Bad Request\r\n";
	char server[] = "Server: Netscape-Enterprise/6.0\r\n";
	char cnt_len[] = "Content-Length:1024\r\n";
	char cnt_type[] = "Content-Type:text/html\r\n\r\n";
 
	/* 화면에 표시될 HTML의 내용 */
	char content1[] = "<html><head><title>BAD Connection</title></head>";
	char content2[] = "<body><font size=+5>Bad Request</font></body></html>";
 
	printf("send_error\n");
	fputs(protocol, fp);
	fputs(server, fp);
	fputs(cnt_len, fp);
	fputs(cnt_type, fp);
	fputs(content1, fp);
	fputs(content2, fp);
	fflush(fp);
}
 
