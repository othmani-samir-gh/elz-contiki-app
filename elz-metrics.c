/* elz-metrics.c */
#include "sys/node-id.h"
#include "contiki.h"
#include "sys/energest.h"
#include "net/link-stats.h"
#include "sys/log.h"
 
#define LOG_MODULE "ELZ-METRICS"
#define LOG_LEVEL LOG_LEVEL_INFO
 
static struct etimer metrics_timer;
 
/* Counters */
static uint32_t packets_sent = 0;
static uint32_t packets_recv = 0;
static uint32_t packets_lost = 0;
static uint32_t rtt_sum_ms   = 0;
static uint32_t rtt_count    = 0;
static uint32_t rtt_max_ms   = 0;
 
/* Application API */
void elz_metrics_log_send(void) { packets_sent++; }
void elz_metrics_log_recv(void) { packets_recv++; }
void elz_metrics_log_loss(void) { packets_lost++; }
void elz_metrics_log_rtt(uint32_t rtt_ms) {
  rtt_sum_ms += rtt_ms;
  rtt_count++;
  if(rtt_ms > rtt_max_ms) rtt_max_ms = rtt_ms;
}
 
PROCESS(elz_metrics_process, "ELZ Metrics");
 
PROCESS_THREAD(elz_metrics_process, ev, data) {
  PROCESS_BEGIN();
  energest_init();
  
  while(1) {
    etimer_set(&metrics_timer, ELZ_METRICS_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&metrics_timer));
    
    /* Energy metrics from Energest */
    energest_flush();
    uint64_t cpu  = energest_type_time(ENERGEST_TYPE_CPU);
    uint64_t lpm  = energest_type_time(ENERGEST_TYPE_LPM);
    uint64_t tx   = energest_type_time(ENERGEST_TYPE_TRANSMIT);
    uint64_t rx   = energest_type_time(ENERGEST_TYPE_LISTEN);
    uint64_t total = cpu + lpm;
    
    uint32_t duty_cycle_pct = 0;
    if(total > 0) {
      duty_cycle_pct = (uint32_t)((tx + rx) * 10000 / total);
    }
    
    /* PDR calculation */
    uint32_t pdr = 0;
    if(packets_sent > 0) {
      pdr = ((packets_sent - packets_lost) * 10000)
            / packets_sent;
    }
    
    /* RTT statistics */
    uint32_t rtt_avg = 0;
    if(rtt_count > 0) rtt_avg = rtt_sum_ms / rtt_count;
    
    /* Structured log output (parsed by scripts/parse-logs.py)
     * Format: ELZ;node_id;type;timestamp;field=value;... */
    LOG_INFO("ELZ;%u;METRICS;%lu;"
             "tx=%lu;rx_ok=%lu;lost=%lu;"
             "pdr=%lu.%02lu;"
             "rtt_avg=%lu;rtt_max=%lu;"
             "duty=%lu.%02lu;"
             "radio_tx_t=%lu;radio_rx_t=%lu\n",
             (unsigned)node_id,
             (unsigned long)clock_seconds(),
             (unsigned long)packets_sent,
             (unsigned long)packets_recv,
             (unsigned long)packets_lost,
             (unsigned long)(pdr / 100),
             (unsigned long)(pdr % 100),
             (unsigned long)rtt_avg,
             (unsigned long)rtt_max_ms,
             (unsigned long)(duty_cycle_pct / 100),
             (unsigned long)(duty_cycle_pct % 100),
             (unsigned long)tx,
             (unsigned long)rx);
    
    /* Reset for next window */
    packets_sent = packets_recv = packets_lost = 0;
    rtt_sum_ms = rtt_count = rtt_max_ms = 0;
  }
  
  PROCESS_END();
}

