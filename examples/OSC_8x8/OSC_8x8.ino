// This program uses OSC (https://github.com/CNMAT/OSC) to interface pad with pureData

#include "Moka.h"

#include <OSCBundle.h>
#include <OSCMessage.h>
#include <OSCBoards.h>

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial(SerialUSB);
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif

Mokas board;

uint8_t color = 0b00110000;

enum KEY_MODE{
	KEY_BY_POS = 0,
	KEY_BY_INDEX,
};

uint8_t keymode = KEY_BY_POS;

enum LED_SET{
	SET_IDLE = 0,
	SET_STATE,
	SET_INT,
	SET_RGB,
};

uint8_t nowSetting = SET_IDLE;

uint8_t ctr = 0;

bool update = false;


void setup(){

	board.beginAuto(2, 2);

	board.setGlobalColor(color);

	board.setDebounce(5);

	uint16_t maxSize = board.getSizeX() * board.getSizeY();

	for(uint8_t j = 0; j < 2; ++j){
		for(uint8_t i = 0; i < maxSize; ++i){
			board.setLed(i);
		}

		board.updateLeds();
		board.updateDisplay();
		delay(50);

		for(uint8_t i = 0; i < maxSize; ++i){
			board.clrLed(i);
		}

		board.updateLeds();
		board.updateDisplay();
		delay(150);
	}

    SLIPSerial.begin(115200);   // set this as high as you can reliably run on your platform
#if ARDUINO >= 100
    while(!Serial)
      ;   // Leonardo bug
#endif

}

void loop(){
 	int size = SLIPSerial.available();

 	if(size){
 		inComming();

 	}

	if(board.readButtons()){
	 	OSCBundle bundleOut;
	 	if(keymode == KEY_BY_POS){
	 		uint8_t maxX = board.getSizeX();
	 		uint8_t maxY = board.getSizeY();
			for(uint8_t x = 0; x < maxX; ++x){
				for(uint8_t y = 0; y < maxY; ++y){
					if(board.isJustPressed(x, y)){
						bundleOut.add("/key").add((int32_t)x).add((int32_t)y).add(true);
					} else if(board.isJustReleased(x, y)){
						bundleOut.add("/key").add((int32_t)x).add((int32_t)y).add(false);
					}
				}
			}
		} else if(keymode == KEY_BY_INDEX){
	 		uint8_t maxSize = board.getSizeX() * board.getSizeY();
			for(uint8_t i = 0; i < maxSize; ++i){
				if(board.isJustPressed(i)){
					bundleOut.add("/key").add((int32_t)i).add(true);
				} else if(board.isJustReleased(i)){
					bundleOut.add("/key").add((int32_t)i).add(false);
				}
			}
		}

		SLIPSerial.beginPacket();
		bundleOut.send(SLIPSerial);
		SLIPSerial.endPacket();
		bundleOut.empty();
		
	}

	if(update){
		board.updateLeds();
		board.updateDisplay();
		update = false;
	}
}

void inComming(){
	OSCBundle bundleIn;

	int size = 0;

	while (!SLIPSerial.endofPacket()){
		if((size = SLIPSerial.available()) > 0){

			while(size--){
				bundleIn.fill(SLIPSerial.read());
			}
		}
	}

//	if(!bundleIn.hasError()){
		bundleIn.route("/led", led);
//		bundleIn.route("/led/set", ledSet);
//		bundleIn.route("/led/all", ledAll);

		bundleIn.route("/size", sendSize);
		bundleIn.dispatch("/key", setKeyMode);
		bundleIn.dispatch("/start", startSequence);
//	} else {
//		board.clrLed(1);
//	}

	bundleIn.empty();
}

void led(OSCMessage &msg, int offset){
//	board.setLed(ctr++);
//	board.updateLeds();
//	board.updateDisplay();
	update = true;
	msg.route("/set", ledSet, offset);
	msg.route("/int", ledInt, offset);
	msg.route("/rgb", ledRGB, offset);
	nowSetting = SET_IDLE;
}

