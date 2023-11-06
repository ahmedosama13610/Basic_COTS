#define  F_CPU 16000000UL
#include "STD_TYPES.h"
#include <util/delay.h>
#include "BIT_MATH.h"
#include "Error_Type.h"

#include "DIO_interface.h"

#include "CLCD_interface.h"
#include "CLCD_prv.h"
#include "CLCD_cfg.h"

#if CLCD_u8CONECCTION_MODE == FOUR_BIT
static void voidSetHalfDataPort(uint8 copy_u8FOURBITDATA)
{
	DIO_u8SetPinValue(CLCD_u8DATA_PORT,CLCD_u8D4_PIN,GET_BIT(copy_u8FOURBITDATA,0));
	DIO_u8SetPinValue(CLCD_u8DATA_PORT,CLCD_u8D5_PIN,GET_BIT(copy_u8FOURBITDATA,1));
	DIO_u8SetPinValue(CLCD_u8DATA_PORT,CLCD_u8D6_PIN,GET_BIT(copy_u8FOURBITDATA,2));
	DIO_u8SetPinValue(CLCD_u8DATA_PORT,CLCD_u8D7_PIN,GET_BIT(copy_u8FOURBITDATA,3));
 
}
#endif 
static void voidSendEnablePulse(void)
{
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT,CLCD_u8E_PIN,DIO_PIN_HIGH);
	_delay_ms(2);
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT,CLCD_u8E_PIN,DIO_PIN_LOW);

} 


void CLCD_voidSendCommand(uint8 copy_u8Command)
{
	/*1-set RS pin to low for command*/
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT,CLCD_u8RS_PIN,DIO_PIN_LOW);
	
	/*2-set RW pin to low for write*/
#if CLCD_u8READ_OPERATION_ENABLE == ENABLED

	DIO_u8SetPinValue(CLCD_u8CTRL_PORT,CLCD_u8RW_PIN,DIO_PIN_LOW);
#endif

	/*3-send the command */
#if CLCD_u8CONECCTION_MODE == EIGHT_BIT
    DIO_u8SetPortValue(CLCD_u8DATA_PORT,copy_u8Command);

	/*-send enable pulse*/
	voidSendEnablePulse();
#elif CLCD_u8CONECCTION_MODE == FOUR_BIT
     /*send the higher 4 bits of the command */
    voidSetHalfDataPort(copy_u8Command>>4u);
  
	/*send enable pulse*/
	voidSendEnablePulse();
	    /*send the lower 4 bits of the command */

    voidSetHalfDataPort(copy_u8Command);

	/*-send enable pulse*/
	
    voidSendEnablePulse();

