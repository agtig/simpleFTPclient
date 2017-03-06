#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/** Error handling function, that prints the error */
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/** main function */
int main(int argc, char *argv[])
{
    int sockfd, portno, n;          // creating variables for socket, port and n
    struct sockaddr_in serv_addr;   // defining struct
    struct hostent *server;         // setting alias for struct
    
    // print welcome messages and ask user to input username and password
    printf("\nWelcome to a small non-userfriendly ftp-client!\n\n");
    printf("Please enter username and password (no input uses [anonymous test@dtu.dk]: [USERNAME PASSWORD]\n");
    char user[80], passwd[80], uspw[160];   // creating arrays for hold username and password
    //fgets(uspw,159,stdin);
    //if () {
    strcpy(user, "anonymous");
    strcpy(passwd, "test@dtu.dk");
    //} else {
    //    copy entered values in uspw to user and paswd
    //}
    
    char buffer[256];       // creating input/output buffer
    char lastInput[256];    // creating variable to hold last input
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]); // converting entered port to integer and save to portno
    sockfd = socket(AF_INET, SOCK_STREAM, 0);   // open socket
    
    // check if there was an error opening the socket
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    // check of the entered hostname exists/are reachable
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // print the server and port we are connecting to
    printf("Connecting to: %s on port: %d\n", argv[1], portno);
    
    // reset variables
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // set variables
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    // check that we are connected
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
       
    if (portno != 21) {
        printf("Port number is not 21\nEntering listening mode...\n");
        n = recv(sockfd,buffer,sizeof(buffer),0);   // receive file from server
                                                    // save in buffer n receives a line number
        buffer[n]='\0';                             // add \0 to the end of buffer
        printf("%s\n",buffer);                      // print buffer containing received message
        if (sizeof(buffer) >= 1023) {
            FILE * fp;
            fp = fopen("downloadedFile.txt", "w");
            fprintf(fp, "%s\n",buffer);                      // print buffer containing received message
            fclose(fp);
            printf("\n\nReceived file saved as \"downloadedFile.txt\"\n");
        }
    } else {
        // print welcome message
        n = recv(sockfd,buffer,sizeof(buffer),0);   // receive welcome message from server
                                                    // save in buffer n receives a line number
        buffer[n]='\0';                             // add \0 to the end of buffer
        printf("%s\n",buffer);                      // print buffer containing received message

        /** automated login START */
        bzero(buffer,256);                          // clear buffer
        printf("Sending 'hello' to server...\n");
        strcpy(buffer, "hello\r\n");                // save 'hello' in buffer
        send(sockfd, buffer, strlen(buffer), 0);    // send buffer through socket

        bzero(buffer,256);                          // clear buffer
        n = recv(sockfd,buffer,sizeof(buffer),0);   // receive server response
        buffer[n]='\0';
        printf("%s\n",buffer);                      // print buffer

        bzero(buffer,256);                          // clear buffer
        printf("Logging in to server using the entered credentials...\n");
        strcpy(buffer, "USER ");                    // assemble user login string
        strcat(buffer, user);                       // add the entered username
        strcat(buffer, "\r\n");                     // add return and new line
        printf("using USER %s\n", user);
        send(sockfd, buffer, strlen(buffer), 0);    // send buffer through socket

        bzero(buffer,256);                          // cleat buffer
        n = recv(sockfd,buffer,sizeof(buffer),0);   // receive server response
        buffer[n]='\0';
        printf("%s",buffer);                        // print buffer

        bzero(buffer,256);                          // clear buffer
        strcpy(buffer, "PASS ");                    // assemble password login string
        strcat(buffer, passwd);                     // add the entered password
        strcat(buffer, "\r\n");                     // add return and new line
        printf("and PASS %s\n", passwd);
        send(sockfd, buffer, strlen(buffer), 0);    // send buffer through socket

        bzero(buffer,256);                          // clear buffer
        n = recv(sockfd,buffer,sizeof(buffer),0);   // receive server response
        buffer[n]='\0';
        printf("%s\n",buffer);                      // print buffer
        /** automated login END */

        do {
            printf("Please enter the message: ");
            bzero(buffer,256);                      // reset buffer
            fgets(buffer,255,stdin);                // save entered input in buffer
            strcpy(lastInput, buffer);              // copy entered input to lastInput (used to end the while loop)

            n = write(sockfd,buffer,strlen(buffer));// send the entered command through the socket
            // check for errors
            if (n < 0) 
                 error("ERROR writing to socket");

            bzero(buffer,256);                      // reset buffer
            n = read(sockfd,buffer,255);            // read server response
            // check for errors
            if (n < 0) 
                 error("ERROR reading from socket");

            printf("%s\n",buffer);                  // print buffer

        // while loop needs the quit command 3 times before it quits. Don't know why yet
        } while(strncmp(lastInput, "quit", 4) != 0 || strncmp(lastInput, "QUIT", 4) != 0);
    }    
    close(sockfd);                              // close socket
    return 0;                                   // close program
}