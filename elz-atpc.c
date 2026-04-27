/* elz-atpc.c */
#include "contiki.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "sys/log.h"
#include "elz-atpc.h"
 
#define LOG_MODULE "ELZ-ATPC"
#define LOG_LEVEL LOG_LEVEL_INFO
 
#define MAX_NEIGHBORS  20
#define EWMA_ALPHA     51   /* 0.2 * 256 (fixed-point) */
 
typedef struct {
  linkaddr_t addr;
  int16_t rssi_avg;     /* EWMA in fixed-point (x256) */
  int8_t  tx_power;     /* current TX power for this neighbor */
  uint8_t used;
} neighbor_power_t;
 
static neighbor_power_t neighbors[MAX_NEIGHBORS];
 
void elz_atpc_init(void) {
  memset(neighbors, 0, sizeof(neighbors));
  /* Set initial TX power */
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, ELZ_ATPC_MAX_POWER);
  LOG_INFO("ATPC initialized, initial power=%d dBm\n",
           ELZ_ATPC_MAX_POWER);
}
 
static neighbor_power_t* find_or_add(const linkaddr_t *addr) {
  int free_slot = -1;
  for(int i = 0; i < MAX_NEIGHBORS; i++) {
    if(neighbors[i].used &&
       linkaddr_cmp(&neighbors[i].addr, addr)) {
      return &neighbors[i];
    }
    if(!neighbors[i].used && free_slot < 0) free_slot = i;
  }
  if(free_slot >= 0) {
    neighbors[free_slot].used = 1;
    linkaddr_copy(&neighbors[free_slot].addr, addr);
    neighbors[free_slot].rssi_avg = ELZ_ATPC_TARGET_RSSI * 256;
    neighbors[free_slot].tx_power = ELZ_ATPC_MAX_POWER;
    return &neighbors[free_slot];
  }
  return NULL;
}
 
void elz_atpc_update(const linkaddr_t *neighbor, int16_t rssi) {
  neighbor_power_t *n = find_or_add(neighbor);
  if(n == NULL) return;
  
  /* EWMA update (fixed-point: value * 256) */
  n->rssi_avg = ((256 - EWMA_ALPHA) * n->rssi_avg
                + EWMA_ALPHA * (rssi * 256)) / 256;
  
  int16_t rssi_smooth = n->rssi_avg / 256;
  int16_t link_margin = rssi_smooth - (-100); /* sensitivity */
  int16_t target_margin = ELZ_ATPC_TARGET_RSSI - (-100);
  int16_t error = link_margin - target_margin;
  
  /* Hysteresis: only adjust if outside deadband */
  if(error > ELZ_ATPC_HYSTERESIS / 2) {
    /* Over-powered: reduce */
    n->tx_power -= ELZ_ATPC_STEP;
    int8_t floor = NODE_IS_LCN ?
                   ELZ_ATPC_LCN_FLOOR : ELZ_ATPC_MIN_POWER;
    if(n->tx_power < floor) n->tx_power = floor;
    LOG_INFO("ATPC: margin=%d dB, power -> %d dBm\n",
             link_margin, n->tx_power);
  } else if(error < -(ELZ_ATPC_HYSTERESIS / 2)) {
    /* Under-powered: increase */
    n->tx_power += ELZ_ATPC_STEP;
    if(n->tx_power > ELZ_ATPC_MAX_POWER)
      n->tx_power = ELZ_ATPC_MAX_POWER;
    LOG_INFO("ATPC: margin=%d dB, power -> %d dBm\n",
             link_margin, n->tx_power);
  }
}
 
int8_t elz_atpc_get_power(const linkaddr_t *neighbor) {
  neighbor_power_t *n = find_or_add(neighbor);
  return n ? n->tx_power : ELZ_ATPC_MAX_POWER;
}

