#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>

#define PORT 5060

long currentTime()
{
    struct timeval currentTime;

    gettimeofday(&currentTime, NULL);

    long t1 = currentTime.tv_usec / 1000;
    long t2 = (long)(currentTime.tv_sec) * 1000;

    return t1 + t2;
}

int main(int argc, char const *argv[])
{

    int listenSockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenSockfd == -1)
    {
        printf("Error creating socket\n");
        return -1;
    }

    struct sockaddr_in serverAddress;

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int letsBind = bind(listenSockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (letsBind == -1)
    {
        perror("Bind failed.\n");
        return -1;
    }

    if (listen(listenSockfd, 500) == -1)
    {
        printf("Listen failed.\n");
        return -1;
    }

    printf("Now listening on port %d\nWaiting for clients to join\n", PORT);

    struct sockaddr_in clientAddress;
    socklen_t clientAddrLen = sizeof(clientAddress);
    int clientFd;

    int i = 0;

    // long start = currentTime(); //Start MEASURING TIME

    while (i<10) // < 10
    {
        // sleep(2);
        printf("Listening...\n");
        // long total_time;
        double total_time;
        long cubic_time;
        long reno_time;
        double cubic_avg;
        double reno_avg;
        double sec = 1000.0;

        // memset(&buffer, '\0', sizeof(buffer));
        memset(&clientAddress, 0, clientAddrLen);
        clientFd = accept(listenSockfd, (struct sockaddr *)&clientAddress, &clientAddrLen);

        if (clientFd == -1)
        {
            perror("Could not accept client\n");
            return -1;
        }
        printf("Client accepted\n");

        //Handle changing CC Algorithm after 5 iterations.
        if (i > 4)
        {
            char recbuff[256];
            socklen_t length;
            strcpy(recbuff, "reno");
            length = strlen(recbuff);

            if (setsockopt(listenSockfd, IPPROTO_TCP, TCP_CONGESTION, recbuff, length) < 0)
            {
                printf("Errorset Reno");
                return -1;
            }
        }
        if (i <= 4)
        {
            cubic_time = currentTime(); //START TAKING TIME FOR CUBIC.
        }
        else
        {
            reno_time = currentTime(); //START TAKING TIME FOR RENO.
        }

        // char buffer[BUFSIZ];
        char buffer[BUFSIZ] = {'\0'};
        int size = sizeof(buffer);
        int isRecive = 1;
        int bytesRead = 0;

        //bytesSent should not be over 1 mb (1048576)
        while (isRecive > 0 && bytesRead < 1048576)
        {
            isRecive = recv(clientFd, &buffer, size, 0);
            bytesRead += isRecive;
        }

        printf("Received file of size: %d\n", bytesRead);
        long finish_round = currentTime();

        // memset(&buffer, 0, sizeof(buffer));

        if (i <= 4)
        {
            cubic_avg += (finish_round-cubic_time)/sec; //FINISH MEASURING TIME FOR CUBIC.
            // printf("Cubic sum so far: %lf\n", cubic_avg);
        }
        else
        {
            reno_avg += (finish_round - reno_time)/sec; //FINISH MEASURING TIME FOR CUBIC.
            // printf("Reno sum so far: %lf\n", reno_avg);
        }

        if (i == 9)
        {

            total_time = (cubic_avg) + (reno_avg);
            cubic_avg = cubic_avg / 5.0;
            reno_avg = reno_avg / 5.0;

            char reply[BUFSIZ];
            
            sprintf(reply, "Total time: %f seconds\n"
            "Cubic CC Algo avg: %f\n"
            "Reno CC Algo avg: %f\n", total_time, cubic_avg, reno_avg);

            printf("%s\n", reply); //Print time samples and send them to the client so he can enjoy it.

            int sentBytes = send(clientFd, &reply, sizeof(reply), 0);


            if (sentBytes <= 0)
            {
                printf("Error sending message\n");
            }
            else
            {
                printf("Message sent!\n");
            }
        }

        i++;

        // printf("------------------------------------------------\n\n\n");

    }
    printf("Connection ended.\n");
    

    int reuse = 1;
    setsockopt(listenSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    close(clientFd);
    close(listenSockfd);

    return 0;
}
