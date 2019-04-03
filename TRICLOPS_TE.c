/*****************************************************************************
TYSON EARL                                                             7/25/14
TY'S CUSTOM ELECTRONICS
TRICLOPS
    
-    CREATED FOR CHAUNCY'S MTN BIKE LED
-    D6 WILL TOGGLE FOR VBATT UNDER/OVER VOLTAGE 9-12.7V OK
-    D5 WILL TOGGLE FOR OVER CURRENT CONDITIONS < 2.7AMPS OK
-    REFINED THE VDD_VSS REFERENCE VOLTAGE NOW 5.04V
-    NOW TAKING 10 SAMPLES OF CURRENT CHANNEL TO CLEAN UP
-    PI LOOP BACK ADDED BACK IN - NEW SETPNTS SELECTED
-    PID LOOP ACTIVE FOR N>1
-    INPUT SWITCH NOW RUNS THROUGH WAKEUP AND N COUNTER FOR 6 STATIC SETPOINTS
-    PI LOOP WOKRING WITH ADDITION OF SETPNT AND INTEGER FOLDBACK
******************************************************************************/
#include <16F884.h>
#device ADC=10
#fuses NOWDT, INTRC_IO, PUT, MCLR, NOPROTECT, NOCPD, NOBROWNOUT, IESO, FCMEN, NOLVP, NODEBUG, NOWRT, BORV40
#use DELAY(CLOCK=8000000)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7) 

#include <stdlib.h>

#define HEARTBEAT PIN_A4
#define BUTTON    PIN_B0
#define LED       PIN_C5  
#define D1        PIN_E0
#define D2        PIN_E1
#define D3        PIN_E2
#define D4        PIN_A7
#define D5        PIN_A6
#define D6        PIN_C0
#define Kp (20)
#define Ki (5.5)

/////////////////////////// VARIABLES //////////////////////////////////

float err,integral=0;
float Batt,Curr,RTD; 
unsigned int8 p,y,z,width=0,x=0,w=0;
signed int8 xp=0,n;
long value1[10],value2[10],value3[5],mean1=0,mean2=0,mean3=0;
const float setpnt[7]={0,0.025,0.2,0.5,0.75,1.0,1.5};// units are in Amps
const int dutytable[7]={1,2,7,20,45,60,70}; // units are xx/255 duty cycle
const int dutytable2[7]={1,2,5,15,30,40,55};// units are xx/255 duty cycle
unsigned int16 res1,res2,res3;
int16 TICK0,TICK1;
////////////////////////////////////////////////////////////////////////////////

void a2d_sub();

void PID();


void main(){

   output_low(LED);
   delay_ms(500);


   setup_adc_ports(sAN0|sAN1|sAN2|VSS_VDD);
   setup_adc(ADC_CLOCK_DIV_2);
   
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_16);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   setup_timer_2(T2_DIV_BY_16, 127, 1);
   
   enable_interrupts(INT_RTCC);
   enable_interrupts(INT_TIMER1);
   enable_interrupts(GLOBAL);
   
   set_timer0(0);    // START TIMER1 OVER
   set_timer1(0);    // START TIMER1 OVER
   TICK0=0;
   TICK1=0;
   n=0;
   
