#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "Error_Type.h"

#include "ADC_interface.h"
#include "ADC_prv.h"
#include "ADC_cfg.h"
#include "ADC_reg.h"

static uint16* ADC_pu16DigResult = NULL;

static void (*ADC_pvNotificationFunc)(void)=NULL;

void ADC_voidInit(void)
{
	/*Reference selection: AVCC pin*/
	SET_BIT(ADMUX, ADMUX_REFS0);
	CLR_BIT(ADMUX, ADMUX_REFS1);

#if ADC_u8RESOLUTION == EIGHT_BITS	
	SET_BIT(ADMUX, ADMUX_ADLAR);
#elif ADC_u8RESOLUTION == TEN_BITS
    CLR_BIT(ADMUX, ADMUX_ADLAR);
#else
#error wrong ADC_u8RESOLUTION configuration value
#endif

	/*check on prescaler configuration setting*/
	ADCSRA &= PREASCALER_MASK;             /*clear the prescaler bits*/
	ADCSRA |= ADC_u8PRESCALER;
	
	/*ADC Enable*/
	SET_BIT(ADCSRA,ADCSRA_ADEN);
	
}
uint8 ADC_u16StartConversionSynch(ADC_Channel_t copy_Channel, uint16* copy_pu16DigResult)
{
	uint8 Local_u8ErrorState = OK;
	if(copy_pu16DigResult!=NULL)
	{
		    uint32 Local_u32Counter = 0u;
			
			/*set the required channel*/
			ADCSRA &= CHANNEL_SELECTION_MASK;           /*clear the channel selection bits*/
			ADCSRA |= copy_Channel;
			
			/*start the conversion*/
			SET_BIT(ADCSRA,ADCSRA_ADSC);
			
			/*wait (block) until conversion is completed or the tiomeout has not yet*/
			while(((GET_BIT(ADCSRA,ADCSRA_ADIF))==0) && (Local_u32Counter < ADC_u32TIMEOUT_COUNT));
			{
				Local_u32Counter++;
			}
			if(Local_u32Counter == ADC_u32TIMEOUT_COUNT)
			{
				/*loop is broken because timeout has passed*/
				Local_u8ErrorState = TIMEOUT_ERR;
			}
			else      /*loop is broken because flag is raised*/
			{
				/*clear the conversion complete flag*/
				SET_BIT(ADCSRA,ADCSRA_ADIF);
				
				#if ADC_u8RESOLUTION == EIGHT_BITS
				*copy_pu16DigResult= (uint16)ADCH
				#elif ADC_u8RESOLUTION == TEN_BITS
				*copy_pu16DigResult= ADC;
				
				#endif
			}
			
			
	}
	else
	{
		Local_u8ErrorState=NULL_PTR;
	}

    return Local_u8ErrorState;
}

uint8 ADC_u16StartConversionASynch(ADC_Channel_t copy_Channel, uint16* copy_pu16DigResult, void (*copy_pvNotificatioFunc)(void))
{
	uint8 Local_u8ErrorState = OK;
	if(copy_pu16DigResult != NULL && copy_pvNotificatioFunc != NULL)
	{
		/*convert the digital result and notification function local variable into global*/
		ADC_pu16DigResult = copy_pu16DigResult;
		ADC_pvNotificationFunc = copy_pvNotificatioFunc;
		
		/*set the required channel*/
		ADCSRA &= CHANNEL_SELECTION_MASK;           /*clear the channel selection bits*/
		ADCSRA |= copy_Channel;
		
		/*start the conversion*/
		SET_BIT(ADCSRA,ADCSRA_ADSC);
		
		/*Enable ADC conversion complete interrupt*/
		SET_BIT(ADCSRA,ADCSRA_ADIE);
	}
	else
	{
		Local_u8ErrorState = NULL_PTR;
	}
	    return Local_u8ErrorState;

}

/*ADC conversion complete ISR*/
void __vector_16 (void)         __attribute__((signal));
void __vector_16 (void) 
{
	#if ADC_u8RESOLUTION == EIGHT_BITS
    *ADC_pu16DigResult = (uint16)ADCH;
	#elif ADC_u8RESOLUTION == TEN_BITS
	*ADC_pu16DigResult = ADC;
	#endif
	
	
	/*ADC interrupt disable */
	CLR_BIT(ADCSRA,ADCSRA_ADIE);
	
	
	if(ADC_pvNotificationFunc != NULL)
	{
		ADC_pvNotificationFunc();
	}
	else
	{
		/*DO Nothing*/
	}
}