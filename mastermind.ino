/*
 * An Arduino clone of the board game Mastermind
 * Copyright (C) 2020  Jacob Koziej, Matthew Bilik
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Adafruit_NeoPixel.h>

/* NEOPIXEL CONFIGURATION */
#define STRIP_PORT 3
#define PIX_COUNT 85
Adafruit_NeoPixel STRIP = Adafruit_NeoPixel(PIX_COUNT, STRIP_PORT, NEO_GRB + NEO_KHZ800);

// Variables to address individual NeoPixels
const int CODE_PEGS[][4] = {
	{0, 10, 20, 30},
	{1, 11, 21, 31},
	{2, 12, 22, 32},
	{3, 13, 23, 33},
	{4, 14, 24, 34},
	{5, 15, 25, 35},
	{6, 16, 26, 36},
	{7, 17, 27, 37},
	{8, 18, 28, 38},
	{9, 19, 29, 39}
};
const int KEY_PEGS[][4] = {
	{40, 50, 60, 70},
	{41, 51, 61, 71},
	{42, 52, 62, 72},
	{43, 53, 63, 73},
	{44, 54, 64, 74},
	{45, 55, 65, 75},
	{46, 56, 66, 76},
	{47, 57, 67, 77},
	{48, 58, 68, 78},
	{49, 59, 69, 79},
};
const int SECRET_CODE[] = {80, 81, 82, 83};
#define INPUT_NEOPIXEL 84

/* PLAYER INPUTS */
#define POTENTIOMETER A0

void setup() {
	STRIP.begin();
	STRIP.show();

	pinMode(POTENTIOMETER, INPUT);
}

void loop() {
}
