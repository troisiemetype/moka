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

class Moka{
public:

	enum I2C_REG{
		SET_ONE_LED = 0x00,					// SET_ONE_LED | LedNumber + 1/3 bytes
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

		COLOR_MODE_8 = 0x00,
		COLOR_MODE_24 = 0x01,
	};

	enum I2C_SPEED{
		I2C_100 = 0,
		I2C_400 = 1,
	};

    void begin(uint8_t address, bool fast = false);
    void beginFast(uint8_t address);

    void setLed(uint8_t index);
    void setLed(uint8_t col, uint8_t row);
    void clrLed(uint8_t index);
    void clrLed(uint8_t col, uint8_t row);
    bool isLed(uint8_t index);
    bool isLed(uint8_t col, uint8_t row);

    void setColor(uint8_t index, uint8_t color);
    void setColor(uint8_t col, uint8_t row, uint8_t color);
    void setBrightness(uint8_t index, uint8_t brightness);
    void setBrightness(uint8_t col, uint8_t row, uint8_t brightness);

    uint8_t getColor(uint8_t index);
    uint8_t getColor(uint8_t col, uint8_t row);
    uint8_t getBrightness(uint8_t index);
    uint8_t getBrightness(uint8_t col, uint8_t row);

    void setGlobalColor(uint8_t color);

    void updateLeds();
    void updateDisplay();

    void readButtons();

    bool isPressed(uint8_t index);
    bool isPressed(uint8_t col, uint8_t row);
    bool wasPressed(uint8_t index);
    bool wasPressed(uint8_t col, uint8_t row);
    bool isJustPressed(uint8_t index);
    bool isJustPressed(uint8_t col, uint8_t row);
    bool isJustReleased(uint8_t index);
    bool isJustReleased(uint8_t col, uint8_t row);

    void displayOn();
    void displayOff();
    void clrDisplay();

    void setDebounce(uint8_t delay);

    bool testInt();

    void reset();

protected:

	inline uint8_t indexToCol(uint8_t index){return (index % 4);}
	inline uint8_t indexToRow(uint8_t index){return (index / 4);}
	inline uint8_t posToIndex(uint8_t col, uint8_t row){return (row * 4 + col);}

    uint8_t _led[16];

    uint16_t _ledState;

    uint16_t _buttons, _prevButtons;

private:
    uint8_t _i2cAddress;
    uint16_t _update;

};

class Mokas{
public:

    bool begin(uint8_t cols, uint8_t rows);
    bool add(Moka *board);
    bool beginAuto(uint8_t cols, uint8_t rows, bool fast = false);

    void setLed(uint16_t index);
    void setLed(uint8_t col, uint8_t row);
    void clrLed(uint16_t index);
    void clrLed(uint8_t col, uint8_t row);
    bool isLed(uint16_t index);
    bool isLed(uint8_t col, uint8_t row);

    void setColor(uint16_t index, uint8_t color);
    void setColor(uint8_t col, uint8_t row, uint8_t color);
    void setBrightness(uint16_t index, uint8_t brightness);
    void setBrightness(uint8_t col, uint8_t row, uint8_t brightness);

    uint8_t getColor(uint16_t index);
    uint8_t getColor(uint8_t col, uint8_t row);
    uint8_t getBrightness(uint16_t index);
    uint8_t getBrightness(uint8_t col, uint8_t row);

    void setGlobalColor(uint8_t color);

    void updateLeds();
    void updateDisplay();

    void readButtons();

    bool isPressed(uint16_t index);
    bool isPressed(uint8_t col, uint8_t row);
    bool wasPressed(uint16_t index);
    bool wasPressed(uint8_t col, uint8_t row);
    bool isJustPressed(uint16_t index);
    bool isJustPressed(uint8_t col, uint8_t row);
    bool isJustReleased(uint16_t index);
    bool isJustReleased(uint8_t col, uint8_t row);

    void displayOn();
    void displayOff();
    void clrDisplay();

    void setDebounce(uint8_t delay);

    bool testInt();

    void reset();

protected:

	uint8_t indexToCol(uint16_t index);
	uint8_t indexToRow(uint16_t index);
	uint16_t posToIndex(uint8_t col, uint8_t row);

	uint8_t indexToBoardCol(uint16_t index);
	uint8_t indexToBoardRow(uint16_t index);
	uint8_t indexToBoard(uint16_t index);
	uint8_t indexToBoardButton(uint16_t index);
	uint8_t indexToBoardButtonCol(uint16_t index);
	uint8_t indexToBoardButtonRow(uint16_t index);

private:
	Moka *_boards[32];

	uint8_t _nbBoards, _nbCol, _nbRow;
	uint8_t _addBoard;



};

#endif