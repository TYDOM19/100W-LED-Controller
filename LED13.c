/*****************************************************************************
TYSON EARL                                                             6/2/14
TY'S CUSTOM ELECTRONICS
100W LED CURRENT LOOP CONTROLLER REV5
    
-    CREATING THIS FOR CHAUNS MTN BIKE HID LED 
-    HARDWARE CCP1 IS WORKING, USB BRIDGE IS WORKING, ANALOG CHANNEL 3 WORKING
-    1 SAMP/5ms
-    INPUT SWITCH NOW RUNS THROUGH WAKEUP AND N COUNTER FOR 10 STATIC SETPOINTS
-    PI LOOP WOKRING WITH ADDITION OF SETPNT AND INTEGER FOLDBACK
******************************************************************************/
#include <16F884.h>
#device ADC=10
#fuses INTRC_IO,NOWDT,NOPROTECT,NOLVP,NOMCLR,NOWRT
#use delay(clock=8000000)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7) // Preprocessor directive rs232

#define button PIN_B0
#define led  PIN_C5// CURRENTLY CONNECTED TO HEXFET
#define D1 PIN_E0
#define D2 PIN_E1
#define D3 PIN_E2
#define D4 PIN_A7
#define D5 PIN_A6
#define D6 PIN_C0
#define HEARTBEAT PIN_A4
#include <stdlib.h>
/////////////////////////// GLOBAL VARIABLES //////////////////////////////////


//int32 i;
float Kp=1.6,Ki=.5;//1.6 .5
//float Kp=1.95,Ki=.9;
unsigned int8 c,y,z,n,s,t,width=0,x=0,w=0;
signed int8 err=0,integral=0,xp=0;
long value[10],mean=0,mean1=0;
//char selection;
int16 TICK0,TICK1;
const int dutytable[6]={1,2,25,50,90,125};
////////////////////////////////////////////////////////////////////////////////


void a2d_sub(){
          setup_adc_ports(sAN0|sAN1|sAN2|VSS_VDD);        //USES DEFAULT VDD,VSS REFERENCE VOLTAGES
          setup_adc(ADC_CLOCK_INTERNAL);    //THIS SETS UP THE ADC mode,SPEED, & SAMPLE TIME
          
          //for(c=5;c<9;c+3){ // LOOPS THROUGH BOTH sAN4&sAN7
               set_adc_channel(7);//AN7 IS THE VBATT MONITOR
               //delay_us(10);//was 150
               for(z=0;z<10;++z){ //40 SAMPLES PER CHANNEL ARE TAKEN
                    value[z] = read_adc();
                    mean1+=value[z];// ADD THEM UP
                    delay_us(100);//was 150
               }
               
               set_adc_channel(3);//AN3 IS THE CURRENT MONITOR
               //delay_us(10);//was 150
               for(y=0;y<10;++y){ //40 SAMPLES PER CHANNEL ARE TAKEN
                    value[y] = read_adc();
                    mean+=value[y];// ADD THEM UP
                    delay_ms(5);//was 150
               }
          
          mean1=mean1/10;//AVERAGE ALL 10 SAMPLES
          mean=mean/10;
                    
          //res1 = ((float)(mean)*10);       // 3V=3.5A SHUNT CURRENT
          //res2 = ((float)(mean1)*13.7);       // 5V=45V  VBATT

}
 
/*void PID (){
for(w=0;w<7;w++){
//output = Kp * err + (Ki * int * dt) + (Kd * der /dt);
   set_pwm1_duty(x);          // This sets the time the pulse isa2d_sub();
   for(x=0;x<100;x++){
      delay_ms(1);
      if(!input(button)){
        break;
      }
   }
      if(!input(button)){
        break;
      }
   a2d_sub();
   err=setpnt-mean; 
   integral+=(signed int8)((err*Ki));
   xp=(signed int8)((err*Kp)+integral);
   
   if(xp<0){//this prevents runaway conditions where setpoint will go to 0
     xp=0;
   }
   if(xp>125){// 50% duty cycle or greater
      setpnt=setpnt-20;
      strtup=strtup-20;
      n=n-1;
   }
   if(setpnt>100){// setpnt limiting to 75
      setpnt=100;
   }
   if(setpnt<=0){// setpnt limiting to 0
      setpnt=0;
   }
   if(n<=0){// n limiting to 0
      n=0;
   }
   x=(int8)xp;
   printf("\n\rsetpnt= %03u AN3= %04lu ERR= %03d INT= %03d Duty= %03u",setpnt,mean,err,integral,x);
}
break;
}*/

