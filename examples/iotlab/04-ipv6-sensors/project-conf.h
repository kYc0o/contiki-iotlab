#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* radio 802.15.4 conf */
#define RF2XX_CHANNEL 22
#define RF2XX_LEDS_ON
#define RF2XX_TX_POWER  PHY_POWER_3dBm
/*
 * Channels: default=11,   max=26,   min=11.
 * TX power: default=0dBm, max=5dBm, min=m30dBm. (see openlab/net/phy.h)
 */

/* ipv6 stack conf */
#define UIP_CONF_BUFFER_SIZE 1024

/* border-router w/ slip bridge */
#define UIP_FALLBACK_INTERFACE slip_interface
#define SLIP_ARCH_CONF_ENABLE 1

#endif /* PROJECT_CONF_H_ */
