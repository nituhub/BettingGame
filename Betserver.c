#include "Betserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <netinet/in.h>

unsigned int createBet( unsigned int maxLimit, unsigned int minLimit){
    static int num = 0; 
    if(num == 0){
        srand(time(NULL));
        num =1;
    }
    return(rand() % (maxLimit - minLimit +1)+ minLimit);
}

unsigned int createWinner(unsigned int maxLimit, unsigned int minLimit){
    static int num =0;
    if(num == 0){
        srand(time(NULL));
        num = 1;
    }
    return (rand() % (maxLimit - minLimit +1)+minLimit);
}

void makeHeader(unsigned int ver,  unsigned int type, unsigned char len,unsigned int clientId, Betserver_header *sendMessage){
    sendMessage ->version = ver;
    sendMessage ->type = type;
    sendMessage ->length = len;  
    sendMessage ->clientID = clientId;
}