void ledSet(OSCMessage &msg, int offset){
	nowSetting = SET_STATE;
	ledDispatch(msg, offset);
}

void ledInt(OSCMessage &msg, int offset){
	nowSetting = SET_INT;
	ledDispatch(msg, offset);
}

void ledRGB(OSCMessage &msg, int offset){
	nowSetting = SET_RGB;
	ledDispatch(msg, offset);
}

void ledDispatch(OSCMessage &msg, int offset){
	if(msg.fullMatch("/one", offset)){
		ledOne(msg, offset);
	} else if(msg.fullMatch("/row", offset)){
		ledRow(msg, offset);
	} else if(msg.fullMatch("/col", offset)){
		ledCol(msg, offset);
	} else if(msg.fullMatch("/map", offset)){
		ledMap(msg, offset);
	} else if(msg.fullMatch("/all", offset)){
		ledAll(msg, offset);
	}
}

void ledOne(OSCMessage &msg, int offset){
	uint8_t size = msg.size();

	uint8_t x, y;
	uint8_t state;

	state = (uint8_t)msg.getInt(0);
	x = (uint8_t)msg.getInt(1);
	if(size == 3) y = (uint8_t)msg.getInt(2);
	

	if(nowSetting == SET_STATE){
		if(size == 3){
			if(state == 0){
				board.clrLed(x, y);
			} else {
				board.setLed(x, y);
			}
		} else if (size == 2){
			if(state == 0){
				board.clrLed(x);
			} else {
				board.setLed(x);
			}
		}
	} else if(nowSetting == SET_INT){
		if(size == 3){
			board.setBrightness(x, y, state);
		} else if(size == 2){
			board.setBrightness(x, state);
		}
	} else if(nowSetting == SET_RGB){
		state &= 0b00111111;
		if(size == 3){
			state |= (board.getBrightness(x, y) << 6);
			board.setColor(x, y, state);
		} else if(size == 2){
			state |= (board.getBrightness(x) << 6);
			board.setColor(x, state);
		}		
	}
}

void ledRow(OSCMessage &msg, int offset){
	uint8_t maxSize = board.getSizeX();


	uint8_t size = msg.size();

	uint8_t index, start, length, end = 0;
	uint8_t state = 0;

	state = (uint8_t)msg.getInt(0);
	index = (uint8_t)msg.getInt(1);

	if(size == 4){
		start = (uint8_t)msg.getInt(2);
		length = (uint8_t)msg.getInt(3);
		end = start + length;
		if(end > maxSize) end = maxSize;
	} else {
		start = 0;
		end = maxSize;
	}

	if(nowSetting == SET_STATE){
		for(uint8_t i = start; i < end; ++i){
			if(state == 0){
				board.clrLed(i, index);
			} else {
				board.setLed(i, index);
			}
		}
	} else if(nowSetting == SET_INT){
		for(uint8_t i = start; i < end; ++i){
			board.setBrightness(i, index, state);
		}

	} else if(nowSetting == SET_RGB){
		for(uint8_t i = start; i < end; ++i){
			state &= 0b00111111;
			state |= (board.getBrightness(i, index) << 6);
			board.setColor(i, index, state);
		}
	}
}

void ledCol(OSCMessage &msg, int offset){
	uint8_t maxSize = board.getSizeY();


	uint8_t size = msg.size();

	uint8_t index, start, length, end = 0;
	uint8_t state = 0;

	state = (uint8_t)msg.getInt(0);
	index = (uint8_t)msg.getInt(1);

	if(size == 4){
		start = (uint8_t)msg.getInt(2);
		length = (uint8_t)msg.getInt(3);
		end = start + length;
		if(end > maxSize) end = maxSize;
	} else {
		start = 0;
		end = maxSize;
	}

	if(nowSetting == SET_STATE){
		for(uint8_t i = start; i < end; ++i){
			if(state == 0){
				board.clrLed(index, i);
			} else {
				board.setLed(index, i);
			}
		}
	} else if(nowSetting == SET_INT){
		for(uint8_t i = start; i < end; ++i){
			board.setBrightness(index, i, state);
		}

	} else if(nowSetting == SET_RGB){
		for(uint8_t i = start; i < end; ++i){
			state &= 0b00111111;
			state |= (board.getBrightness(index, i) << 6);
			board.setColor(index, i, state);
		}
	}
}

