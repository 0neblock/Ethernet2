#include <inttypes.h>
#include "ping.h"
#include "Ethernet2.h"
#include "util.h"
#include "utility/socket.h"
#include "utility/w5500.h"

#define PING_TIMEOUT 500

int16_t Pings::ping_auto(uint8_t *addr){
    uint8_t _sock;
    
    for (int i = 0; i < MAX_SOCK_NUM; i++) {
        uint8_t s = w5500.readSnSR(i);
        if (s == SnSR::CLOSED || s == SnSR::FIN_WAIT || s == SnSR::CLOSE_WAIT) {
            _sock = i;
        break;
        }
    }
    
    if (_sock == MAX_SOCK_NUM)
        return 0;
//    pingTimeout = timeout;
    return ping_request(_sock, addr);
}

int16_t Pings::ping_request(uint8_t s, uint8_t *addr){
    pingMessage = {0};
    pingMessage.Type = PING_REQUEST;
    pingMessage.Code = CODE_ZERO;
    pingMessage.ID = htons(idCount++);
    pingMessage.SeqNum = htons(seqCount++);
    pingMessage.CheckSum = 0;
    pingMessage.CheckSum = htons(checksum((uint8_t*)&pingMessage, sizeof pingMessage));
    
    w5500.writeSnPROTO(s, IPPROTO::ICMP);
    socket(s, SnMR::IPRAW, 3000, 0);
    sentTime = millis();
    sendto(s, (uint8_t*)&pingMessage, sizeof pingMessage, addr, 3000);
    return ping_reply(s, addr);
}

int16_t Pings::ping_reply(uint8_t s, uint8_t *addr){
    pingMessage = {0};
    uint16_t rlen = 32;
    uint16_t port = 0;
    long latency = 0; 
    while(w5500.getRXReceivedSize(s)==0){
        latency = millis() - sentTime;
        if(latency > PING_TIMEOUT){
            close(s);
            return 0;
        }
    }
    rlen = recvfrom(s, (uint8_t*)&pingMessage, w5500.getRXReceivedSize(s), addr, &port);
    close(s);
    Serial.print("Reply in: ");Serial.print(latency);Serial.println("ms");
    
    
    if(pingMessage.Type == 0){ // Echo reply
        return latency;
    } else {
        return -1;
    }
}

uint16_t Pings::checksum(uint8_t *buf, uint16_t len) {
    
  /// From: https://gist.github.com/chrisnc/b0c072ed8e9fb8cac96c
    
  const uint8_t *data = buf;
  uint32_t sum;

  for (sum = 0; len >= 2; data += 2, len -= 2)
  {
    sum += data[0] << 8 | data[1];
  }
  if (len > 0)
  {
    sum += data[0] << 8;
  }
  while (sum > 0xffff)
  {
    sum = (sum >> 16) + (sum & 0xffff);
  }
  sum = ~sum;
//  uint16_t temp = 0;
//    temp = sum>>8;
//    temp |= sum<<8;
//    sum = temp;
  /*
   * From RFC 768:
   * If the computed checksum is zero, it is transmitted as all ones (the
   * equivalent in one's complement arithmetic). An all zero transmitted
   * checksum value means that the transmitter generated no checksum (for
   * debugging or for higher level protocols that don't care).
   */
  return sum ? sum : 0xffff;
}