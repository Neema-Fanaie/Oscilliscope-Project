
#include <Arduino.h>
#include <U8g2lib.h>


const uint8_t PIN_RES = 8;
const uint8_t PIN_DC = 9;
const uint8_t PIN_CS = 10;

U8G2_SH1106_128X64_NONAME_2_4W_HW_SPI display(U8G2_R0, PIN_CS, PIN_DC, PIN_RES);

void grid(){

  for (uint16_t i = 0; i <= 8; i++) {
    for (uint16_t j = 0; j <= 4; j++) {
      
      uint16_t x = i * 16;
      uint16_t y = j * 16;

      if (x == 0 && y == 0){
        
        display.drawLine(x, y, x+1, y);
        display.drawLine(x, y, x, y+1);

        display.drawLine(x + 6, y, x + 9, y);
        display.drawLine(x, y + 6, x, y + 9);

      } else if (x == 0 ){
      
        display.drawLine(x, y-1, x+1, y-1);
        display.drawLine(x, y-2, x, y);

        display.drawLine(x + 6, y - 1 , x + 9, y -1);
        display.drawLine(x, y + 5 , x, y + 8);
      
      } else if (y == 0){

        display.drawLine(x-2, y, x, y);
        display.drawLine(x-1, y, x-1, y+1);         

        display.drawLine(x + 5, y, x + 8, y);
        display.drawLine(x - 1 , y + 6 , x - 1, y + 9); 
      
      } else {
        
        display.drawLine(x - 2, y - 1 , x, y -1);
        display.drawLine(x - 1, y -2, x - 1 , y);

        display.drawLine(x + 6, y - 1 , x + 9, y -1);
        display.drawLine(x - 1 , y + 6 , x - 1, y + 9); 
      }
    }
  }
}