/*
 * Moka is a board that manage 16 RGB leds and 16 soft buttons as an user interface with visual feedback.
 * Copyright 2017 - Pierre-Loup Martin / le labo du troisième
 *
 * This program is part of Moka.
 *
 * Moka is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Infos:
 * https://www.lelabodutroisieme.fr
 * https://www.github.com/troisiemetype/mokaboard for hardware reference
 * https://www.github.com/troisiemetype/mokafirmware for software reference
 * https://www.github.com/troisiemetype/moka for user library
 */

#include "Moka.h"

//Moka class: managing one Moka tile.

// Set the new tile: create its address and the I2C bus speed.
// Speed default to 100KHz
void Moka::begin(uint8_t address, bool fast){
	_i2cAddress = address;
	Wire.begin();
	if(fast){
		Wire.setClock(200000L);
	}

	displayOn();
	
	Wire.beginTransmission(_i2cAddress);
	Wire.write(COLOR_MODE | COLOR_MODE_8);
	Wire.endTransmission();

	_update = 0;
}

// Set the new tile with a bus speed of 400000Hz
void Moka::beginFast(uint8_t address){
	begin(address, true);
}

// LED settings: All this methods apply both to class tables and to Moka tile.
// You will have to call update() for these settings to take effect.

// Set a led to be lit. This doesn't change its color (nor its brightness)
void Moka::setLed(uint8_t index){
	if(index > 15) return;

	_ledState |= _BV(index);
	_update |= _BV(index);
}

void Moka::setLed(uint8_t col, uint8_t row){
	setLed(posToIndex(col, row));
}

// Shut a led. This doesn't chaged its color
void Moka::clrLed(uint8_t index){
	if(index > 15) return;

	_ledState &= ~_BV(index);
	_update |= _BV(index);
}

void Moka::clrLed(uint8_t col, uint8_t row){
	clrLed(posToIndex(col, row));
}

// Get the led state, i.e. lit or shut.
bool Moka::isLed(uint8_t index) const{
	if(index > 15) return false;

	return (_ledState & _BV(index));
}

bool Moka::isLed(uint8_t col, uint8_t row) const{
	return isLed(posToIndex(col, row));
}

// Set the led color. The color has to be formatted as 0bAARRGGBB, A beeing the alpha channel
void Moka::setColor(uint8_t index, uint8_t color){
	if(index > 15) return;

	_led[index] = color;
	_update |= _BV(index);
}

void Moka::setColor(uint8_t col, uint8_t row, uint8_t color){
	setColor(posToIndex(col, row), color);
}

// Set the led brightness. This is a convenient function that just change the top two bits of its color.
void Moka::setBrightness(uint8_t index, uint8_t brightness){
	if(index > 15) return;
	if(brightness > 3) return;
	_update |= _BV(index);

	_led[index] &= 0x3F;
	_led[index] |= (brightness << 6);
}

void Moka::setBrightness(uint8_t col, uint8_t row, uint8_t brightness){
	setBrightness(posToIndex(col, row), brightness);
}

// Get the color set to this led.
uint8_t Moka::getColor(uint8_t index) const{
	return _led[index];
}

uint8_t Moka::getColor(uint8_t col, uint8_t row) const{
	return getColor(posToIndex(col, row));
}

// Get the brightness attached to this led.
// This returns the two top bits of color, right aligned: 0b01xxxxxx will return 3 (and not 0b01000000)
uint8_t Moka::getBrightness(uint8_t index) const{
	return ((_led[index] & 0xC0) >> 6);
}

uint8_t Moka::getBrightness(uint8_t col, uint8_t row) const{
	return getBrightness(posToIndex(col, row));
}

// Set a color for the whole panel.
// This is the same as calling setColor on each led with the same color, except you only call it once,
// And only one command is sent trough I2C.
void Moka::setGlobalColor(uint8_t color){
	for(uint8_t i = 0; i < 16; i++){
		_led[i] = color;
	}
	_update = 0xFF;	
}

