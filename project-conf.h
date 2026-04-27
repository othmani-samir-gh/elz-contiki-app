/* project-conf.h — ELZ v1.0 Configuration */
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
 
/*-------------------------------------------------------
 * Step 0 — Node Classification
 * Set via Makefile: CFLAGS += -DNODE_IS_LCN=1
 *------------------------------------------------------*/
#ifndef NODE_IS_LCN
#define NODE_IS_LCN  0    /* default: Normal Node */
#endif
 
/*-------------------------------------------------------
 * Step 3 — Differentiated CSMA/CA Parameters
 * IEEE 802.15.4-2020 Section 6.2.5.1
 *------------------------------------------------------*/
#if NODE_IS_LCN
  #define CSMA_CONF_MIN_BE            1
  #define CSMA_CONF_MAX_BE            3
  #define CSMA_CONF_MAX_BACKOFF       3
  #define CSMA_CONF_MAX_FRAME_RETRIES 2
#else
  #define CSMA_CONF_MIN_BE            3
  #define CSMA_CONF_MAX_BE            5
  #define CSMA_CONF_MAX_BACKOFF       4
  #define CSMA_CONF_MAX_FRAME_RETRIES 3
#endif
 
/*-------------------------------------------------------
 * Step 4 — Frame Size and ACK
 *------------------------------------------------------*/
#define SICSLOWPAN_CONF_COMPRESSION  SICSLOWPAN_COMPRESSION_IPHC

/* Optimal payload target (adjust per BER, see Step 4 formula) */
#define ELZ_PAYLOAD_SIZE_LCN   40
#define ELZ_PAYLOAD_SIZE_NN    80

/* ACK control is handled via platform patch (see Fix 2)
 * Do NOT define CSMA_CONF_SEND_SOFT_ACK here —
 * the COOJA platform hard-defines it in contiki-conf.h
 * AFTER including project-conf.h, causing a redefinition error.
 *
 * Instead, we use a custom flag that our patched
 * platform file will respect: */
#if NODE_IS_LCN
  #define ELZ_ACK_ENABLED  1
#else
  #define ELZ_ACK_ENABLED  0
#endif



 
/*-------------------------------------------------------
 * Step 5 — RPL Routing Configuration
 *------------------------------------------------------*/
/*define RPL_CONF_SUPPORTED_OFS      {&rpl_of_etx}*/
#define RPL_CONF_SUPPORTED_OFS      {&rpl_mrhof}
#define RPL_CONF_OF_OCP             RPL_OCP_MRHOF
/* Faster local repair: reduce DIO interval */
#define RPL_CONF_DIO_INTERVAL_MIN   12  /* 2^12 ms = ~4 sec */
#define RPL_CONF_DIO_INTERVAL_DOUBLINGS  4
/* Proactive route repair */
#define RPL_CONF_DEFAULT_LIFETIME_UNIT  60
#define RPL_CONF_DEFAULT_LIFETIME       10
 
/*-------------------------------------------------------
 * Step 6 — RF Channel and TX Power
 *------------------------------------------------------*/
#define IEEE802154_CONF_DEFAULT_CHANNEL  26  /* least Wi-Fi interference */
 
/* ATPC initial power — will be adjusted at runtime */
#define ELZ_ATPC_TARGET_RSSI    (-90)  /* dBm: sensitivity + 10 dB margin */
#define ELZ_ATPC_HYSTERESIS       8    /* dB: deadband to prevent oscillation */
#define ELZ_ATPC_STEP             3    /* dB: adjustment step size */
#define ELZ_ATPC_MIN_POWER     (-10)   /* dBm floor */
#define ELZ_ATPC_MAX_POWER        5    /* dBm ceiling (regional) */
#define ELZ_ATPC_LCN_FLOOR        0    /* dBm: never go below for LCN */
 
/*-------------------------------------------------------
 * Step 7 — Duty Cycle (ContikiMAC RDC proxy)
 * NOTE: This is NOT beacon-enabled duty cycling.
 * It approximates the energy savings for simulation.
 *------------------------------------------------------*/
#define NETSTACK_CONF_MAC    csma_driver
 
/*-------------------------------------------------------
 * Step 9 — Queue Configuration
 *------------------------------------------------------*/
#define QUEUEBUF_CONF_NUM   16
#define ELZ_QUEUE_LCN_DEPTH  4
#define ELZ_QUEUE_NN_DEPTH  12
 
/*-------------------------------------------------------
 * Step 10 — Metrics Collection
 *------------------------------------------------------*/
#define ENERGEST_CONF_ON     1
#define ELZ_METRICS_INTERVAL (60 * CLOCK_SECOND)  /* 60 sec */
 
/* Logging */
#define LOG_CONF_LEVEL_MAC   LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_RPL   LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_APP   LOG_LEVEL_DBG
 
#endif /* PROJECT_CONF_H_ */

