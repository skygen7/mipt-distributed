#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

    int    sockfd, n, i;
    char   sendline[1000],recvline[1000];
    struct sockaddr_in servaddr;
    unsigned short port;

    if(argc < 2 || argc > 3){
        printf("Usage: a.out <IP address> <port - default 51000>\n");
        exit(1);
    }

    bzero(sendline,1000);
    bzero(recvline,1000);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    if(argc == 3){
        port = atoi(argv[2]);
        if(port == 0){
            printf("Invalid port\n");
            exit(-1);
        }
    } else {
        port = 51000;
    }
    servaddr.sin_port   = htons(port);

    if(inet_aton(argv[1], &servaddr.sin_addr) == 0){
        printf("Invalid IP address\n");
        exit(1);
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Can\'t create socket, errno = %d\n", errno);
        exit(1);
    }

    if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        printf("Can\'t connect, errno = %d\n", errno);
        close(sockfd);
        exit(1);
    }
    char ex[] = "Bye!\n";
    while (strcmp(ex, sendline) != 0) {
        printf("Your message => ");
        bzero(sendline, 1000);
        fgets(sendline, 1000, stdin);
        if( (n = write(sockfd,sendline, strlen(sendline)+1)) < 0){
            printf("Can\'t write, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }

        if ( (n = read(sockfd,recvline, 1000)) < 0){
            printf("Can\'t read, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
        printf("%s\n", recvline);
        if (strcmp(ex, recvline) == 0) {
            close(sockfd);
            exit(1);
        }
    }
    close(sockfd);
    return 0;
}