#endif
	
}
void CLCD_voidSendData(uint8 copy_u8Data)
{
	/*1-set RS pin to high for data*/
	DIO_u8SetPinValue(CLCD_u8CTRL_PORT,CLCD_u8RS_PIN,DIO_PIN_HIGH);
	
	/*2-set RW pin to low for write*/
#if CLCD_u8READ_OPERATION_ENABLE == ENABLED

	DIO_u8SetPinValue(CLCD_u8CTRL_PORT,CLCD_u8RW_PIN,DIO_PIN_LOW);
#endif
	
	/*3-send the data */
#if CLCD_u8CONECCTION_MODE == EIGHT_BIT
	DIO_u8SetPortValue(CLCD_u8DATA_PORT,copy_u8Data);


	
	
	/*4-send enable pulse*/
	voidSendEnablePulse();
#elif CLCD_u8CONECCTION_MODE == FOUR_BIT
	/*send the higher 4 bits of the command */
	voidSetHalfDataPort(copy_u8Data>>4u);
	
	/*send enable pulse*/
	voidSendEnablePulse();
	/*send the lower 4 bits of the command */

	voidSetHalfDataPort(copy_u8Data);

	/*-send enable pulse*/
	
	voidSendEnablePulse();

#endif
	
}
void CLCD_voidInit(void)
{
	/*1-wait for more than 30ms after power on*/
	_delay_ms(40);
	
	/*2-function set command:2lines, font size 5x7 */
#if CLCD_u8CONECCTION_MODE == EIGHT_BIT	
	CLCD_voidSendCommand(0b00111000);
	
#elif CLCD_u8CONECCTION_MODE == FOUR_BIT
	voidSetHalfDataPort(0b0010);
	voidSendEnablePulse();
	voidSetHalfDataPort(0b0010);
	voidSendEnablePulse();
    voidSetHalfDataPort(0b1000);
    voidSendEnablePulse();

#endif
	
	/*3-display on/off control:display on ,cursor off ,blink cursor off*/
	CLCD_voidSendCommand(0b00001100);
	
	/*4-Display Clear*/
	CLCD_voidSendCommand(1);
	
}
void CLCD_voidWriteNumber(sint32 copy_s32Number)
{
	char Local_chNumber[10];
	uint8 Local_u8RightDigit,Local_u8Counter1=0;
	sint8 Local_s8Counter2;
	if(copy_s32Number==0)
	{
		CLCD_voidSendData('0');
	}
	else if(copy_s32Number<0)
	{
		CLCD_voidSendData('-');
		copy_s32Number *=-1;
	}
	while(copy_s32Number != 0)
	{
		Local_u8RightDigit=copy_s32Number%10;   /*get the right most digit*/
		copy_s32Number /=10;                    /*get the rid of right most digit*/
		Local_chNumber[Local_u8Counter1]=Local_u8RightDigit + '0';
		Local_u8Counter1++;
	}
	for(Local_s8Counter2=(sint8)Local_u8Counter1-1;Local_s8Counter2>=0;Local_s8Counter2--)
	{
		CLCD_voidSendData(Local_chNumber[(uint8)Local_s8Counter2]);
	}
	
	
}
void CLCD_voidGoToXY(uint8 copy_u8XPos,uint8 copy_YPos)
{
	uint8 Local_u8DDRamAdd = (copy_YPos * 0x40u)+ copy_u8XPos;
	SET_BIT(Local_u8DDRamAdd,7u);
	/*send the setting DDRAM address command*/
	CLCD_voidSendCommand(Local_u8DDRamAdd);
}
uint8 CLCD_u8WriteSpecialCharacter(uint8 copy_u8PatternNumber,uint8 copy_pu8PatternArr[],uint8 copy_u8XPos,uint8 copy_u8YPos)
{
	uint8 Local_u8ErrorState = OK;
	if (copy_pu8PatternArr!=NULL)
	{
		uint8 Local_u8CGRAMAdd = copy_u8PatternNumber*8u;
		uint8 Local_u8Iterator;
		/*set bit6 clear bit7 for setting CGRAM address command*/
		SET_BIT(Local_u8CGRAMAdd,6u);
		/*set CGRAM address*/
		CLCD_voidSendCommand(Local_u8CGRAMAdd);
		/*write the pattern inside CGRAM*/
		for(Local_u8Iterator=0u;Local_u8Iterator<8u;Local_u8Iterator++)
		{
			CLCD_voidSendData(copy_pu8PatternArr[Local_u8Iterator]);
		}
		/*set DDRAM address*/
		CLCD_voidGoToXY(copy_u8XPos,copy_u8YPos);
		/*write the corresponding pattern number to display from cgram*/
		CLCD_voidSendData(copy_u8PatternNumber);
		
	}
	else
	{
		uint8 Local_u8ErrorState=NULL_PTR;
	}
	return Local_u8ErrorState;
}
void CLCD_voidWriteString(char *str)
{
		while(*str!='\0')
		{
			CLCD_voidSendData(*str);
			str++;
		}

}
void CLCD_voidClearLCD(void)
{
	CLCD_voidSendCommand(0b0001);
}
