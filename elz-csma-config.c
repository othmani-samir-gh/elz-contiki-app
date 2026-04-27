/* elz-csma-config.c */
#include "contiki.h"
#include "net/mac/csma/csma.h"
#include "sys/log.h"
#include "elz-csma-config.h"
 
#define LOG_MODULE "ELZ-CSMA"
#define LOG_LEVEL LOG_LEVEL_INFO
 
static uint32_t tx_total = 0;
static uint32_t tx_fail  = 0;
static int hold_counter  = 0;
static int last_direction = 0;  /* +1 or -1 */
static struct etimer measure_timer;
 
/* Current adaptive parameters */
static uint8_t current_min_be;
static uint8_t current_max_be;
 
PROCESS(elz_csma_process, "ELZ CSMA Adaptive");
 
void elz_csma_init(void) {
  #if NODE_IS_LCN
    current_min_be = 1;
    current_max_be = 3;
  #else
    current_min_be = 3;
    current_max_be = 5;
  #endif
  process_start(&elz_csma_process, NULL);
}
 
void elz_csma_report_tx(int success) {
  tx_total++;
  if(!success) tx_fail++;
}
 
PROCESS_THREAD(elz_csma_process, ev, data) {
  PROCESS_BEGIN();
  
  while(1) {
    etimer_set(&measure_timer,
               ELZ_CSMA_MEASURE_WINDOW * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&measure_timer));
    
    if(tx_total > 0) {
      uint32_t plr = (tx_fail * 100) / tx_total;
      
      if(plr > ELZ_CSMA_PLR_HIGH) {
        if(last_direction == 1) hold_counter++;
        else { hold_counter = 1; last_direction = 1; }
        
        if(hold_counter >= ELZ_CSMA_HOLD_WINDOWS) {
          if(current_min_be < current_max_be - 1) {
            current_min_be++;
            LOG_INFO("PLR=%lu%% -> macMinBE++ = %u\n",
                     (unsigned long)plr, current_min_be);
          }
          hold_counter = 0;
        }
      } else if(plr < ELZ_CSMA_PLR_LOW) {
        if(last_direction == -1) hold_counter++;
        else { hold_counter = 1; last_direction = -1; }
        
        if(hold_counter >= ELZ_CSMA_HOLD_WINDOWS) {
          if(current_min_be > 1) {
            current_min_be--;
            LOG_INFO("PLR=%lu%% -> macMinBE-- = %u\n",
                     (unsigned long)plr, current_min_be);
          }
          hold_counter = 0;
        }
      } /* else: in hysteresis band, hold */
      
      LOG_INFO("Window: tx=%lu fail=%lu PLR=%lu%% BE=%u\n",
               (unsigned long)tx_total, (unsigned long)tx_fail,
               (unsigned long)plr, current_min_be);
    }
    
    /* Reset counters for next window */
    tx_total = 0;
    tx_fail  = 0;
  }
  
  PROCESS_END();
}

