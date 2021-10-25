#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv)
{

    int sockfd, n, len, N, k;
    int sendarr[3];
    double answer, res = 0.0;
    struct sockaddr_in servaddr, cliaddr;
    unsigned short port;

    if(argc < 2 || argc > 3){
        printf("Usage: a.out <IP address> <port - default 51000>\n");
        exit(1);
    }

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

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Can\'t create socket, errno = %d\n", errno);
        exit(1);
    }

    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family      = AF_INET;
    cliaddr.sin_port        = htons(0);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0){
        printf("Can\'t bind socket, errno = %d\n", errno);
        close(sockfd);
        exit(1);
    }
    printf("Enter N: ");
    scanf("%i", &N);
    sendarr[0] = N;
    printf("Enter k: ");
    scanf("%i", &k);
    for (int i = 0; i < k; i++) {
        int ibeg = i * N / k;
        sendarr[1] = ibeg;
        int iend;
        if (i == k - 1) {
            iend = N - 1;
        }
        else {
            iend = (i + 1) * N / k - 1;
        }
        sendarr[2] = iend;
        if(sendto(sockfd, sendarr, 3 * sizeof(int), 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
            close(sockfd);
            exit(1);
        }
        printf("Send to server: [N: %d, ibeg: %d, iend: %d]\n", N, ibeg, iend);
        if((n = recvfrom(sockfd, (char*)&answer, sizeof (double), 0, (struct sockaddr *) NULL, NULL)) < 0){

            printf("Can\'t receive answer, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
        res += answer;
    }
    printf("Result: %lf", res);
    close(sockfd);
    return 0;
}
