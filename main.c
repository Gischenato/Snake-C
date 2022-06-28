/* Nokia 5110 LCD AVR Library example
*
* Copyright (C) 2015 Sergey Denisov.
* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public Licence
* as published by the Free Software Foundation; either version 3
* of the Licence, or (at your option) any later version.
*
* Original library written by SkewPL, http://skew.tk
* Custom char code by Marcelo Cohen - 2021
*/

#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "nokia5110.h"


#define TIMER_CLK F_CPU / 2048
#define IRQ_FREQ 1

typedef struct nodo {
     short x;
     short y;
}Nodo;

typedef struct fila {
     Nodo fila[100];
     int inicio, fim;
     int qnt, tam; 
}Fila;

Nodo newNodo(int x, int y){
     Nodo p;
     p.x = x;
     p.y = y;
     return p;
}

short frutaX = 8;
short frutaY = 12;
short vx = 2;
short vy = 0;
short x = 20;
short y = 20;
short terminou = 0;
short total = 0;

Fila corpo;

// uint8_t snake[] = { 0b011110, 0b011110, 0b011110, 0b011110, 0b011110 };

void start(int x, int y){
     // nokia_lcd_set_cursor(42 + x, 24 + y);
     nokia_lcd_set_pixel(42+x, 24+y, 1);

}

void desenhaBorda(){
     for(int i = 0; i < 83; i++){
          nokia_lcd_set_pixel(i,0,1);
          nokia_lcd_set_pixel(i,47,1);
     }
     for(int i = 0; i < 84; i++){
          nokia_lcd_set_pixel(0,i,1);
          nokia_lcd_set_pixel(83,i,1);
     }
}



void insert(Fila *f, int x, int y){
     if(f->qnt == f->tam){
          // printf("FILA CHEIA\n");
          return;
     }
     f->fim = (f->fim + 1) % f->tam;
     Nodo n = newNodo(x, y);
     f->fila[f->fim] = n;
     f->qnt += 1;
}

void removeNodo(Fila *f){
     if(f->qnt == 0) return;
     f->inicio = (f->inicio + 1) % f->tam;
     f->qnt -= 1;
}

int insertTest(Fila *f, int x, int y){

     if(f->fim >= f->inicio){
          for(int i = f->inicio; i<=f->fim; i++){
               Nodo n = f->fila[i];
               if(n.x == x && n.y == y)
                    return 1;
          }
     }
     else{
          for(int i = f->inicio; i<f->tam; i++){
               Nodo n = f->fila[i];
               if(n.x == x && n.y == y)
                    return 1;
          }
          for(int i = 0; i<=f->fim; i++){
               Nodo n = f->fila[i];
               if(n.x == x && n.y == y)
                    return 1;
          }
     }
     insert(f, x, y);
     if(x == frutaX && y == frutaY) return 2;
     return 0;
}

void printa(Fila *f){
     if(f->fim >= f->inicio){
          for(int i = f->inicio; i<=f->fim; i++){
               Nodo n = f->fila[i];
               start(n.x, n.y);
          }
     }
     else{
          for(int i = f->inicio; i<f->tam; i++){
               Nodo n = f->fila[i];
               start(n.x, n.y);
          }
          for(int i = 0; i<=f->fim; i++){
               Nodo n = f->fila[i];
               start(n.x, n.y);
          }
     }
}


ISR(TIMER1_COMPA_vect){
     // srand(2000);
     nokia_lcd_clear();
     desenhaBorda();
     // start(frutaX, frutaY);
     if(terminou) return;
     x += vx;
     y += vy;
     if(43 + x > 82) x = 39;
     if(41 + x < 0)  x = -41;
     if(30 + y > 51) y = 21;
     if(23 + y < 0)  y = -23;
     int bateu = insertTest(&corpo, x, y);
     if(bateu == 1){
          nokia_lcd_clear();
          terminou = 1;
          return;
     }
     if(bateu == 0) removeNodo(&corpo);
     else{
          frutaX = rand() % 20;
          if(frutaX % 2 == 1)frutaX++;
          frutaY = rand() % 20;
          if(frutaY % 2 == 1)frutaY++;
     }
     printa(&corpo);
     start(frutaX, frutaY);
     // printa(&macas);
     // start(x,y);
     //nokia_lcd_set_pixel(67, 10, 1);
     nokia_lcd_render();

}


int main(void){
     corpo.fim = -1;
     corpo.tam = 100;
     corpo.inicio = corpo.qnt = 0;
     corpo.fim = -1;
     insert(&corpo, 27, 24);
     insert(&corpo, 26, 26);
     // insert(&corpo, 28, 25);
     // insert(&corpo, 25, 27);
     // insert(&corpo, 24, 28);
     // insert(&corpo, 23, 29);
     // insert(&corpo, 22, 30);
     // insert(&corpo, 21, 31);
     cli();

     //! Timer
     TCCR1A = 0;
     TCCR1B = 0;
     TCNT1 = 0;
     OCR1A = (TIMER_CLK / IRQ_FREQ) - 1;
     TCCR1B |= (1 << WGM12);
     TCCR1B |= (1 << CS12) | (1 << CS10);
     TIMSK1 |= (1 << OCIE1A);

     //? Botoes

     DDRB &= ~((1 << PB0) | (1 << PB7));
     DDRD &= ~((1 << PD7) | (1 << PD6) | (1 << PD5));

     PORTB &= ~((1 << PB0) | (1 << PB7));
     PORTD &= ~((1 << PD7) | (1 << PD6) | (1 << PD5));

     sei();

     nokia_lcd_init();
     nokia_lcd_clear();
     // nokia_lcd_custom(1, snake);

     for (;;){
          //start(frutaX, frutaY);
          if (PINB & (1 << PB0)){
               if(vy != -2){
                    vx = 0;
                    vy = 2;
               }
          }
          if (PIND & (1 << PD6)){
               if(vx != 2){
                    vy = 0;
                    vx = -2;
               }
          }
          if (PIND & (1 << PD7)){
               if(vx != -2){
                    vy = 0;
                    vx = 2;
               }    
          }
          if (PIND & (1 << PD5)){
               if(vy != 2){
                    vx = 0;
                    vy = -2;
               }
          } 
          if (PINB & (1 << PB7)){
               for(int i = 0; i < corpo.qnt; i++) removeNodo(&corpo);
               corpo.fim = -1;
               corpo.tam = 100;
               corpo.inicio = corpo.qnt = 0;
               corpo.fim = -1;
               vy = 2;
               vx = 0;
               x = 0;
               y = 0;
               insert(&corpo, 0, 0);
               terminou = 0;
          } 
}
}