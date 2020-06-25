/*
 * An Arduino clone of the board game Mastermind
 * Copyright (C) 2020  Jacob Koziej
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
#include <LiquidCrystal.h>

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
const int PUSH_BUTTON[] = {A1, A2, A3, A4, A5};

/* LCD CONFIGURATION */
/************** rs, e, d4, d5, d6, d7 */
LiquidCrystal LCD(13, 12, 8, 7, 4, 2);

/* GAME COLORS */
const int GAME_COLORS[][3] = {
	{255, 255, 255}, // WHITE
	{255, 000, 000}, // RED
	{255, 165, 000}, // ORANGE
	{255, 255, 000}, // YELLOW
	{000, 255, 000}, // GREEN
	{000, 000, 255}, // BLUE
	{128, 000, 128}, // PURPLE
	{000, 000, 000}, // NONE
};

/* GAME COLOR STORAGE */
int code_peg_color[10][4], key_peg_color[10][4], secret_code_color[4];

/* GLOBAL VARIABLES */
int current_row;
int difficulty;
int input[4];
int player_count;

void setup() {
	STRIP.begin();
	STRIP.show();

	LCD.begin(16, 2);

	pinMode(POTENTIOMETER, INPUT);
	for (int i; i <= 4; i++) {
		pinMode(PUSH_BUTTON[i], INPUT);
	}

	Serial.flush();
	Serial.begin(9600);
}

void loop() {
	bootstrap();
}

// Returns range of 0 to (items - 1)
int potentiometerSelection(int items) {
	int output = map(analogRead(POTENTIOMETER), 0, 1023, 0, items);

	/*
	 * For some reason the map function has a quirk where the maximum 
	 * value is only mapped at the upper bound of the input (1023).
	 * This is the work-around for this quirk.
	 */
	if (output != items) {
		return output;
	} else {
		return output - 1;
	}
}

void updateIndicator(int color) {
	STRIP.setPixelColor(INPUT_NEOPIXEL, GAME_COLORS[color][0], GAME_COLORS[color][1], GAME_COLORS[color][2]);
	STRIP.show();
}

void updateSecretCode(int color0, int color1, int color2, int color3) {
	int temp[] = {color0, color1, color2, color3};
	for (int i = 0; i <= 3; i++) {
		STRIP.setPixelColor((80 + i), GAME_COLORS[temp[i]][0], GAME_COLORS[temp[i]][1], GAME_COLORS[temp[i]][2]);
	}
	STRIP.show();
}

void updateAnyNeoPixel(int pixel, int color) {
	STRIP.setPixelColor(pixel, GAME_COLORS[color][0], GAME_COLORS[color][1], GAME_COLORS[color][2]);
	STRIP.show();
}

// Return the state of push buttons 0 to 4
int buttonState(int button) {
	return digitalRead(PUSH_BUTTON[button]);
}

void bootstrap() {
	Serial.println("-----BEGIN BOOTSTRAP-----");

	// Print MASTERMIND in top right of LCD
	LCD.clear();
	LCD.rightToLeft();
	LCD.setCursor(15, 0);
	LCD.print("DNIMRETSAM");

	LCD.leftToRight();
	LCD.setCursor(0, 1);
	LCD.print("Press any button");

	// Wait for player to press a button before moving on
	int game_start = 0;
	while (game_start == 0) {
		for (int i = 0; i <= 4; i++) {
			int push_button_state = digitalRead(PUSH_BUTTON[i]);

			if (push_button_state == 1) {
				do {
					push_button_state = digitalRead(PUSH_BUTTON[i]);
				} while (push_button_state == 1);

				game_start = 1;
				break;
			}
		}
	}

	Serial.print("\nEvent: ");
	Serial.println("player initiated game");

	// Set how many players will be playing the game
	LCD.clear();
	LCD.print("Player count:");
	LCD.setCursor(0, 1);
	LCD.print("[==============]");
	LCD.rightToLeft();

	int current_selection, previous_selection = !potentiometerSelection(2);

	while (1) {
		int push_button_state = digitalRead(PUSH_BUTTON[4]);
		current_selection = potentiometerSelection(2);

		if (current_selection != previous_selection) {
			LCD.setCursor(15, 0);

			if (current_selection == 0) {
				LCD.print("1");
			} else {
				LCD.print("2");
			}

			previous_selection = current_selection;
		}

		if (push_button_state == 1) {
			do {
				push_button_state = digitalRead(PUSH_BUTTON[4]);
			} while (push_button_state == 1);

			player_count = (current_selection + 1);
			break;
		}
	}

	Serial.print("Player Count: ");
	Serial.println(player_count);

	// Set game difficulty
	LCD.clear();
	LCD.print("Game Difficulty:");
	LCD.rightToLeft();

	previous_selection = -1; // prime the forever loop

	while (1) {
		int push_button_state = digitalRead(PUSH_BUTTON[4]);
		current_selection = potentiometerSelection(3);

		if (current_selection != previous_selection) {
			LCD.setCursor(15, 1);

			switch (current_selection) {
				case 0:
					LCD.print("lamroN"); // Normal
					break;
				case 1:
					LCD.print("ykcirT"); // Tricky
					break;
				case 2:
					LCD.print("draH  "); // Hard
					break;
			}

			previous_selection = current_selection;
		}

		if (push_button_state == 1) {
			do {
				push_button_state = digitalRead(PUSH_BUTTON[4]);
			} while (push_button_state == 1);

			difficulty = current_selection;
			break;
		}
	}

	Serial.print("Game Difficulty: ");
	Serial.println(difficulty);

	current_row = 0;

	Serial.print("Game Row set to: ");
	Serial.println(current_row);

	Serial.println("\n-----END BOOTSTRAP-----\n");
}
