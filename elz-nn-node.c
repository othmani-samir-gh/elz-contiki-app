/* elz-nn-node.c — Normal Node (Telemetry) */
#include "contiki.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/packetbuf.h"
#include "sys/log.h"
#include "sys/node-id.h"
#include "random.h"
#include "elz-csma-config.h"
#include "elz-atpc.h"
#include "elz-metrics.h"

#define LOG_MODULE "ELZ-NN"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT_SERVER  5678
#define UDP_PORT_CLIENT  8766   /* different from LCN port */
#define SEND_INTERVAL    (CLOCK_SECOND)  /* 1 second period */

static struct simple_udp_connection udp_conn;
static uint32_t seq_num = 0;

/* Packet structure */
typedef struct {
  uint32_t seq;
  uint32_t timestamp_ms;
  uint8_t  payload[ELZ_PAYLOAD_SIZE_NN - 8];
} elz_nn_packet_t;

PROCESS(elz_nn_process, "ELZ NN");
AUTOSTART_PROCESSES(&elz_nn_process, &elz_metrics_process);

static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data, uint16_t datalen)
{
  /* Echo reply received — compute RTT */
  if(datalen >= 8) {
    uint32_t sent_time;
    memcpy(&sent_time, data + 4, sizeof(sent_time));
    uint32_t now_ms = clock_time() * 1000 / CLOCK_SECOND;
    uint32_t rtt = now_ms - sent_time;
    elz_metrics_log_rtt(rtt);
    elz_metrics_log_recv();

    /* Update ATPC with RSSI from this packet */
    radio_value_t rssi;
    NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi);
    const linkaddr_t *sender_lladdr =
        packetbuf_addr(PACKETBUF_ADDR_SENDER);
    if(sender_lladdr != NULL) {
      elz_atpc_update(sender_lladdr, (int16_t)rssi);
    }
  }
}

PROCESS_THREAD(elz_nn_process, ev, data) {
  static struct etimer send_timer;
  static elz_nn_packet_t pkt;

  PROCESS_BEGIN();

  /* Initialize modules */
  elz_csma_init();
  elz_atpc_init();

  /* Set channel */
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL,
                           IEEE802154_CONF_DEFAULT_CHANNEL);

  /* Register UDP connection */
  simple_udp_register(&udp_conn, UDP_PORT_CLIENT, NULL,
                      UDP_PORT_SERVER, udp_rx_callback);

  /* Wait for RPL DAG to form */
  LOG_INFO("NN node %u started. Waiting for RPL DAG...\n", node_id);
  etimer_set(&send_timer, 15 * CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

  /* Main send loop */
  while(1) {
    /* Add small random jitter to avoid synchronization */
    etimer_set(&send_timer,
               SEND_INTERVAL + (random_rand() % (CLOCK_SECOND / 4)));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

    if(NETSTACK_ROUTING.node_is_reachable()) {
      uip_ipaddr_t dest;
      NETSTACK_ROUTING.get_root_ipaddr(&dest);

      /* Build packet */
      memset(&pkt, 0, sizeof(pkt));
      pkt.seq = seq_num++;
      pkt.timestamp_ms = clock_time() * 1000 / CLOCK_SECOND;

      /* NN nodes: disable ACK request per-packet */
      packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 0);

      /* Set TX power for this destination (ATPC) */
      const linkaddr_t *parent_lladdr =
          packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
      int8_t pwr = (parent_lladdr != NULL) ?
          elz_atpc_get_power(parent_lladdr) : ELZ_ATPC_MAX_POWER;
      NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, pwr);

      /* Send */
      simple_udp_sendto(&udp_conn, &pkt, sizeof(pkt), &dest);
      elz_metrics_log_send();

      LOG_DBG("TX seq=%lu power=%d dBm\n",
              (unsigned long)pkt.seq, pwr);
    }
  }

  PROCESS_END();
}