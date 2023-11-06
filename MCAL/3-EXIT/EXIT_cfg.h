#ifndef EXIT_CFG_H
#define EXIT_CFG_H

#include "EXIT_interface.h"

/*configure int0 trigger source, options : 1_FALLING EDGE*/

#define  EXTI_u8INT0_TRIG_SRC              FALLING_EDGE

/*configure int1 trigger source, options : 1_FALLING EDGE*/

#define  EXTI_u8INT1_TRIG_SRC              FALLING_EDGE

/*configure int2 trigger source, options : 1_FALLING EDGE*/

#define  EXTI_u8INT2_TRIG_SRC              FALLING_EDGE

/*configure int0 initial state, options : 1_enabled*/

#define  EXTI_u8INT0_INIT_STATE             ENABLED

/*configure int1 initial state, options : 1_enabled*/

#define  EXTI_u8INT1_INIT_STATE             ENABLED

/*configure int1 initial state, options : 1_enabled*/

#define  EXTI_u8INT2_INIT_STATE             DISABLED

#endif