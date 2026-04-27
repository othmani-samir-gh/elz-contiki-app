/* elz-metrics.h */
#ifndef ELZ_METRICS_H
#define ELZ_METRICS_H

#include "contiki.h"

/* Declare the process so other files can reference it
 * in AUTOSTART_PROCESSES() */
PROCESS_NAME(elz_metrics_process);

/* Application API */
void elz_metrics_log_send(void);
void elz_metrics_log_recv(void);
void elz_metrics_log_loss(void);
void elz_metrics_log_rtt(uint32_t rtt_ms);

#endif /* ELZ_METRICS_H */
