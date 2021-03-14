#include "Betserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define PORT 2222
#define TRUE 1

Betserver_result ResMessage;
Betserver_header SendHeader;
Betserver_header RecvHeader;
Betserver_info   AckMessage;
Betserver_betnum BetMessage;

int main(int argc, char *argv[])
{
    int sd, sdMax, i, HeadSock , clientSock[BETSERVER_NUM_CLIENTS] , newSock,
    clientsMax = BETSERVER_NUM_CLIENTS , dataread, address_len,request;  
    struct sockaddr_in address;  
    struct timeval timeoutLimit;
    unsigned int win_num[BETSERVER_NUM_CLIENTS];
    int connections = TRUE; 
    unsigned int winner=0;
    fd_set readfd;  

    //initialising all client 
    for (i = 0; i < clientsMax; i++)  
    {  
        clientSock[i] = 0;  
    }  

    //create a socket 
    if( (HeadSock = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket creation failed!!");  
        exit(1);  
    }  

    //Allow multiple connections 
    if( setsockopt(HeadSock, SOL_SOCKET, SO_REUSEADDR, (char *)&connections, sizeof(connections)) < 0 )  
    {  
        perror("setsockopt error");  
        exit(1);  
    }

    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(PORT);  

    //bind the socket 
    if (bind(HeadSock, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("binding failed!!");  
        exit(1);  
    }  
    printf("Listening on portnum: %d \n", PORT);  

    if (listen(HeadSock, 5) < 0)  
    {  
        perror("listen error!!");  
        exit(1);  
    }  

    //accept the incoming connection 
    address_len = sizeof(address);  
    puts(" Incoming connections .....\n");  

    //selecting winner
    winner = createWinner(BETSERVER_NUM_MAX, BETSERVER_NUM_MIN);
    
    FD_ZERO(&readfd);  
    FD_SET(HeadSock, &readfd);  
    sdMax = HeadSock;
   
    while(TRUE)  
    { 
        FD_ZERO(&readfd);  
        FD_SET(HeadSock, &readfd);  
        sdMax = HeadSock;
        timeoutLimit.tv_sec = 15;
        
        //Adding sockets
        for ( i = 0 ; i < clientsMax ; i++)  
        {  
            sd = clientSock[i];  

            //add valid sockets in read list 
            if(sd > 0)  
                FD_SET( sd , &readfd);  

            if(sd > sdMax)  
                sdMax = sd;  
        }  

        // timeout limit is set to 15sec
        request = select( sdMax + 1 , &readfd , NULL , NULL , &timeoutLimit);  

        if ((request < 0) && (errno!=EINTR)) 
        {  
            printf("Error in selection \n");  
        }  
        else if ( request == 0 ) //selected reuest
        {
            for (i = 0; i < clientsMax; i++)  
            {  
                if ((sd = clientSock[i]) && (i != HeadSock))
                {    
                    if(win_num[sd] == -1) {
                        close(sd);
                        FD_CLR(sd, &readfd);
                    }
                    printf("\nServer sends Status: Result = %u\n\r", winner);
                    ResMessage.winNum = winner;
                    if (winner == win_num[sd])
                        ResMessage.status = 1; // submitted bet = winning number
                    else
                        ResMessage.status = 0; // submitted bet != winning number
                    //create result header
                    makeHeader(PROTOCOL_VERSION, BETSERVER_RESULT, sizeof(SendHeader) + sizeof(ResMessage), sd, &SendHeader);
                    if (send(sd, (Betserver_header *) &SendHeader, sizeof(SendHeader), 0) == -1) 
                        perror("\nSend HeaderResult Error!!\n");
                        if (send(sd, (Betserver_result *) &ResMessage, sizeof(ResMessage), 0) == -1) 
                            perror("\nSend Result Error!!\n");

                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                            printf("\n Server Sends Status:BETSERVER_RESULT \n");
                            printf("\n  |Version = %u |  Type = %u |  Length = %d | ClientID = %d |  \n", SendHeader.version, SendHeader.type,SendHeader.length, sd);
                            printf("\n  |Current Bet Status %d | **And Winner is: %u **| \n",ResMessage.status,ResMessage.winNum);
                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                }
            }
            winner = createWinner(BETSERVER_NUM_MAX, BETSERVER_NUM_MIN);
        }
//// done 

        if (FD_ISSET(HeadSock, &readfd))  
        {  
            if ((newSock = accept(HeadSock,(struct sockaddr *)&address, (socklen_t*)&address_len))<0)  
            {  
                perror("Accept Error!!");  
                exit(1);  
            }  

            //  Send and receive commands for new connection
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , newSock , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   

            //add new socket 
            for (i = 0; i < clientsMax; i++)  
            {  
                if( clientSock[i] == 0 )  
                {  
                    clientSock[i] = newSock;  
                    printf("Adding sockets list %d\n" , i);  
                    break;  
                }  
            }  
        }  //check done

    
        for (i = 0; i < clientsMax; i++)  
        {  
            sd = clientSock[i];  

            if (FD_ISSET( sd , &readfd))  
            {  
                // incoming message check
                if ((dataread = read( sd , &RecvHeader, sizeof(RecvHeader))) == 0)  
                {  
                    printf("\nHost interrupted \n" );  
                    close( sd );  //close sock
                    clientSock[i] = 0;  
                }  
                else
                {  
                    switch(RecvHeader.type) 
                    {
                        case BETSERVER_OPEN: 
                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                            printf("\n Server Receives Status:BETSERVER_OPEN\n");
                            printf("  |Version = %u |  Length = %u |   Type = %d |     ClientID = %d |  \n",RecvHeader.version,RecvHeader.length, RecvHeader.type,RecvHeader.clientID);
                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                            AckMessage.minLimit = BETSERVER_NUM_MIN;
                            AckMessage.maxLimit = BETSERVER_NUM_MAX;
                            
                            //header for BETSERVER_ACCEPT 
                            makeHeader(PROTOCOL_VERSION, BETSERVER_ACCEPT, sizeof(SendHeader) + sizeof(AckMessage), sd, &SendHeader);
                            
                            //header sent to the client
                            if (send(sd, (Betserver_header *) &SendHeader, sizeof(SendHeader), 0) == -1)
                                perror("Send Error!!\n");
                            
                            //BETSERVER_ACCEPT is sent to the client
                            if (send(sd, (Betserver_info *) &AckMessage, sizeof(AckMessage), 0) == -1)
                                perror("Send Error!!\n");

                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                            printf("\nServer Sends Status:BETSERVER_ACCEPT ");
                            printf(" | Version = %u  |  Type = %u  |       Length = %d |          ClientID = %d |  \n",SendHeader.version,SendHeader.type,SendHeader.length,SendHeader.clientID);
                            printf(" | Minimum Limit = %u | Maximum Limit  %u  \n", AckMessage.minLimit,AckMessage.maxLimit);
                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                            break;
                                           
                        case BETSERVER_BET: 
                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                            printf("\nServer Receives Status:BETSERVER_BET\n");
                            printf("  |    Version = %u | Length = %u |    Type = %d |     ClientID = %d |  \n",RecvHeader.version,RecvHeader.length, RecvHeader.type,RecvHeader.clientID);
                            printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
                            if ((recv(sd, &BetMessage, sizeof(BetMessage), 0))<= 0) {
                                perror("Receive Error");
                                exit(1);
                            }
                            printf(" |Client made Bet : %u \n", BetMessage.bet);
                            
                            //check for bet limit
                            if (BetMessage.bet <= BETSERVER_NUM_MIN || BetMessage.bet >= BETSERVER_NUM_MAX) {
                                printf("\nBet was out of the limit\n");
                                win_num[sd] = -1;
                                close(sd);
                                FD_CLR(sd, &readfd);
                            } else {
                                printf("\nReceived a valid Bet.. Good Luck \n");
                                win_num[sd] = BetMessage.bet;
                            }
                            break;
                                          
                        default:
                            printf("\nInvalid Type received!\n");
                    }
                }  
            }  
        }  
    }  
    return 0;  
}//main closed