///////////////////// BUTTON PRESSED < 30 SEC CHECK ///////////////////////////   

   while(true){
      do {
         //if(TICK1>930){//If 30 sec has passed
         //} 
      } while(input(BUTTON));//while BUTTON is high do nothing
      delay_ms(10);
      
   ///////////////////////////////// WAKEUP //////////////////////////////////////
      setup_ccp1(CCP_PWM);   // Configure CCP1 as a PWM
      set_pwm1_duty(0);
      TICK1=0;// START COUNTS OVER
   ///////////////////////// BUTTON N COUNTER INCREMENT //////////////////////////  
     while(n<7){
      TICK1=0;
      //////////////////////// BUTTON PRESSED ////////////////////////////////////
      if(!input(BUTTON)){
         delay_ms(10);//switch debounce
         if((n<0)||(n>8)){
            n=0;
         }
         n+=1;//incriment loopup table forward   
         a2d_sub();
         if(Batt>=1.04){// >= 11.5V
            width=dutytable2[n];
         }
         if(Batt<1.04){//  <  11.5V
            width=dutytable[n];
         }
         delay_ms(20);
         
         while(!input(BUTTON)){
         if(TICK1>40){//IF BUTTON held longer than 3 sec break from loop n>5    
            n=7;
            printf("RESET - STATE 0");
            break;
         }
         }//WHILE PRESSING DOWN DO NOTHING/////////////////////////////////////////
         switch(n){
            case 1: 
            delay_ms(150);
            output_high(D1);
            delay_ms(50);
            break;
            case 2:
            output_low(D1);
            output_low(D2);
            delay_ms(150);
            output_high(D1);
            delay_ms(50);
            output_high(D2);
            delay_ms(50);
            break;
            case 3:
            output_low(D1);
            output_low(D2);
            output_low(D3);
            delay_ms(150);
            output_high(D1);
            delay_ms(50);
            output_high(D2);
            delay_ms(50);
            output_high(D3);
            delay_ms(50);
            break;
            case 4:
            output_low(D1);
            output_low(D2);
            output_low(D3);
            output_low(D4);
            delay_ms(150);
            output_high(D1);
            delay_ms(50);
            output_high(D2);
            delay_ms(50);
            output_high(D3);
            delay_ms(50);
            output_high(D4);
            delay_ms(50);
            break;
            case 5:
            output_low(D1);
            output_low(D2);
            output_low(D3);
            output_low(D4);
            output_low(D5);
            delay_ms(150);
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
            break;
            case 6:
            output_low(D1);
            output_low(D2);
            output_low(D3);
            output_low(D4);
            output_low(D5);
            output_low(D6);
            delay_ms(150);
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
            break;
         }
    ///////////////////////////////////////////////////////////////////////////////     
   
      }
      if (input(BUTTON) && (n<=6)){
         delay_ms(15);
      //////////////////////// BUTTON RELEASE ////////////////////////////////////
         while(input(BUTTON)){
            a2d_sub();
            ///////////////// CONDITIONAL CHECKS ///////////////////    
            if((Batt<0.83)||(Batt>1.21)){// UNDER OR OVER VOLTAGE TEST
               output_low(D1);
               output_low(D2);
               output_low(D3);
               output_low(D4);
               output_low(D5);
               output_high(D6);
               width=0;
               n=0;// get out of loop, battery low
               set_pwm1_duty(width);            
               
               while((Batt<0.87)||(Batt>1.2)){
                  a2d_sub();
                  output_high(D6);
                  delay_ms(100);
                  output_low(D6);
                  delay_ms(100);
                  width=0;
                  n=0;// get out of loop, battery low
               }
            }
            set_pwm1_duty(width); 
            delay_ms(50);
            if(n>1){
               PID();  
            }
            if(Curr>2.5){                 //RTD TEMP TEST
               output_low(D1);
               output_low(D2);
               output_low(D3);
               output_low(D4);
               //output_low(D5);
               output_low(D6);
               output_high(D5);
               width=0;
               set_pwm1_duty(width);
               n=0;// get out of loop, battery low
                  while(input(BUTTON)){           
                     output_high(D5);
                     delay_ms(100);
                     output_low(D5);
                     delay_ms(100);
               }
            }
   
      ////////////////////////////////////////////////////////////////////////////         
            TICK1=0;
            while(TICK1<17){//5 SECONDS
              if (!input(BUTTON)){
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
     output_low(LED);
     set_pwm1_duty(0);
     x = 0;
     err=0;
     integral=0;
     mean1=0;
     mean2=0;
     mean3=0;
     TICK1 = 0;
     n=0;
     output_low(D1);
     output_low(D2);
     output_low(D3);
     output_low(D4);
     output_low(D5);
     output_low(D6);
     //setpnt=0;
     while(!input(BUTTON)){}//WHILE PRESSING DOWN DO NOTHING
     delay_ms(10);
   }
}
 
 
#int_TIMER1                //THIS MONITORS INACTIVITY TIME (BETWEEN MAGNET REVOLUTIONS),ALSO USED FOR EEPROM DATA AQUISITION
void TIMER1_isr() {                           
   TICK1++;// there are ((8E9/(2^16))/4) OFLOWS PER SEC ~ 31 PER SEC
}

#int_TIMER0                //THIS MONITORS INACTIVITY TIME (BETWEEN MAGNET REVOLUTIONS),ALSO USED FOR EEPROM DATA AQUISITION
void TIMER0_isr() {                           
   TICK0++;// there are ((8E9/(2^16))/4) OFLOWS PER SEC ~ 31 PER SEC
   if(TICK0>500){
      output_high(HEARTBEAT);
      delay_ms(50);
      output_low(HEARTBEAT);
      TICK0=0;
   }
}
   
   
void a2d_sub(){
         
   set_adc_channel(0);//AN0 IS VBATT MONITOR
   for(z=0;z<10;z++){ //10 SAMPLES TAKEN
        value1[z] = read_adc();
        mean1+=value1[z];// ADD THEM UP
        delay_ms(1);//was 150
   }

   set_adc_channel(1);//AN1 IS CURRENT MONITOR
   for(y=0;y<10;y++){ //10 SAMPLES TAKEN
        value2[y] = read_adc();
        mean2+=value2[y];// ADD THEM UP
        delay_ms(1);//was 150
   }

   set_adc_channel(2);//AN2 IS THE TEMP MONITOR
   for(p=0;p<5;p++){ //10 SAMPLES TAKEN
        value3[p] = read_adc();
        mean3+=value3[p];// ADD THEM UP
        delay_ms(1);//was 150
   }
          
       mean1=mean1/10;
       mean2=mean2/10;//AVERAGE ALL 10 SAMPLES
       mean3=mean3/5;//AVERAGE ALL 10 SAMPLES   
          
       res1 = (float)(mean1);//  VBatt      
       res2 = (float)(mean2);// Current   
       res3 = (float)(mean3);// Temp
       
       Batt = (float)res1*.00492; // 5.04V     resolution VBatt
       Curr = (float)res2*.00492; // 5.04V     resolution Current        
       RTD  = (float)res3*.00492; // 5.04V     resolution RTD Temp Sensor
}
 
void PID (){
   //output = Kp * err + (Ki * int * dt) + (Kd * der /dt);
   for(w=0;w<10;w++){

   set_pwm1_duty(width);           
   for(x=0;x<100;x++){
      delay_ms(1);
      if(!input(BUTTON)){
        break;
      }
   }
   if(!input(BUTTON)){
        break;
   }
   a2d_sub();
   err=setpnt[n]-Curr; 
   integral+=(err*Ki);
   xp=(signed int8)((err*Kp)+integral);
   
   if(xp<0){//this prevents runaway conditions where setpoint will go to 0
     xp=0;
     n--;
      output_low(D1);
      output_low(D2);
      output_low(D3);
      output_low(D4);
      output_low(D5);
      output_low(D6);
      delay_ms(50);
      switch(n){
         case 1: 
         delay_ms(50);
         output_high(D1);
         delay_ms(50);
         break;
         case 2:
         output_low(D1);
         output_low(D2);
         delay_ms(50);
         output_high(D1);
         delay_ms(50);
         output_high(D2);
         delay_ms(50);
         break;
         case 3:
         output_low(D1);
         output_low(D2);
         output_low(D3);
         delay_ms(50);
         output_high(D1);
         delay_ms(50);
         output_high(D2);
         delay_ms(50);
         output_high(D3);
         delay_ms(50);
         break;
         case 4:
         output_low(D1);
         output_low(D2);
         output_low(D3);
         output_low(D4);
         delay_ms(50);
         output_high(D1);
         delay_ms(50);
         output_high(D2);
         delay_ms(50);
         output_high(D3);
         delay_ms(50);
         output_high(D4);
         delay_ms(50);
         break;
         case 5:
         output_low(D1);
         output_low(D2);
         output_low(D3);
         output_low(D4);
         output_low(D5);
         delay_ms(50);
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
         break;
         case 6:
         output_low(D1);
         output_low(D2);
         output_low(D3);
         output_low(D4);
         output_low(D5);
         output_low(D6);
         delay_ms(50);
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
         break;
      
      }
      if(n<=0){
         n=20;
      }
   }
   width=(int8)xp;
   }
   break;
}

