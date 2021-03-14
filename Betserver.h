
typedef struct BETSERVER_RESULT
{
    unsigned int winNum; 
    unsigned char status;
}Betserver_result;

typedef struct BETSERVER_HEADER{
    unsigned int version: 3; 
    unsigned int length : 5; 
    unsigned char type; 
    unsigned int clientID;
}Betserver_header; 

typedef struct BETSERVER_BETNUM{
    unsigned int bet;
}Betserver_betnum;

typedef struct BETSERVER_STATUSINFO{
    unsigned int minLimit; 
    unsigned int maxLimit;
}Betserver_info;

#define BETSERVER_NUM_CLIENTS 40
#define BETSERVER_NUM_MIN 0xe0ffff00
#define BETSERVER_NUM_MAX 0xe0ffffaa
#define PROTOCOL_VERSION 0x1

#define BETSERVER_OPEN 0x1
#define BETSERVER_ACCEPT 0x2
#define BETSERVER_BET 0x3
#define BETSERVER_RESULT 0x4

int unsigned createBet( unsigned int maxLimit,  unsigned int minLimit );
unsigned int createWinner(unsigned int maxLimit, unsigned int minLimit);
void makeHeader(unsigned int ver,  unsigned int type, unsigned char len, unsigned int clientId,Betserver_header *sendMessage);