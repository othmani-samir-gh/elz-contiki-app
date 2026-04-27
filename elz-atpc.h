/* elz-atpc.h */
#ifndef ELZ_ATPC_H
#define ELZ_ATPC_H
 
#include "net/linkaddr.h"
 
void elz_atpc_init(void);
void elz_atpc_update(const linkaddr_t *neighbor, int16_t rssi);
int8_t elz_atpc_get_power(const linkaddr_t *neighbor);
 
#endif




