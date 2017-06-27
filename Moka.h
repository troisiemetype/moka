/*
 * Moka is a board that manage 16 RGB leds and 16 soft buttons as an user interface with visual feedback.
 * It can be tiled up to 32 modules, each with a hardware set I2Caddress
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

#ifndef MOKA_H
#define MOKA_H

#include <Arduino.h>

#include <Wire.h>

#define COLOR_MODE_8        0;
#define COLOR_MODE_24       1;

#define COLOR_MODE          COLOR_MODE_8;

class Moka{
public:

	enum I2C_REG{
		SET_ONE_LED  =0x00,					// SET_ONE_LED | LedNumber + 1/3 bytes
		SET_GLOBAL_LED = 0x10,				// SET_GLOBAL + 1/3 bytes
		SET_ALL_LED = 0x20,					// SET_ALL + 16/72 bytes
		GET_BUTTONS = 0x40,					// GET_BUTTONS + 2 bytes from slave to master
		LED_STATE = 0x50,					// LED_STATE + 2 byte

		DISPLAY_STATE = 0x60,				// DISPLAY_STATE | State

		DEBOUNCE_DELAY = 0x82,				// DEBOUNCE_DELAY + 1 byte

		HAS_CHANGED = 0x83,					// HAS_CHANGED + 1 byte from slave to master // INT emitted

		COLOR_MODE = 0x84,					// COLOR_MODE | mode

		CLR_DISPLAY = 0xF0,					// CLR_DISPLAY
		UPDATE_DISPLAY = 0xF5,				// UPDATE_DISPLAY

		RESET = 0xFF,						// RESET
	};

    void begin(uint8_t address, bool fast = false);
    void beginFast(uint8_t address);

    void setLed(uint8_t index);
    void clrLed(uint8_t index);
    bool isLed(uint8_t index);

    void setColor(uint8_t index, uint8_t color);
    void setLedBrightness(uint8_t index, uint8_t brightness);

    void setGlobalColor(uint8_t color);

    void update();

    void readButtons();

    bool isPressed(uint8_t index);
    bool wasPresed(uint8_t index);
    bool isJustPressed(uint8_t index);
    bool isJustReleased(uint8_t index);

    void displayOn();
    void displayOff();
    void clrDisplay();

    void setDebounce(uint8_t delay);

    bool testInt();

    void reset();

protected:
    uint8_t _led[16][3];

    uint16_t _ledState;

    uint16_t _buttons, _prevButtons;




private:
    uint8_t _i2cAddress;
    bool _mustUpdate;

};

#endif