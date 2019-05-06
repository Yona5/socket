// Cwk2: server.c - multi-threaded server using readn() and writen()

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "rdwrn.h"
#include <sys/utsname.h>
#include <dirent.h> 
#include <signal.h>
#include <time.h>       

// thread function
void *client_handler(void *);

void send_name(int);
void send_rand_num(int);
void send_uname(int);
void list_files(int);
void send_file(int, char[]);
int process_option();
void han_sig();
void file_exist(int);

char ser_addr[256];
time_t begtim;
time_t endtim;

// you shouldn't need to change main() in the server except the port number
int main(void)
{
    begtim = time(NULL);

    struct sigaction sigact;
    sigact.sa_handler = &han_sig;

    if(sigaction(SIGINT, &sigact, NULL) == -1){
        perror("sigaction");
        exit(EXIT_FAILURE);
    }



    int listenfd = 0, connfd = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t socksize = sizeof(struct sockaddr_in);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(50031);

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1) {
	perror("Failed to listen");
	exit(EXIT_FAILURE);
    }
    // end socket setup

    
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    while (1) {
	printf("Waiting for a client to connect...\n");
	connfd =
	    accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
	printf("Connection accepted...\n");

   //for printing server address
    inet_ntop( AF_INET, &client_addr, ser_addr, INET_ADDRSTRLEN );

	pthread_t sniffer_thread;
        // third parameter is a pointer to the thread function, fourth is its actual parameter
	if (pthread_create
	    (&sniffer_thread, NULL, client_handler,
	     (void *) &connfd) < 0) {
	    perror("could not create thread");
	    exit(EXIT_FAILURE);
	}
	//Now join the thread , so that we dont terminate before the thread
	//pthread_join( sniffer_thread , NULL);
	printf("Handler assigned\n");
    }

    // never reached...
    // ** should include a signal handler to clean up
    exit(EXIT_SUCCESS);
} // end main()
  

void han_sig(){
    endtim = time(NULL);
    // calculate elapsed time by finding difference (end - begin)
    printf("Time elapsed is %d seconds\n", (int)(endtim - begtim));
    printf("you hit ctrl + c. Exiting program.\n");
    exit(EXIT_SUCCESS);
}


// thread function - one instance of each for each connected client
// this is where the do-while loop will go
void *client_handler(void *socket_desc)
{
    //Get the socket descriptor
    int connfd = *(int *) socket_desc;
    
    int option = process_option(connfd);
    int x = 0;

    do{
        switch(option){

            case 97:{
               send_name(connfd);
               break;
            }
            case 98:{
                send_rand_num(connfd);
                break;
            }
            case 99:{
                send_uname(connfd);
                break;
            }
            case 100:{
                list_files(connfd);
                break;
            }
            case 101:{
                file_exist(connfd);
                break;
            }
            default:
                break;
        }
    }while(x!=0);

    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    // always clean up sockets gracefully
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    return 0;
}  // end client_handler()



void send_name(int socket){
    char name[] = "Yonas Tilahun\nID: S1719046\nServer address: ";
    strcat(name, ser_addr);
    size_t len = strlen(name) + 1;
    writen(socket, (unsigned char *) &len, sizeof(size_t));
    writen(socket, (unsigned char *) name, len);
}// end get_and_send_employee()

void send_rand_num(int socket){
    //intialize random number generator
    srand(time(NULL));
    int num = 999;
    char s1[1024] = "The Generated numbers are:\n ";
    
    //get str length 
    int str_len = snprintf( NULL, 0, "%d", num );
    // allocate memory 
    char* string = malloc( str_len + 1 );
    
    int i;
    for(i = 0; i < 5; i++){
        num = rand()%1000;
        snprintf( string, str_len + 1, "%d", num);
        strcat(s1, string);
        strcat(s1, "\n ");
    }
    //free the allocated memory 
    free(string);
    size_t len = strlen(s1) +1;
    writen(socket, (unsigned int *) &len, sizeof(size_t));
    writen(socket, (unsigned int *) s1, len);
}

void send_uname(int socket){
    struct utsname sys_info;
    size_t payload_length = sizeof(sys_info);

    errno = 0;
    if (uname(&sys_info) != 0) {
      perror("uname");
      exit(EXIT_FAILURE);
    }

    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));
    writen(socket, (unsigned char *) &sys_info, payload_length); 

}

void list_files(int socket){
    //open directory 
    DIR *dir = opendir ("./upload");
    // pointer for a directory 
    struct dirent *d;
    
    char dest[1024];
    //list of files
    char l_files[1024];
    if (dir != NULL){
        //readdir returns a pointer to a dirent structure
        while((d = readdir(dir))){

            strncpy(dest, d->d_name, strlen(d->d_name) +1);
            strcat(l_files, dest);
            strcat(l_files, " \n");
        }
        closedir(dir);
    }
    else{
        perror("Couldn't open the directory");
    }
    printf("%s\n", l_files);
    size_t len = strlen(l_files) +1;
    writen(socket, (unsigned int *) &len, sizeof(size_t));
    writen(socket, (unsigned int *) l_files, len);
}

void send_file(int socket, char location[]){

    char buff[2064];
    FILE *f;
    f = fopen(location,"r");
    size_t y = fread(buff, sizeof(unsigned char), 2064, f);
    fclose(f);
    writen(socket, (unsigned char *) &y, sizeof(size_t));
    writen(socket, (unsigned char *) buff, y);
}

int process_option(int socket){
    char option[1024];
    size_t len;
    readn(socket, (unsigned char *) &len, sizeof(size_t));
    readn(socket, (unsigned char *) &option, len);
    return *option;
}

void file_exist(int socket){
    char file_name[64];
    size_t len;

    readn(socket, (unsigned char *) &len, sizeof(size_t));
    readn(socket, (unsigned char *) file_name, len);

    char location[64] = "./upload/";

    strcat(location, file_name);
    if(access(location, F_OK) != -1){
        char ans[5] = "yes";
        size_t len = sizeof(ans) + 1;
        writen(socket, (unsigned char *) &len, sizeof(size_t));
        writen(socket, (unsigned char *) &ans, len);
        send_file(socket, location);
    }else{
        char ans[5] = "no";
        size_t len = sizeof(ans) + 1;
        writen(socket, (unsigned char *) &len, sizeof(size_t));
        writen(socket, (unsigned char *) &ans, len);
    }
}