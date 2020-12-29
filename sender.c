#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "127.0.0.1"

int main(int argc, char const *argv[])
{

    for (int i = 0; i < 10; i++)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0)
        {
            printf("Could not create a new socket\n");
            return -1;
        }

        struct sockaddr_in serverAddress;
        memset(&serverAddress, 0, sizeof(serverAddress));

        serverAddress.sin_port = htons(SERVER_PORT);
        serverAddress.sin_family = AF_INET;

        int convertIP = inet_pton(AF_INET, SERVER_IP_ADDRESS, &serverAddress.sin_addr);

        if (convertIP < 0)
        {
            printf("IP Convertion failed!\n");
            return -1;
        }

        int connection;
        if ((connection = connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0)
        {
            perror("Connection failed!\n");
            return -1;
        }

        if (i > 4)
        {
            char recbuff[256];
            socklen_t length;
            strcpy(recbuff, "reno");
            length = strlen(recbuff);
            if ((setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, recbuff, length)) < 0)
            {
                printf("Errorset Reno");
                return 0;
            }
            memset(&recbuff, 0, sizeof(recbuff));
        }

        //Need to create a large file right here:

        char buffer[BUFSIZ]; //(((2^10)*2^10)-1) (Original file size is 1 mb).
        // printf("%s\n", buffer);
        memset(&buffer, 0, sizeof(buffer));

        int file_fd = open("1mb.txt", O_RDONLY);
        if (file_fd < 0)
        {
            perror("Error getting reference to file");
            return -1;
        }

        FILE *file = fopen("1mb.txt", "r");
        struct stat fileStat;
        fstat(file_fd, &fileStat);

        // printf("File size is: %ld\n", fileStat.st_size);

        int dataCount = 0;
        // int read;
        int b;
        int remainingData = fileStat.st_size;
        long int offset = 0;
        

        while (remainingData > 0)
        {
            b = fread(&buffer, BUFSIZ, 1, file);
            // long int offset = 0;

            printf("Sending... %d\n", i);

            while (((b = sendfile(sockfd, file_fd, &offset, BUFSIZ)) > 0) && (remainingData > 0))
            {
                dataCount += b;
                remainingData -= b;
                // printf("remaining data: %d\n", remainingData);
            }
        }
        fclose(file);

        // printf("File is at %p, data count = %d\n", file, dataCount);

        // printf("Message (of %d bytes) sent successfully!\n", dataCount);

        memset(&buffer, 0, sizeof(buffer));
        if (i == 9)
        {
            printf("\nWaiting for server to reply: \n");

            if ((recv(sockfd, &buffer, sizeof(buffer), 0)) >= 0)
            {
                printf("Message from server:\n%s\n", buffer);
            }
            else
            {
                printf("No reply from server :(\n");
            }
        }

        close(sockfd);
        printf("Closing Session\n");
    }

    return 0;
}
