#ifndef PINGs_h
#define PINGs_h

#define BUF_LEN 32
#define PING_REQUEST 8
#define PING_REPLY 0
#define CODE_ZERO 0



typedef struct pingmsg {
     uint8_t Type; // 0 - Ping Reply, 8 - Ping Request
     uint8_t Code; // Always 0
     int16_t CheckSum; // Check sum
     int16_t ID; // Identification
     int16_t SeqNum; // Sequence Number
    // Ping Data : 1452 = IP RAW MTU - sizeof(Type + Code + CheckSum + ID + SeqNum)
     int8_t Data[BUF_LEN];
} PINGMSGR;


class Pings {
    public:
        Pings(){
            
        }
    
        PINGMSGR pingMessage = {0};
    
        int16_t ping_auto(uint8_t *addr);
    
        int16_t ping_request(uint8_t s, uint8_t *addr);
    
        int16_t ping_reply(uint8_t s, uint8_t *addr);
    
        uint16_t checksum(uint8_t *buf, uint16_t len);
    
        int idCount = 128;
        int seqCount = 0;
        long sentTime = 0;
        
};

#endif