#ifndef PROJECT_CONF_H_
//PROJECT_CONF will be defined by the include

// #define SLIP_ARCH_CONF_ENABLE 1
// #define UIP_FALLBACK_INTERFACE slip_interface
////#ifndef UIP_FALLBACK_INTERFACE
////#define UIP_FALLBACK_INTERFACE rpl_interface
////#endif

#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    1500
#endif

//#ifndef QUEUEBUF_CONF_NUM
//#define QUEUEBUF_CONF_NUM          4
//#endif
//#ifndef UIP_CONF_RECEIVE_WINDOW
//#define UIP_CONF_RECEIVE_WINDOW  60
//#endif

#include "../project-conf.h"

#endif /* PROJECT_CONF_H_ */
