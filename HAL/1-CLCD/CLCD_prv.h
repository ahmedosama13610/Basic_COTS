#ifndef CLCD_PRV_H
#define CLCD_PRV_H

#define FOUR_BIT      1u
#define EIGHT_BIT     2u

#define ENABLED       1u
#define DISABLED      2u

#if CLCD_u8CONECCTION_MODE == FOUR_BIT

static void voidSetHalfDataPort(uint8 copy_u8FOURBITDATA);

#endif
//static void voidSendEnablePulse(void);

       
#endif