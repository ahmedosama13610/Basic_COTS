#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "Error_Type.h"

#include "ADC_interface.h"
#include "ADC_prv.h"
#include "ADC_cfg.h"
#include "ADC_reg.h"

static uint16* ADC_pu16DigResult = NULL;

static void (*ADC_pvNotificationFunc)(void)=NULL;

static uint8 ADC_u8BusyFlag = IDLE;

static ADC_Chain_t *ADC_pChainData = NULL;

static uint8 ADC_u8ConversionIndex=0u;

static uint8 ADC_u8ISRSource;

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
	ADMUX &= PREASCALER_MASK;             /*clear the prescaler bits*/
	ADMUX |= ADC_u8PRESCALER;
	
	/*ADC Enable*/
	SET_BIT(ADCSRA,ADCSRA_ADEN);
	
}
uint8 ADC_u16StartSingleConversionSynch(ADC_Channel_t copy_Channel, uint16* copy_pu16DigResult)
{
	uint8 Local_u8ErrorState = OK;
	if(copy_pu16DigResult!=NULL)
	{
		    if(ADC_u8BusyFlag!=NULL)
			{
				
		    uint32 Local_u32Counter = 0u;
			
			/*ADC is now busy*/
			ADC_u8BusyFlag = BUSY;
			
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
				*copy_pu16DigResult= (uint16)ADCH;
				#elif ADC_u8RESOLUTION == TEN_BITS
				*copy_pu16DigResult= ADC;
				
				#endif
				
				/*Task is done ,ADC is now idle*/
				ADC_u8BusyFlag = IDLE;
			}
		
	}
	else
	{
		Local_u8ErrorState =BUSY_ERR;
	}
	}
	else
	{
		Local_u8ErrorState=NULL_PTR;
	}

    return Local_u8ErrorState;
}

uint8 ADC_u16StartDSingleConversionASynch(ADC_Channel_t copy_Channel, uint16* copy_pu16DigResult, void (*copy_pvNotificatioFunc)(void))
{
	uint8 Local_u8ErrorState = OK;
	if((copy_pu16DigResult != NULL) && (copy_pvNotificatioFunc != NULL))
	{
		if(ADC_u8BusyFlag == IDLE)
		{
			/*ADC is now busy*/
			ADC_u8BusyFlag = BUSY;
			
			/*ISR source is  single conversions asynchronous functions */
		    ADC_u8ISRSource = SINGLE_CONV_ASYNCH;
	 	
		
		    /*convert the digital result and notification function local variable into global*/
		    ADC_pu16DigResult = copy_pu16DigResult;
		    ADC_pvNotificationFunc = copy_pvNotificatioFunc;
		    
		    /*set the required channel*/
		    ADMUX &= CHANNEL_SELECTION_MASK;           /*clear the channel selection bits*/
		    ADMUX |= copy_Channel;
		    
		    /*start the conversion*/
		    SET_BIT(ADCSRA,ADCSRA_ADSC);
		    
		    /*Enable ADC conversion complete interrupt*/
		    SET_BIT(ADCSRA,ADCSRA_ADIE);
		}
		else
		{
			Local_u8ErrorState = BUSY_ERR;
		}
	}
	else
	{
		Local_u8ErrorState = NULL_PTR;
	}
	    return Local_u8ErrorState;

}

uint8 ADC_u8StartChainConversionAsynch(const ADC_Chain_t* copy_ChainData)
{
	uint8 Local_u8ErrorState = OK;
	
	if(copy_ChainData != NULL)
	{
		if(ADC_u8BusyFlag == IDLE)
		{
			/*ADC is now busy*/
			ADC_u8BusyFlag = BUSY;
			
			/*ISR source is  chain conversions asynchronous functions */
			ADC_u8ISRSource = CHAIN_CONV_ASYNCH;
			
		    /*Convert Local chain data into global*/
		    ADC_pChainData = copy_ChainData;
		    
		    ADC_u8ConversionIndex = 0u;
			
			/*set the first channel*/
			ADMUX &= CHANNEL_SELECTION_MASK;      /*clear the selection bits*/
			ADMUX |= ADC_pChainData->ChannelArr[ADC_u8ConversionIndex];
			
			/*start the conversion*/
			SET_BIT(ADCSRA,ADCSRA_ADSC);
			
			/*Enable ADC conversion complete interrupt*/
			SET_BIT(ADCSRA,ADCSRA_ADIE);
		} 
		else
		{
			Local_u8ErrorState = BUSY_ERR;
		}
		
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
	if(ADC_u8ISRSource == SINGLE_CONV_ASYNCH)
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
	else if (ADC_u8ISRSource == CHAIN_CONV_ASYNCH)
	{
#if ADC_u8RESOLUTION == EIGHT_BITS
    ADC_pChainData->ResultArr[ADC_u8ConversionIndex] = (uint16)ADCH;
#elif ADC_u8RESOLUTION == TEN_BITS
    ADC_pChainData->ResultArr[ADC_u8ConversionIndex] = ADC;
#endif
     
	   /*increment conversion index*/
	   ADC_u8ConversionIndex++;
	   
	   if(ADC_u8ConversionIndex == ADC_pChainData->ConversionsNum)
	   {
		   /*chain is finished*/
		   /*ADC is now idle*/
		   ADC_u8BusyFlag = IDLE;
		   
		   /*Disable ADC conversion complete interrupt*/
		   CLR_BIT(ADCSRA,ADCSRA_ADIE);
		   
		   /*invoke the callback notification function*/
		   if(ADC_pChainData->NotificationFunc != NULL)
		   {
			   ADC_pChainData->NotificationFunc();
		   }
		   else
		   {
			   /*Do nothing*/
		  
		   }
	   }
	   else
	   {
		   /*chain is not finished*/
		   
		   /*set the first channel*/
		   ADMUX &= CHANNEL_SELECTION_MASK;      /*clear the selection bits*/
		   ADMUX |= ADC_pChainData->ChannelArr[ADC_u8ConversionIndex];
		   
		   /*start the conversion*/
		   SET_BIT(ADCSRA,ADCSRA_ADSC);
	   }		

	}
}