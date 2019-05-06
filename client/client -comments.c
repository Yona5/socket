// Cwk2: client.c - message length headers with variable sized payloads
//  also use of readn() and writen() implemented in separate code module

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "rdwrn.h"
#include <sys/utsname.h>
#include <string.h>

// function prototype
void get_name(int);
void get_rand_num(int);
void get_uname(int);
void get_file_names(int);
void send_option(int, char *);
void get_file(int, char []);
void pre_file(int);

int main(void)
{
    // *** this code down to the next "// ***" does not need to be changed except the port number
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error - could not create socket");
	exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;

    // IP address and port of server we want to connect to
    serv_addr.sin_port = htons(50031);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // try to connect...
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
	perror("Error - connect failed");
	exit(1);
    } else
       printf("Connected to server...\n");

    // ***
    // your own application code will go here and replace what is below... 
    // i.e. your menu etc.
    int terminate = 1;
    do{
	    printf("\n");
	    printf("===========================\n");
	    printf("1. Get student information\n"); 
	    printf("2. Generate random numbers\n"); 
	    printf("3. Get server information\n"); 
	    printf("4. Get file names\n"); 
	    printf("5. Get file\n");
	    printf("6. Quit\n");
	    printf("===========================\n");
	    printf("Enter your choice: ");
	    
	    
	    int option;
	    scanf("%d", &option);

	    switch(option){

	        case 1:{
	           send_option(sockfd, "a");
	           get_name(sockfd);
	           break;
	        }
	        case 2:{
	        	send_option(sockfd, "b");
	        	get_rand_num(sockfd);
	            break;
	        }
	        case 3:{
	        	send_option(sockfd, "c");
	        	get_uname(sockfd);
	            break;
	        }
	        case 4:{
	        	send_option(sockfd, "d");
	        	get_file_names(sockfd);
	            break;
	        }
	        case 5:{
	        	send_option(sockfd, "e");
	        	pre_file(sockfd);
	            break;
	        }
	        case 6:{
	        	terminate = 0;
	        	printf("\n");
	        	printf("Thanks. Good bye!\n\n");
	            break;
	        }
	        default:
	            printf("Please enter the correct number\n\n");
	    }
	}while(terminate == 1);

    
    // *** make sure sockets are cleaned up
    close(sockfd);
    exit(EXIT_SUCCESS);
} // end main()

//get student info
void get_name(int socket){
    char name[256];
    size_t len;

    readn(socket, (unsigned char *) &len, sizeof(size_t));
    readn(socket, (unsigned char *) name, len);

    printf("Name: %s\n", name);
}//end get_name()

//get random numbers from the server
void get_rand_num(int socket){
    char number[256];
    size_t len;

    readn(socket, (unsigned int *) &len, sizeof(size_t));
    readn(socket, (unsigned int *) number, len);

    printf("Name info: %s\n", number);
}//end get_rand_num()

// get server information like OS
void get_uname(int socket){
    struct utsname sys_info;
    size_t payload_length = sizeof(sys_info);

    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) &sys_info, payload_length);
    //print the details of the info
    printf("system name = %s\n", sys_info.sysname);
    printf("node name   = %s\n", sys_info.nodename);
    printf("release     = %s\n", sys_info.release);
    printf("version     = %s\n", sys_info.version);
}//end get_uname

//get a list of files from the server
void get_file_names(int socket){
    char f_name[1024];
    size_t len;

    readn(socket, (unsigned char *) &len, sizeof(size_t));
    readn(socket, (unsigned char *) f_name, len);

    printf("File names: %s\n", f_name);
}//end get_file_names()

// send the option so that server can respond accordingly
void send_option(int socket, char *option){
    size_t len = strlen(option);
    writen(socket, (unsigned char *) &len, sizeof(size_t));
    writen(socket, (unsigned char *) option, len);

}//end send_option()

//get the content of a file from a server
void get_file(int socket, char ans[]){

    char buff[2064];
    size_t len;
	

    readn(socket, (unsigned char *) &len, sizeof(size_t));
    readn(socket, (unsigned char *) buff, len);
	//read the content of the file
	FILE *fp;
	fp = fopen(ans,"w");
	//copy the content
	fwrite(buff, sizeof(unsigned char), len, fp);
	fclose(fp);
	printf("file copied");
}//end get_file()

//check if file exist in the server 
void pre_file(int socket){

	printf("Type the name of the file: ");
	char ans[64];
	scanf("%s", ans);
	size_t length = strlen(ans) + 1;

    writen(socket, (unsigned char *) &length, sizeof(size_t));
    writen(socket, (unsigned char *) ans, length);

    char buff[48];
    size_t len;

    readn(socket, (unsigned char *) &len, sizeof(size_t));
    readn(socket, (unsigned char *) buff, len);

    //call the function that copies a file from a server
    if(strcmp(buff, "yes") == 0){
    	get_file(socket, ans);
    }else{
    	printf("%s doesn't exist or have the right permission.", ans);
    }

}