////////////////////////////// START //////////////////////////////////////////
void main(void){
   output_low(led);
   delay_ms(500);
   
/////////////////////////// INTRO SCREEN //////////////////////////////////////

   //printf("\f");
   //printf("\r\n100W LED SMART CONTROLLER:\r\n");
   delay_ms(500);

   setup_timer_2(T2_DIV_BY_16, 127, 1);
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_32);
   
   ENABLE_INTERRUPTS(INT_TIMER0);
   ENABLE_INTERRUPTS(INT_TIMER1);
   ENABLE_INTERRUPTS(GLOBAL); 
   set_timer1(0);    // START TIMER1 OVER
   set_timer1(0);    // START TIMER1 OVER
   TICK0=0;
   TICK1=0;
   n=0;
   //setpnt=0;
   
///////////////////// BUTTON PRESSED < 30 SEC CHECK ///////////////////////////   

while(true){
   do {
      if(TICK1>930){//If 30 sec has passed
         printf("\n\r\nTIRED?\r\n");
      }
      
   } while(input(button));//while button is high do nothing
   delay_ms(10);
   
///////////////////////////////// WAKEUP //////////////////////////////////////

   //printf("\n\r\nI'M AWAKE:\r\n");
   setup_ccp1(CCP_PWM);   // Configure CCP1 as a PWM
   set_pwm1_duty(0);
   TICK1=0;// START COUNTS OVER 

   
///////////////////////// BUTTON N COUNTER INCREMENT //////////////////////////  
  //integral=0;
  //setpnt=0;
  //x=1;
  while(n<5){
  TICK1=0;
  if(!input(button)){
      delay_ms(10);//switch debounce
      n+=1;   
      width=dutytable[n];
      delay_ms(45);
      
      while(!input(button)){
      if(TICK1>50){//IF button held longer than 3 sec break from loop n>5    
         n=6;
         //printf("\r\nPANIC BUTTON PRESSED= %lu OFLOWS\r\n",TICK1);
         break;
      }
      }//WHILE PRESSING DOWN DO NOTHING/////////////////////////////////////////
      
      output_high(D1);
      delay_ms(50);
      output_high(D2);
      delay_ms(50);
      output_high(D3);
      delay_ms(50);
      output_high(D4);
      delay_ms(50);
      output_high(D5);
      delay_ms(50);
      output_high(D6);
      delay_ms(50);
 ///////////////////////////////////////////////////////////////////////////////     

   }
   //printf("\n\r n=%u \n",n);
   
   if (input(button) && (n<=5)){
      delay_ms(10);
      output_low(D1);
      output_low(D2);
      output_low(D3);
      output_low(D4);
      output_low(D5);
      output_low(D6);
      while(input(button)){
         //DO PID LOOP HERE & OCCATIONALLY CHECK FOR BUTTON DEPRESS     
         set_pwm1_duty(width); 
         //PID();
         TICK1=0;
         while(TICK1<10){//30 SECONDS
           if (!input(button)){
               break;
           }
         }
         TICK1=0;//RESET COUNTER
      }
      //delay_ms(20);
   }
  }
///////////////////////////////////////////////////////////////////////////////  
  delay_ms(10);
  output_low(led);
  set_pwm1_duty(0);
  x = 0;
  err=0;
  integral=0;
  mean=0;
  TICK1 = 0;
  n=0;
  output_low(D1);
  output_low(D2);
  output_low(D3);
  output_low(D4);
  output_low(D5);
  output_low(D6);
  //setpnt=0;
  while(!input(button)){}//WHILE PRESSING DOWN DO NOTHING
  delay_ms(10);

}
}

   
   #int_TIMER1                //THIS MONITORS INACTIVITY TIME (BETWEEN MAGNET REVOLUTIONS),ALSO USED FOR EEPROM DATA AQUISITION
   void TIMER1_isr() {                           
   TICK1=TICK1+1;// there are ((8E9/(2^16))/4) OFLOWS PER SEC ~ 31 PER SEC
   }

   #int_TIMER0                //THIS MONITORS INACTIVITY TIME (BETWEEN MAGNET REVOLUTIONS),ALSO USED FOR EEPROM DATA AQUISITION
   void TIMER0_isr() {                           
   TICK0=TICK0+1;// there are ((8E9/(2^16))/4) OFLOWS PER SEC ~ 31 PER SEC
   if(TICK0>500){
      output_high(HEARTBEAT);
      delay_ms(50);
      output_low(HEARTBEAT);
      TICK0=0;
   }
   }
