//ARCADE LOGIC
//Andrew Armstrong 18/04/2018
//www.backofficeshow.com

//Boobie Config = 
//Int OSC
//Ra0 (pin9) = Reference Voltage - use 1/100 voltage divider connected to VIN
//Ra1 (pin10) = Probe Voltage - use 1/100 voltage divider
//RC0 to RC2 (pin13 to 15) = Status LEDS
//POWERSRC JUMPER should be switched to VIN (not USB)

#include <16F688.h>
// C Optimised for the CCS PIC Compiler
// Using onboard clock
#fuses INTRC_IO,NOWDT,PUT, NOPROTECT,NOMCLR,NOFCMEN,BROWNOUT_SW//NOMCLR
#DEVICE ADC=10 //Give us the full 10 bits ADC!
#use delay(clock=8000000) //Not using the crystal
#use rs232(baud=57600, parity=N, xmit=PIN_C4, rcv=PIN_C5, bits=8, STOP=1)
#define version 1
#DEFINE LOW 0
#DEFINE HIGH 1
#DEFINE UNCERTAIN 3
#DEFINE OFF 255

#int_timer0
void timer0_isr(void)
{  

}

//Serial Interrupt
#INT_RDA
void SerialInt()
{
   getchar();
}

//Sets output based on status
void outputLEDs(unsigned char status)
{
   output_low(pin_c0);
   output_low(pin_c1);
   output_low(pin_c2);
   switch (status)
   {
      case OFF:
         break;
      case LOW:
         output_high(pin_c0);
         printf("L");
         break;
      case HIGH:
         output_high(pin_c2);
         printf("H");         
         break;      
      case UNCERTAIN:
         output_high(pin_c1);
         printf("U");
         break;      
   }
}

//Samples the ADC Port and returns a floating volt value
float read_analogue(unsigned char channel)
{
   float result = 0.0;
   int16 tempval = 0;
   char samples = 0;
   int16 average = 0.0;
   
   set_adc_channel(channel);
   for(samples = 1; samples <= 4; samples += 1)   
   {
      tempval = read_adc(); 
      average += tempval;               
      delay_us(10);   
   }
   
   average = average / 4;
   
   //Result_mV = ( Reference in mV / 1023) * Result
   //So for a reference of 3.3V (e.g.  using Vdd/Vss as ref) and ADC result of 250: 
   //Result = (3300/1023) * 250 = 806mV 
   
   result = average * (5000/1023); 

   result = result * 10;
   
   return result;                                 
   }

void main()
{
   float supplyVoltage; 
   float testVoltage;       
   
   setup_timer_0 ( RTCC_INTERNAL | RTCC_8_BIT | RTCC_DIV_2 );
   setup_adc(adc_clock_internal); //The ADC uses a clock to work
   setup_adc_ports(sAN0 | sAN1); //RC2 as analogue
   enable_interrupts(INT_RTCC);
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);

   printf("\r\nBoobie Logic v%d\r\nAndrew Armstrong @backofficeshow\r\n", version);

   outputLEDs(OFF);

   while (TRUE)
   {
      supplyVoltage = read_analogue(0); 
      testVoltage = read_analogue(1); 
      
      printf("%02.2fv ", testVoltage / 1000);
      
      if ((supplyVoltage >= 4700) && (supplyVoltage <= 5250)) //TTL Logic
      {
         printf("T ");
         if (testVoltage >= 2000)
         {
            outputLEDs(HIGH);
         }
         else if (testVoltage <= 800)
         {
            outputLEDs(LOW);
         }
         else
         {
            outputLEDs(UNCERTAIN);
         }
      }
      else //CMOS Logic
      {
         printf("C ");
         if (testVoltage >= supplyVoltage*0.66)
         {
            outputLEDs(HIGH);
         }
         else if (testVoltage <= supplyVoltage*0.33)
         {
            outputLEDs(LOW);
         }
         else
         {
            outputLEDs(UNCERTAIN);
         }
      }
      printf("\r\n");
   }  
} 
