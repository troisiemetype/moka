
#include "Moka.h"

Moka board;

uint8_t color = 0b00110100;

void setup(){
	Serial.begin(115200);

	board.begin(10);
	board.setGlobalColor(color);
/*
	for(uint8_t i = 0; i < 16; ++i){
		board.setLed(i);
		board.updateLeds();
		board.updateDisplay();
		delay(50);
	}

	for(uint8_t i = 0; i < 16; ++i){
		board.clrLed(i);
		board.updateLeds();
		board.updateDisplay();
		delay(50);		
	}
*/
}

void loop(){
	board.readButtons();
//	board.setGlobalColor(++color);
	for(uint8_t i = 0; i < 16; ++i){
//		Serial.print(board.isPressed(i));

		if(board.isJustPressed(i)){
			board.setLed(i);
		} else if(board.isJustReleased(i)){
			board.clrLed(i);
		} else {

		}
	}
//	Serial.println();

	board.updateLeds();
	board.updateDisplay();
	delay(10);

}