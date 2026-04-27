/* elz-csma-config.h */
#ifndef ELZ_CSMA_CONFIG_H
#define ELZ_CSMA_CONFIG_H
 
#define ELZ_CSMA_MEASURE_WINDOW    30   /* seconds */
#define ELZ_CSMA_PLR_HIGH          5    /* percent: trigger increase */
#define ELZ_CSMA_PLR_LOW           1    /* percent: allow decrease */
#define ELZ_CSMA_HOLD_WINDOWS      3    /* consecutive windows before change */
 
void elz_csma_init(void);
void elz_csma_report_tx(int success);
 
#endif

