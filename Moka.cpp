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


void Moka::begin(uint8_t address, bool fast){
	_i2cAddress = address;
	Wire.begin();
	if(fast){
		Wire.setClock(400000L);
	}

	displayOn();
}

void Moka::beginFast(uint8_t address){
	begin(true);
}

void Moka::setLed(uint8_t index){
	if(index > 15) return;

	_ledState |= _BV(index);
	_mustUpdate = true;

	Wire.beginTransmission(_i2cAddress);
	Wire.write(LED_STATE);
	Wire.write(_ledState >> 8);
	Wire.write(_ledState);
	Wire.endTransmission();
}

void Moka::clrLed(uint8_t index){
	if(index > 15) return;

	_ledState &= ~_BV(index);
	_mustUpdate = true;

	Wire.beginTransmission(_i2cAddress);
	Wire.write(LED_STATE);
	Wire.write(_ledState >> 8);
	Wire.write(_ledState);
	Wire.endTransmission();
}

bool Moka::isLed(uint8_t index){
	if(index > 15) return false;

	return (_ledState & _BV(index));
}

void Moka::setColor(uint8_t index, uint8_t color){
	if(index > 15) return;

	_led[index][0] = color;
	_mustUpdate = true;

	Wire.beginTransmission(_i2cAddress);
	Wire.write(SET_ONE_LED | index);
	Wire.write(color);
	Wire.endTransmission();
}

void Moka::setLedBrightness(uint8_t index, uint8_t brightness){
	if(index > 15) return;
	if(brightness > 3) return;
	_mustUpdate = true;

	_led[index][0] &= 0x3F;
	_led[index][0] |= (brightness << 6);

	Wire.beginTransmission(_i2cAddress);
	Wire.write(SET_ONE_LED | index);
	Wire.write(color);
	Wire.endTransmission();
}

void Moka::setGlobalColor(uint8_t color){
	for(uint8_t i = 0; i < 16; i++){
		_led[i][0] = color;
	}
	_mustUpdate = true;

	Wire.beginTransmission(_i2cAddress);
	Wire.write(SET_GLOBAL_LED);
	Wire.write(color);
	Wire.endTransmission();	
}

void Moka::update(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(UPDATE_DISPLAY);
	Wire.endTransmission();

	_mustUpdate = false;
}

void Moka::readButtons(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(GET_BUTTONS);
	Wire.endTransmission();

	Wire.requestFrom(_i2cAddress, 2);
	_prevButtons = _buttons;
	_buttons = (Wire.read() << 8);
	_buttons |= Wire.read();
}

bool Moka::isPressed(uint8_t index){
	if(index > 15) return false;

	return (_buttons & _BV(index));

}

bool Moka::wasPressed(uint8_t index){
	if(index > 15) return false;

	return (_prevButtons & _BV(index));
}

bool Moka::isJustPressed(uint8_t index){
	if(index > 15) return false;

	return (isPressed(index) && !wasPressed(index));
}

bool Moka::isJustReleased(uint8_t index){
	if(index > 15) return false;

	return (!isPressed(index) && wasPressed(index));
}

void Moka::displayOn(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(DISPLAY_STATE | 1);
	Wire.endTransmission();
}

void Moka::displayOff(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(DISPLAY_STATE | 0);
	Wire.endTransmission();
}

void Moka::clrDisplay(){
	for(uint8_t i = 0; i < 16; i++){
		_led[i][0] = 0;
	}

	Wire.beginTransmission(_i2cAddress);
	Wire.write(CLR_DISPLAY);
	Wire.endTransmission();
}

void Moka::setDebounce(uint8_t delay){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(DEBOUNCE_DELAY);
	Wire.write(delay);
	Wire.endTransmission();
}

bool Moka::testInt(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(HAS_CHANGED);
	Wire.endTransmission();

	Wire.requestFrom(_i2cAddress, 1);
	return Wire.read();
}

void Moka::reset(){
	Wire.beginTransmission(_i2cAddress);
	Wire.write(RESET);
	Wire.endTransmission();
}