// Update the leds, i.e. send the new led values to the display.
// First send the led state (lit or shut)
// Then, following the number of led to update, send the whole panel, or one led after the other.
// This way communication is reduced to the minimum.
void Moka::updateLeds(){
//	Serial.println(_update, BIN);
	if(_update == 0) return;

	uint8_t ok = 0;

//	Serial.print("led state \t");
//	Serial.println(ok);

	uint8_t qty = 0;
	for(uint8_t i = 0; i < 16; i++){
		if((bool)(_update & _BV(i))){
			++qty;
//			Serial.print("update led\t");
//			Serial.print(i);
		}
	}


	// Arduino limits the send buffer to 32 bytes, so this needs to be fixed for the 24 color mode.
	if(qty > 7){
		Wire.beginTransmission(_i2cAddress);
		Wire.write(SET_ALL_LED);
		for(uint8_t i = 0; i < 16; i++){
			Wire.write(_led[i]);
		}
		ok = Wire.endTransmission();
//		Serial.print("all leds \t");
//		Serial.println(ok);

	} else {
		Wire.beginTransmission(_i2cAddress);
		for(uint8_t i = 0; i < 16; i++){
			if(_update & _BV(i)){
				Wire.write(SET_ONE_LED | i);
				Wire.write(_led[i]);
				ok = Wire.endTransmission(false);
//				Serial.print("led ");
//				Serial.print(i);
//				Serial.print("\t");
//				Serial.println(ok);

			}
		}
		Wire.endTransmission();
	}

	Wire.beginTransmission(_i2cAddress);
	Wire.write(LED_STATE);
	Wire.write((_ledState >> 8));
	Wire.write(_ledState & 0xFF);
	Wire.endTransmission(false);


//	Serial.println();

	_update = 0;

}

// Update display with fresh led values.
// This is separated from the led update, so all leds can be updated,
// and once done the display are all updated at the same time.
void Moka::updateDisplay() const{
	Wire.beginTransmission(_i2cAddress);
	Wire.write(UPDATE_DISPLAY);
	Wire.endTransmission();
}


// Buttons methods. These return the values stored in class table.
// You have to call readButtons() to get fresh values from the Moka tile.

// Get a read of the buttons from the panel.
bool Moka::readButtons(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(GET_BUTTONS);
	uint8_t ok = Wire.endTransmission();
//	Serial.print("buttons asked \t");
//	Serial.println(ok);

	ok = Wire.requestFrom(_i2cAddress, 2);
	
	if(ok == 2){
//		Serial.print("bytes returned: ");
//		Serial.println(ok);
		_prevButtons = _buttons;
		_buttons = ((uint16_t)Wire.read() << 8);
		_buttons |= Wire.read();
//		Serial.print("buttons:\t");
//		Serial.println(_buttons, BIN);

	} else {
//		Serial.println("com pb");
		return false;
	}

	if(_prevButtons != _buttons){
		return true;
	} else {
		return false;
	}
}

// Return true if the given button is being pressed.
bool Moka::isPressed(uint8_t index) const{
	if(index > 15) return false;

	return (_buttons & _BV(index));
}

bool Moka::isPressed(uint8_t col, uint8_t row) const{
	return isPressed(posToIndex(col, row));
}

// Return true if the button was pressed on last reading, but is not
bool Moka::wasPressed(uint8_t index) const{
	if(index > 15) return false;

	return (_prevButtons & _BV(index));
}

bool Moka::wasPressed(uint8_t col, uint8_t row) const{
	return wasPressed(posToIndex(col, row));
}

// Return true if the button has been newly pressed.
bool Moka::isJustPressed(uint8_t index) const{
	if(index > 15) return false;

	return (isPressed(index) && !wasPressed(index));
}

bool Moka::isJustPressed(uint8_t col, uint8_t row) const{
	return isJustPressed(posToIndex(col, row));
}

// Return true if the button has been newly released.
bool Moka::isJustReleased(uint8_t index) const{
	if(index > 15) return false;

	return (!isPressed(index) && wasPressed(index));
}

bool Moka::isJustReleased(uint8_t col, uint8_t row) const{
	return isJustReleased(posToIndex(col, row));
}

