#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

int main()
{

    int  sockfd, clilen, n;
    int arr[3];
    struct sockaddr_in servaddr, cliaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(51005);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Can\'t create socket, errno = %d\n", errno);
        exit(1);
    }

    if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        printf("Can\'t bind socket, errno = %d\n", errno);
        close(sockfd);
        exit(1);
    }

    while(1) {
        clilen = sizeof(cliaddr);
        bzero(arr, 3);
        if ((n = recvfrom(sockfd, arr, 3 * sizeof(int), 0, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
            printf("Can\'t receive answer, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
        printf("Receive from client: [N: %d, ibeg: %d, iend: %d]\n", arr[0], arr[1], arr[2]);

        double x, next_x, square, result = 0.0;

        int N = arr[0];
        int ibeg = arr[1];
        int iend = arr[2];
//        printf("lol: [N: %d, ibeg: %d, iend: %d]\\n", N, ibeg, iend);

        double h = 2.0 / N;
        for (int i = ibeg; i < iend; i++) {
            x = h * i;
            next_x = h * (i + 1);
            square = (h * (sqrt(4 - pow(x, 2)) + sqrt(4 - pow(next_x, 2)))) / 2;
            result += square;
        }

        if(sendto(sockfd, (char*)&result, sizeof(double), 0, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            printf("Can\'t send answer, errno = %d\n", errno);
            close(sockfd);
            exit(1);
        }
        printf("Result: %f. Sending...\n", result);
//        printf("from %s %i\n",inet_ntoa(cliaddr.sin_addr),arr[0]);
    }
    return 0;
}