void ledMap(OSCMessage &msg, int offset){
	uint8_t maxSizeX = board.getSizeX();
	uint8_t maxSizeY = board.getSizeY();


	uint8_t size = msg.size();

	uint8_t index, startX, startY, lengthX, lengthY, endX, endY = 0;
	uint8_t state = 0;

	state = (uint8_t)msg.getInt(0);
	startX = (uint8_t)msg.getInt(1);
	startY = (uint8_t)msg.getInt(2);
	lengthX = (uint8_t)msg.getInt(3);
	lengthY = (uint8_t)msg.getInt(4);

	endX = startX + lengthX;
	if(endX > maxSizeX) endX = maxSizeX;

	endY = startY + lengthY;
	if(endY > maxSizeY) endY = maxSizeY;

	if(nowSetting == SET_STATE){
		for(uint8_t x = startX; x < endX; ++x){
			for(uint8_t y = startY; y < endY; ++y){
				if(state == 0){
					board.clrLed(x, y);
				} else {
					board.setLed(x, y);
				}
			}
		}
	} else if(nowSetting == SET_INT){
		for(uint8_t x = startX; x < endX; ++x){
			for(uint8_t y = startY; y < endY; ++y){
				board.setBrightness(x, y, state);
			}
		}

	} else if(nowSetting == SET_RGB){
		for(uint8_t x = startX; x < endX; ++x){
			for(uint8_t y = startY; y < endY; ++y){
				state &= 0b00111111;
				state |= (board.getBrightness(x, y) << 6);
				board.setColor(x, y, state);
			}
		}
	}
}

void ledAll(OSCMessage &msg, int offset){
	uint16_t boardSize = board.getSizeX() * board.getSizeY();

	uint8_t state;

	state = (uint8_t)msg.getInt(0);

	if(nowSetting == SET_STATE){
		for(uint16_t i = 0; i < boardSize; ++i){
			if(state == 0){
				board.clrLed(i);
			} else {
				board.setLed(i);
			}
		}
	} else if(nowSetting == SET_INT){
		for(uint16_t i = 0; i < boardSize; ++i){
			board.setBrightness(i, state);
		}
	} else if(nowSetting == SET_RGB){
		state &= 0b00111111;
		state |= (board.getBrightness(0) << 6);
		for(uint16_t i = 0; i < boardSize; ++i){
			board.setColor(i, state);
		}
	}
}


void sendSize(OSCMessage &msg, int offset){
 	OSCMessage messageOut = OSCMessage("/size");
 	messageOut.add((int32_t)board.getSizeX());
 	messageOut.add((int32_t)board.getSizeY());

 	SLIPSerial.beginPacket();
	messageOut.send(SLIPSerial);
	SLIPSerial.endPacket();
}

void setKeyMode(OSCMessage &msg){
	uint8_t size = msg.getDataLength(0);
	char command[size];

	msg.getString(0, command, size);

	if(strcmp(command, "pos") == 0){
		keymode = KEY_BY_POS;
	} else if(strcmp(command, "index") == 0){
		keymode = KEY_BY_INDEX;
	}
}

void startSequence(OSCMessage &msg){

	uint16_t maxSize = board.getSizeX() * board.getSizeY();

	for(int8_t i = -4; i < (int16_t)(maxSize + 4); ++i){

		for(int8_t j = 0; j < maxSize; ++j){

			int8_t delta = abs(i - j);

			if(delta < 4){
				board.setBrightness((uint8_t)j, (uint8_t)(3 - delta));
				board.setLed((uint8_t)j);
			} else {
				board.clrLed((uint8_t)j);
			}
		}
			board.updateLeds();
			board.updateDisplay();
			delay(10);		
	}

}