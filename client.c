#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include "Betserver.h"

Betserver_header SendMessage;
Betserver_header RecvMessage;
Betserver_betnum BetMessage;
Betserver_result ResMessage;
Betserver_info   AckMessage;

int main(int argc, char **argv)
{
    int sockfd, readcheck, portnum;
    struct sockaddr_in serveraddr;
    struct hostent *serv;
    char *hostname;
    unsigned int betNumber=0;
    int ResStatus=0;
    
    /* validate command line arguments */
    if (argc != 3) {
        fprintf(stderr,"Please enter: ./client <hostname> <portnumber>\n");
        exit(0);
    }
    hostname = argv[1];
    portnum = atoi(argv[2]);

     /* address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portnum);

     /* server's DNS name */
    serv = gethostbyname(hostname);
    if (serv == NULL) {
        fprintf(stderr,"ERROR, invalid host : %s\n", hostname);
        exit(0);
    }

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("socket opening failed!!\n");
    else
        printf("socket opened sucessfully\n");
    

    /* connect with server */
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
        perror("connection failed!!\n");
    else
        printf("connection stablished successfully\n");
    

    makeHeader(PROTOCOL_VERSION, BETSERVER_OPEN, sizeof(SendMessage), 0, &SendMessage);
    if (send(sockfd, (Betserver_header*) &SendMessage, sizeof(SendMessage), 0) == -1)
        perror("\n Send Error!!");

    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    printf("\n Client Sends Status:BETSERVER_OPEN \n");
    printf(" |   Version = %u |    Type = %u |     Length = %d |   ClientID = %d |  \n", SendMessage.version, SendMessage.type,SendMessage.length, SendMessage.clientID);
    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");

    //Infinite loop for receiving message
    for (;;) {
        readcheck = read(sockfd, &RecvMessage, sizeof(RecvMessage));
        if ( readcheck <= 0) {
            perror("\nReceived invalid Bet\n\r");
            printf("\nClosing Socket..: %d", sockfd);
            close(sockfd);
            exit(1);
        } 
        else 
        {
            switch (RecvMessage.type) {
                case BETSERVER_ACCEPT: 
                    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                    printf("\nClient Receives Status:BETSERVER_ACCEPT\n");
                    printf("\n  |  Version = %u | Type = %u |  Length = %d | ClientID = %d |  \n",RecvMessage.version, RecvMessage.type,RecvMessage.length, RecvMessage.clientID);
                    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                    if ((read(sockfd, &AckMessage, sizeof(AckMessage))) <= 0) {
                        perror("\nReceive Error!!");
                        exit(1);
                    }
                    printf("\n |  BETSERVER Minimum Limit = %u | BETSERVER Maximum Limit = %u |  \n",AckMessage.minLimit, AckMessage.maxLimit);
                    betNumber = createBet(BETSERVER_NUM_MAX, BETSERVER_NUM_MIN);
                    BetMessage.bet = betNumber;
                    printf("\nBet made by client is : %u\n", betNumber);
                    
                    makeHeader(PROTOCOL_VERSION, BETSERVER_BET,sizeof(SendMessage) + sizeof(BetMessage),RecvMessage.clientID, &SendMessage);
                    
                    if (send(sockfd, (Betserver_header *) &SendMessage,sizeof(SendMessage), 0) == -1)
                        perror("\nsend Error!!\n");
                    if (send(sockfd, (Betserver_betnum *) &BetMessage, sizeof(BetMessage),0) == -1)
                        perror("\nsend Error!!\n");
                    
                    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                    printf("\nClient Sends Status:BETSERVER_BET \n");
                    printf("\n  |  Version = %u |  Type = %u |  Length = %d | ClientID = %d |  \n", SendMessage.version, SendMessage.type,SendMessage.length, SendMessage.clientID);
                    printf("\n |Bet made by client = %u \n", BetMessage.bet);
                    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                    break;
                                     
                case BETSERVER_RESULT: 
                    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                    printf("\nClient Receives Status:BETSERVER_RESULT\n");
                    printf("\n  |  Version = %u |  Type = %u |  Length = %d | ClientID = %d |  \n",RecvMessage.version, RecvMessage.type,RecvMessage.length, RecvMessage.clientID);
                    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                    if ((recv(sockfd, &ResMessage, sizeof(ResMessage), 0)) <= 0) {
                        perror("\nReceive Error!!\n");
                        exit(1);
                    }
                    printf("\n Current Bet Status: %d | **And Winner is: %u ** \n",ResMessage.status, ResMessage.winNum);
                    if (ResMessage.status == 1)
                        printf("\nYOU WON !!!\n");
                    else
                        printf("\nSORRY,YOU LOST!!! BETTER LUCK NEXT TIME\n");

                    ResStatus = 1;
                    break;                    
            }
            if (ResStatus == 1)
                break;
        }
    }//closing infinite loop
    close(sockfd);//socket close
    return 0;
}//main close
    


    