// Set the display on, independently of led values. update() has to be called after.
void Moka::displayOn(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(DISPLAY_STATE | 1);
	Wire.endTransmission();
}

// Set the display off, independently of led values. update() has to be called.
void Moka::displayOff(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(DISPLAY_STATE | 0);
	Wire.endTransmission();
}

// Clear all the led values on the display. Update() has to be called too.
void Moka::clrDisplay(){
	for(uint8_t i = 0; i < 16; i++){
		_led[i] = 0;
	}

	Wire.beginTransmission(_i2cAddress);
	Wire.write(CLR_DISPLAY);
	Wire.endTransmission();
}

// Set the debounce delay for buttons.
void Moka::setDebounce(uint8_t delay) const{
	Wire.beginTransmission(_i2cAddress);
	Wire.write(DEBOUNCE_DELAY);
	Wire.write(delay);
	Wire.endTransmission();
}

// Ask to the board if it has signalled an INT.
bool Moka::testInt(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(HAS_CHANGED);
	Wire.endTransmission();

	Wire.requestFrom(_i2cAddress, (uint8_t)1);
	return Wire.read();
}

// Reset the board. has to be seen if it's possible. Seems not.
void Moka::reset() const{
	Wire.beginTransmission(_i2cAddress);
	Wire.write(RESET);
	Wire.endTransmission();
}



/////////////////////////////////////////////////
// Mokas class: managing several board together//
/////////////////////////////////////////////////


// Create a big board out of several tiles. Cols and rows
bool Mokas::begin(uint8_t cols, uint8_t rows){
	_nbCol = cols;
	_nbRow = rows;
	if(rows == 0) return true;
	if(cols == 0) return true;
	_sizeX = cols * 4;
	_sizeY = rows * 4;
	_nbBoards = cols * rows;
	_addBoard = 0;
	if(_nbBoards > 32) return true;

	return false;
}

// Add a tile to the board. Will return true until we have reach the number of board defined with begin()
bool Mokas::add(Moka *board){
	if(_addBoard >= _nbBoards) return true;
	_boards[_addBoard] = board;
	++_addBoard;
	return false;
}

// Creates a matrix of cols columns by rows column, and automaticly creates boards that compose it.
// It uses the default address, so if you use it to have to set the physical addresses on the boards
// crescent for left to right, and up to down. The first board has the address 10, i.e. no jumper set.
bool Mokas::beginAuto(uint8_t cols, uint8_t rows, bool fast){
	bool status = begin(cols, rows);
	if(status) return status;
	uint8_t address = 10;
	for(uint8_t i = 0; i < _nbBoards; i++){
		Moka *board = new Moka();
		_boards[i] = board;
		board->begin(address + i, fast);
	}

	return false;
}

void Mokas::setLed(uint16_t index){
	_boards[indexToBoard(index)]->setLed(indexToBoardButton(index));
}

void Mokas::setLed(uint8_t col, uint8_t row){
	setLed(posToIndex(col, row));
}

void Mokas::clrLed(uint16_t index){
	_boards[indexToBoard(index)]->clrLed(indexToBoardButton(index));
}

void Mokas::clrLed(uint8_t col, uint8_t row){
	clrLed(posToIndex(col, row));
}

bool Mokas::isLed(uint16_t index) const{
	return _boards[indexToBoard(index)]->isLed(indexToBoardButton(index));
}

bool Mokas::isLed(uint8_t col, uint8_t row) const{
	return isLed(posToIndex(col, row));
}

void Mokas::setColor(uint16_t index, uint8_t color){
	_boards[indexToBoard(index)]->setColor(indexToBoardButton(index), color);
}

void Mokas::setColor(uint8_t col, uint8_t row, uint8_t color){
	setColor(posToIndex(col, row), color);
}

void Mokas::setBrightness(uint16_t index, uint8_t brightness){
	_boards[indexToBoard(index)]->setBrightness(indexToBoardButton(index), brightness);
}

void Mokas::setBrightness(uint8_t col, uint8_t row, uint8_t brightness){
	return setBrightness(posToIndex(col, row), brightness);
}


uint8_t Mokas::getColor(uint16_t index) const{
	return _boards[indexToBoard(index)]->getColor(indexToBoardButton(index));
}

