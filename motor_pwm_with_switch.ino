#include <stdint.h>
/* 
   Motor speed controller for Go Baby Go Conversions
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

   Authors: CJ Evans, Aya Nakayama, Carmen Quispe, and Jayesh Bhakta (bhaktaj@lacitycollege.edu)
   Institution: Los Angeles City College, Los Angeles, CA 90029
   Date: 07/23/2018 
   
   
   
*/
   

/*Hardware Configuration*/
const uint8_t MOSFET_GATE_PIN=6;
const uint8_t TRIGGER_INPUT=10;
const uint8_t ADC_BATTERY=2;
const uint8_t ADC_DRAIN=1;
const uint8_t LED_RED=4;
const uint8_t LED_YELLOW=3;
const uint8_t LED_GREEN=2;

/*Controller alogrithm constants*/
const int LOOPS_PER_SPEED_INCREMENT=1;
const int TARGET_BEMF_MIN=100;
const int MAX_BEMF=900;
const int PWM_PERIOD_US=2500;
const int BEMF_SETTLING_TIME_US=500;

/*global variables*/
volatile int v_batt;
volatile int v_drain;
volatile int v_bemf;
volatile int v_bemf_error;
volatile int target_bemf;
volatile int pwm_ontime_offset_us;
volatile int pwm_ontime_us=0; 
volatile unsigned int counter=0;

void setup() 
{
  /*set pin MOSFET_GATE_PIN to output*/
  pinMode(MOSFET_GATE_PIN, OUTPUT);
  
  /*ensure pin MOSFET_GATE_PIN is low*/
  digitalWrite(MOSFET_GATE_PIN,LOW);

  /*configure trigger input pin*/
  pinMode(TRIGGER_INPUT, INPUT);

  /*configure led outputs*/
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_YELLOW,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
}

void loop() 
{
  
  /*set battery level indicator LEDs*/
  v_batt=3*analogRead(ADC_BATTERY);
  digitalWrite(LED_GREEN,((v_batt>1024*6/5)?HIGH:LOW));
  digitalWrite(LED_YELLOW,(((v_batt>1024*5/5)&&(v_batt<1024*6/5))?HIGH:LOW));
  digitalWrite(LED_RED,((v_batt<1024*5/5)?HIGH:LOW));
  
  /*reset the speed_setting*/
  target_bemf=TARGET_BEMF_MIN;
  counter=0;
  pwm_ontime_us=0; 
  
  /*motor off*/
  digitalWrite(MOSFET_GATE_PIN,LOW);
  
  /*run motor if trigger has been pressed*/
  while(!digitalRead(TRIGGER_INPUT))
  {
      /*turn the battery indicators off*/
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      
      /*switch motor on for the period corresponding to the duty cycle*/
      digitalWrite(MOSFET_GATE_PIN,HIGH);
      delayMicroseconds(pwm_ontime_us);
  
      /*switch motor off */
      digitalWrite(MOSFET_GATE_PIN,LOW);
      
      /*wait for the Back EMF (BEMF) settling time*/
      delayMicroseconds(BEMF_SETTLING_TIME_US);
      
      /*measure back emf related voltages, sample 3 times to get average*/
      v_batt=3*analogRead(ADC_BATTERY);
      v_drain=3*analogRead(ADC_DRAIN);

      /*complete off time under current pwm duty cycle*/
      delayMicroseconds(PWM_PERIOD_US-pwm_ontime_us-BEMF_SETTLING_TIME_US);

      /*account for noise on v_drain reading*/
      if (v_drain>v_batt)
      {
        v_drain=v_batt;
      }

      /*calculate bemf*/
      v_bemf=v_batt-v_drain;

      /*accelerate motor*/
      if (target_bemf<MAX_BEMF)
      { 
        if((counter%LOOPS_PER_SPEED_INCREMENT)==0)
        {
          target_bemf++;
        }       
        counter++;
      }

      /*calculate back emf and compare to the target value*/
      v_bemf_error=v_bemf-target_bemf;
      
      /* calculate pwm_on_time_offset_us */
      if(target_bemf<=200)
      {
        pwm_ontime_offset_us=200;
      }
      else
      {
        pwm_ontime_offset_us=target_bemf;
      }

      /*calculate the pwm_ontime_us*/
      if(pwm_ontime_offset_us>=v_bemf_error)
      {
        /*simple proportional control*/
        pwm_ontime_us=pwm_ontime_offset_us-v_bemf_error;
        
        /*limit the maximum pwm_ontime*/ 
        if(pwm_ontime_us>2000)
        {
          pwm_ontime_us=2000;
        }
      }
      else
      {
        /*motor running too fast*/
        pwm_ontime_us=0;
      }
  }
}
