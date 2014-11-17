//timer interrupts
//by Amanda Ghassaei
//June 2012
//http://www.instructables.com/id/Arduino-Timer-Interrupts/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 */


void setup(){

  //set pins as outputs
  pinMode(8, OUTPUT);


  cli();//stop interrupts


}//end setup


#define DELAY_1CYCLE() \
    __asm__("nop\n\t")

#define DELAY_1CYCLE_PLUS2() \
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t")            

#define DELAY_1US() \
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t");\
    __asm__("nop\n\t")
    
#define DELAY_10US() \
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US()


#define DELAY_50US() \
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US()


#define DELAY_20US() \
    DELAY_10US();\
    DELAY_10US()



#define DELAY_100US() \
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US();\
    DELAY_10US()

#define DELAY_5US() \
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US();\
    DELAY_1US()

#define DELAY_2US() \
    DELAY_1US();\
    DELAY_1US()
    
    


#define DELAY_99US()\
  DELAY_20US();\  
  DELAY_20US();\
  DELAY_50US();\
  DELAY_5US();\
  DELAY_2US();\
  DELAY_2US()
  
#define DELAY1()\
  DELAY_1CYCLE_PLUS2();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\

  
#define DELAY2()\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\ 
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\
  DELAY_1CYCLE();\

  
void loop(){

//Enter museum here.
Loop:

  PORTB = 0xFE;  
  DELAY1(); //1us

  DELAY_99US();
//  DELAY_100US();

  PORTB = 0x01;  
  DELAY2();  //1us

  DELAY_99US();
//  DELAY_100US();
  
  goto Loop;  
  
}