uint8_t Mokas::getColor(uint8_t col, uint8_t row) const{
	return getColor(posToIndex(col, row));
}

uint8_t Mokas::getBrightness(uint16_t index) const{
	return _boards[indexToBoard(index)]->getBrightness(indexToBoardButton(index));
}

uint8_t Mokas::getBrightness(uint8_t col, uint8_t row) const{
	return getBrightness(posToIndex(col, row));
}


void Mokas::setGlobalColor(uint8_t color){
	for(uint8_t i = 0; i < _nbBoards; i++){
		_boards[i]->setGlobalColor(color);
	}
}


void Mokas::updateLeds(){
	for(uint8_t i = 0; i < _nbBoards; i++){
		_boards[i]->updateLeds();
	}
}

void Mokas::updateDisplay() const{
	Wire.beginTransmission(0);
	Wire.write(Moka::UPDATE_DISPLAY);
	Wire.endTransmission();
}


void Mokas::readButtons(){
	for(uint8_t i = 0; i < _nbBoards; i++){
		_boards[i]->readButtons();
	}
}


bool Mokas::isPressed(uint16_t index) const{
	return _boards[indexToBoard(index)]->isPressed(indexToBoardButton(index));
}

bool Mokas::isPressed(uint8_t col, uint8_t row) const{
	return isPressed(posToIndex(col, row));
}

bool Mokas::wasPressed(uint16_t index) const{
	return _boards[indexToBoard(index)]->wasPressed(indexToBoardButton(index));
}

bool Mokas::wasPressed(uint8_t col, uint8_t row) const{
	return wasPressed(posToIndex(col, row));
}

bool Mokas::isJustPressed(uint16_t index) const{
	return _boards[indexToBoard(index)]->isJustPressed(indexToBoardButton(index));
}

bool Mokas::isJustPressed(uint8_t col, uint8_t row) const{
	return isJustPressed(posToIndex(col, row));
}

bool Mokas::isJustReleased(uint16_t index) const{
	return _boards[indexToBoard(index)]->isJustReleased(indexToBoardButton(index));
}

bool Mokas::isJustReleased(uint8_t col, uint8_t row) const{
	return isJustReleased(posToIndex(col, row));
}

void Mokas::displayOn(){
	Wire.beginTransmission(0);
	Wire.write(Moka::DISPLAY_STATE | 1);
	Wire.endTransmission();
}

void Mokas::displayOff(){
	Wire.beginTransmission(0);
	Wire.write(Moka::DISPLAY_STATE | 0);
	Wire.endTransmission();
}

void Mokas::clrDisplay(){
	for(uint8_t i = 0; i < _nbBoards; i++){
		_boards[i]->setGlobalColor(0);
	}
}

void Mokas::setDebounce(uint8_t delay) const{
	for(uint8_t i = 0; i < _nbBoards; i++){
		_boards[i]->setDebounce(delay);
	}
}

bool Mokas::testInt(){
	return true;
}

void Mokas::reset() const{

}


// convenience functions to convert index to position and position to index.
uint8_t Mokas::indexToCol(uint16_t index) const{
	return (index % _nbCol);
}

uint8_t Mokas::indexToRow(uint16_t index) const{
	return (index / _nbCol);
}

uint16_t Mokas::posToIndex(uint8_t col, uint8_t row) const{
	return (uint16_t)(col + row * 4);
}


uint8_t Mokas::indexToBoardCol(uint16_t index) const{
	return (indexToCol(index) / 4);
}

uint8_t Mokas::indexToBoardRow(uint16_t index) const{
	return (indexToRow(index) / 4);
}

uint8_t Mokas::indexToBoard(uint16_t index) const{
	return (indexToBoardCol(index) + indexToBoardRow(index) * _nbCol);
}

uint8_t Mokas::indexToBoardButtonCol(uint16_t index) const{
	return (indexToCol(index) - 4 * indexToBoardCol(index));
}

uint8_t Mokas::indexToBoardButtonRow(uint16_t index) const{
	return (indexToRow(index) - 4 * indexToBoardRow(index));
}
