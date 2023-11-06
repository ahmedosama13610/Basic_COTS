#ifndef EXTI_REG_H
#define EXTI_REG_H



#define MCUCR             *((volatile uint8*)0x55)         /*mcu control register*/
#define MCUCR_ISC00        0u                              /*int.sense ctrl 0 bit 0*/
#define MCUCR_ISC01        1u                               /*int.sense ctrl 0 bit 1*/
#define MCUCR_ISC10        2u                               /*int.sense ctrl 1 bit 0*/ 
#define MCUCR_ISC11        3u                                /*int.sense ctrl 1 bit 1*/

#define MCUCSR             *((volatile uint8*)0x54)          /*mcu control and status register*/
#define MCUCSR_ISC2        6u                                /*int .sense ctrl t*/


#define GICR               *((volatile uint8*)0x5B)          /*general interrupt control register*/
#define GICR_INT1          7u
#define GICR_INT0          6u
#define GICR_INT2          5u


#endif