#include "cs537.h"
#include "request.h"
#include <pthread.h>


// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too
#define MAX_BUF 10000
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 
pthread_cond_t fill = PTHREAD_COND_INITIALIZER; 
int buffer[MAX_BUF];
int full = 0, use = 0, count = 0;
int bufs;

void put(int value){
	buffer[full] = value;
	full = (full +1) % bufs;
	count++;
}

int get(){
	int tmp = buffer[use];
	use = (use + 1) % bufs;
	count--;
	return tmp;
}

void getargs(int *port,int *threads, int *bufs, int argc, char *argv[])
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <port> <threads> <buffers>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
	*threads = atoi(argv[2]);
	*bufs = atoi(argv[3]);
	
}

void *worker(void *arg){
	while(1){
		pthread_mutex_lock(&mutex);
		while(count == 0){
			pthread_cond_wait(&fill,&mutex);
		}
		int fd = get();
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
		requestHandle(fd);
		Close(fd);
	}
	return 0;
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, threads, clientlen;
    struct sockaddr_in clientaddr;
	//testing github - again

    getargs(&port, &threads, &bufs, argc, argv);

	pthread_t tid[threads];
	int i=0;
	
	for(i=0;i<threads;i++){
		pthread_create(&tid[i],NULL,worker,NULL);
		}

    listenfd = Open_listenfd(port);
    while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
		pthread_mutex_lock(&mutex);
		while(count == bufs){
			pthread_cond_wait(&empty,&mutex);
		}
		put(connfd);
		pthread_cond_signal(&fill);
		pthread_mutex_unlock(&mutex);
	}	
	return 0;
	// 
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work.
	// 
}


    


 
