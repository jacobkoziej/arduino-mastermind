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
	{255, 000, 000}, // RED
	{255, 165, 000}, // ORANGE
	{255, 255, 000}, // YELLOW
	{000, 255, 000}, // GREEN
	{000, 000, 255}, // BLUE
	{128, 000, 128}, // PURPLE
	{000, 000, 000}, // NONE
	{255, 255, 255}, // WHITE
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
	generateCode();
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

void generateCode() {
	Serial.println("-----BEGIN CODE GENERATION-----");
	Serial.print("\nEvent: ");

	if (player_count == 1) {
		Serial.println("computer generating code");
		// Computer generated secret code
		switch (difficulty) {
			// Normal - no duplicate colors
			case 0:
				// prime secret_code_color
				for (int i = 0; i <= 3; i++) {
					secret_code_color[i] = -1;
				}

				for (int a = 0; a <= 3; a++) {
					int char_exists = 1;
					int temp_val;
					// Generate random colors until it is not duplicate
					while (char_exists == 1) {
						temp_val = random(6);
						char_exists = 0;
						for (int b = 0; b <= 3; b++) {
							if (temp_val == secret_code_color[b]) {
								char_exists = 1;
								break;
							}
						}
					}
					secret_code_color[a] = temp_val;
				}
				break;

			// Tricky - duplicate colors allowed
			case 1:
				for (int i = 0; i <= 3; i++) {
					secret_code_color[i] = random(6);
				}
				break;

			// Hard - duplicate/no color allowed
			case 2:
				for (int i = 0; i <= 3; i++) {
					secret_code_color[i] = random(7);
				}
				break;
		}
		// Placebo progress bar
		LCD.clear();
		LCD.print("Code Generation:");
		LCD.setCursor(0, 1);
		LCD.print("[");
		LCD.setCursor(15, 1);
		LCD.print("]");
		LCD.setCursor(1, 1);
		int placebo_delay = 220;
		for (int i = 1; i <= 14; i++) {
			delay(placebo_delay);
			LCD.print("=");
			placebo_delay += 20;
		}
		delay(1500);
	} else {
		Serial.println("player generating code");

		LCD.clear();
		LCD.print("Code Generation:");
		LCD.setCursor(3, 1);
		LCD.print("By Code Maker");
		delay(3000);

		playerInput();
		for (int i = 0; i <= 3; i++) {
			secret_code_color[i] = input[i];
		}
		LCD.clear();
		LCD.print("Code Generation:");
	}

	LCD.setCursor(0, 1);
	LCD.print("    COMPLETE    ");
	delay(3000);

	Serial.print("Secret Code: ");
	for (int i = 0; i <= 3; i++) {
		Serial.print(secret_code_color[i]);
	}

	Serial.println("\n\n-----END CODE GENERATION-----\n");
}

void LCDcodeStatus(int color, int pos, int update) {
	/*
	 * color - color to show
	 * pos - position of color
	 * update:
	 *	0 - prime LCD
	 *	1 - update only current color
	 *	2 - update only position color
	 */

	String color_word[] = {
		"   RED",
		"ORANGE",
		"YELLOW",
		" GREEN",
		"  BLUE",
		"PURPLE",
		"  NONE"
	};
	char color_letter[] = {'R', 'O', 'Y', 'G', 'B', 'P', 'N'};
	int color_pos[] = {3, 6, 9, 12};

	switch (update) {
		case 0:
			LCD.clear();
			LCD.print("Current:");
			LCD.setCursor(0, 1);
			LCD.print("==[ ][ ][ ][ ]==");
			break;

		case 1:
			LCD.setCursor(10, 0);
			LCD.print(color_word[color]);
			break;

		case 2:
			LCD.setCursor(color_pos[pos], 1);
			LCD.print(color_letter[color]);
			break;
	}
}

