#ifndef _WIRESHARK_CONFIG_H_
#define _WIRESHARK_CONFIG_H_

#include <config/config.h>

#define VERBOSE_CARRIER_INTEGRATOR (0x0001)
#define VERBOSE_RX_DIAG            (0x0002)
#define VERBOSE_CIR                (0x0004)
#define VERBOSE_NOT_TO_CONSOLE     (0x1000)

typedef int (*on_config_update_t)(void);

int app_conf_init(on_config_update_t on_config_update);
uint16_t app_conf_get_acc_samples();
uint16_t app_conf_get_verbose();



#endif
