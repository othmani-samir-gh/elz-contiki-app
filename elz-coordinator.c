/* coordinator Sink / Border Router */
#include "contiki.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "sys/log.h"
 
#define LOG_MODULE "ELZ-COORD"
#define LOG_LEVEL LOG_LEVEL_INFO
 
#define UDP_PORT_SERVER  5678
 
static struct simple_udp_connection udp_conn;
 
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data, uint16_t datalen)
{
  uint32_t seq, timestamp;
  if(datalen >= 8) {
    memcpy(&seq, data, sizeof(seq));
    memcpy(&timestamp, data + 4, sizeof(timestamp));
    
    uint32_t now_ms = clock_time() * 1000 / CLOCK_SECOND;
    uint32_t oneway = now_ms - timestamp;
    
    LOG_INFO("RX from ");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_(" seq=%lu delay=%lu ms len=%u\n",
              (unsigned long)seq,
              (unsigned long)oneway,
              datalen);
    
    /* Send echo-reply for RTT measurement */
    simple_udp_sendto(&udp_conn, data, 8, sender_addr);
  }
}
 
PROCESS(elz_coord_process, "ELZ Coordinator");
AUTOSTART_PROCESSES(&elz_coord_process);
 
PROCESS_THREAD(elz_coord_process, ev, data) {
  PROCESS_BEGIN();
  
  NETSTACK_ROUTING.root_start();
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL,
                           IEEE802154_CONF_DEFAULT_CHANNEL);
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER,
                           ELZ_ATPC_MAX_POWER);
  
  simple_udp_register(&udp_conn, UDP_PORT_SERVER, NULL,
                      0, udp_rx_callback);
  
  LOG_INFO("Coordinator started on CH %d\n",
           IEEE802154_CONF_DEFAULT_CHANNEL);
  
  PROCESS_YIELD();
  PROCESS_END();
}