void playerInput() {
	LCDcodeStatus(NULL, NULL, 0);
	for (int i =0; i <= 3; i++) {
		input[i] = 6;
		LCDcodeStatus(input[i], i, 2);
	}

	// Set the amount of colors
	int colors;
	if (difficulty != 2) {
		colors = 6;
	} else {
		colors = 7;
	}

	int complete = 0;
	int current, previous = -1;

	while (complete == 0) {
		current = potentiometerSelection(colors);

		if (current != previous) {
			LCDcodeStatus(current, NULL, 1);
			previous = current;
		}

		for (int i = 0; i <= 3; i++) {
			if ((digitalRead(PUSH_BUTTON[i]) == 1) && (current != input[i])) {
				LCDcodeStatus(current, i, 2);
				input[i] = current;
			}
		}

		if (digitalRead(PUSH_BUTTON[4]) == 1) {
			// Don't continue until button is released
			int button_pressed = 1;
			while (button_pressed == 1) {
				button_pressed = digitalRead(PUSH_BUTTON[4]);
			}

			complete = inputCheck();

			// If the check failed inform the user
			if (complete == 0) {
				LCD.clear();
				LCD.print("==INVALID CODE==");
				LCD.setCursor(0, 1);
				LCD.print("[==============]");
				delay(1000);
				
				// Restore previous output
				LCDcodeStatus(NULL, NULL, 0);
				LCDcodeStatus(current, NULL, 1);
				for (int i =0; i <= 3; i++) {
					LCDcodeStatus(input[i], i, 2);
				}
			}
		}
	}
}

// Return if the code input is valid for difficulty
int inputCheck() {
	int return_val = 1;

	switch (difficulty) {
		case 0:
			for (int a = 0; a <= 3; a++) {
				// Ignore running if check has already failed
				if (return_val == 1) {
					for (int b = 0; b <= 3; b++) {
						// No duplicates or color NONE
						if (((input[a] == input[b]) && (a != b)) || (input[b] == 6)) {
							return_val = 0;
							break;
						}
					}
				} else {
					break;
				}
			}
			break;

		case 1:
			for (int i = 0; i <= 3; i++) {
				// No color NONE
				if (input[i] == 6) {
					return_val = 0;
					break;
				}
			}
			break;
	}

	return return_val;
}

void decodeFeedback() {
	Serial.println("-----BEGIN DECODE FEEDBACK-----");
	int color_passed[] = {0, 0, 0, 0};
	int color_pos_passed[] = {0, 0, 0, 0};

	// Check if color and position are both correct
	for (int i = 0; i <= 3; i++) {
		if (input[i] == secret_code_color[i]) {
			color_pos_passed[i] = 1;
		}
	}

	// Check for correct color in incorrect positions
	for (int a = 0; a <= 3; a++) {
		for (int b = 0; b <= 3; b++) {
			if (input[a] == secret_code_color[b]) {
				if ((color_pos_passed[b] == 0) && (color_passed[b] == 0)) {
					color_passed[b] = 1;
					break;
				}
			}
		}
	}

	// Get count for each check passed
	int colors_passed = 0, colors_pos_passed = 0;
	for (int i = 0; i <= 3; i++) {
		colors_passed += color_passed[i];
		colors_pos_passed += color_pos_passed[i];
	}

	Serial.print("\nColors in correct position: ");
	Serial.println(colors_pos_passed);
	Serial.print("Colors in incorrect position: ");
	Serial.println(colors_passed);

	/* Load checks into key peg storage */

	// Preload with color NONE
	for (int i = 0; i <= 3; i++) {
		key_peg_color[current_row][i] = 6;
	}
	// Load correct color and position with RED
	for (int i = 0; i < colors_pos_passed; i++) {
		key_peg_color[current_row][i] = 0;
	}
	// Load correct colors and wrong position with WHITE
	for (int i = colors_pos_passed; i < (colors_pos_passed + colors_passed); i++) {
		key_peg_color[current_row][i] = 7;
	}

	Serial.print("Key Peg Feedback: ");
	for (int i = 0; i <= 3; i++) {
		Serial.print(key_peg_color[0][i]);
	}
	Serial.println("\n\n-----END DECODE FEEDBACK-----\n");
}
