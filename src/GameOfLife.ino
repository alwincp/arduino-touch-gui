/*
 Game Of Life (Display)
 */

#include <Arduino.h>
#include <MI0283QT2.h>
#include <ADS7846.h>
#include <GameOfLife.h>

uint16_t generation = 0;
uint16_t drawcolor[5] = { RGB( 15, 15, 15), RGB(255,255, 0), RGB( 0, 0, 0), RGB( 40, 40, 0), RGB(100,100, 0) };
uint8_t frame[GOL_X_SIZE][GOL_Y_SIZE];

uint8_t alive(uint8_t x, uint8_t y) {
	if ((x < GOL_X_SIZE) && (y < GOL_Y_SIZE)) {
		if ((frame[x][y] == ON_CELL) || (frame[x][y] == NEW_CELL)) {
			return 1;
		}
	}

	return 0;
}

uint8_t neighbors(uint8_t x, uint8_t y) {
	uint8_t count = 0;

	//3 above
	if (alive(x - 1, y - 1)) {
		count++;
	}
	if (alive(x, y - 1)) {
		count++;
	}
	if (alive(x + 1, y - 1)) {
		count++;
	}

	//2 on each side
	if (alive(x - 1, y)) {
		count++;
	}
	if (alive(x + 1, y)) {
		count++;
	}

	//3 below
	if (alive(x - 1, y + 1)) {
		count++;
	}
	if (alive(x, y + 1)) {
		count++;
	}
	if (alive(x + 1, y + 1)) {
		count++;
	}

	return count;
}

void play_gol(MI0283QT2 lcd) {
	uint8_t x, y, count;

	//update cells
	for (x = 0; x < GOL_X_SIZE; x++) {
		for (y = 0; y < GOL_Y_SIZE; y++) {
			count = neighbors(x, y);
			if ((count == 3) && !alive(x, y)) {
				frame[x][y] = NEW_CELL; //new cell
			}
			if (((count < 2) || (count > 3)) && alive(x, y)) {
				frame[x][y] = DEL_CELL; //cell dies
			}
		}
	}

	//inc generation
	if (++generation > GOL_MAX_GEN) {
		init_gol(lcd);
	}
}

void draw_gol(MI0283QT2 lcd) {
	uint8_t c, x, y, color;
	uint16_t px, py;

	for (x = 0, px = 0; x < GOL_X_SIZE; x++) {
		for (y = 0, py = 0; y < GOL_Y_SIZE; y++) {
			c = frame[x][y];
			if (c && (c != ON_CELL)) {
				if (c == NEW_CELL) { //new
					frame[x][y] = ON_CELL;
					color = ALIVE_COLOR;
				} else if (c == 1) { //del
					frame[x][y] = 0;
					color = DEAD_COLOR;
				} else if (c == 2) { //die
					frame[x][y] = 1;
					color = DIE1_COLOR;
				} else if (c <= DEL_CELL) { //die
					frame[x][y]--;
					color = DIE2_COLOR;
				}
				lcd.fillRect(px + 1, py + 1, px + (lcd.getWidth() / GOL_X_SIZE) - 2,
						py + (lcd.getHeight() / GOL_Y_SIZE) - 2, drawcolor[color]);
			}
			py += (lcd.getHeight() / GOL_Y_SIZE);
		}
		px += (lcd.getWidth() / GOL_X_SIZE);
	}
}

void init_gol(MI0283QT2 lcd) {
	uint8_t x, y;
	uint16_t px, py;
	uint32_t c;

	generation = 0;

	//change color
	drawcolor[2] = RGB( 255, 255, 255);
	switch (drawcolor[1]) {
	case RGB( 0,255, 0): //RGB(255,  0,  0)
		drawcolor[0] = RGB( 15, 15, 15);
		drawcolor[1] = RGB(255, 0, 0);
		drawcolor[3] = RGB( 40, 0, 0);
		drawcolor[4] = RGB(100, 0, 0);
		break;

	case RGB( 0, 0,255): //RGB(  0,255,  0)
		drawcolor[0] = RGB( 15, 15, 15);
		drawcolor[1] = RGB( 0,255, 0);
		drawcolor[3] = RGB( 0, 40, 0);
		drawcolor[4] = RGB( 0,100, 0);
		break;

	case RGB(255,255, 0): //RGB(  0,  0,255)
		drawcolor[0] = RGB( 15, 15, 15);
		drawcolor[1] = RGB( 0, 0,255);
		drawcolor[3] = RGB( 0, 0, 40);
		drawcolor[4] = RGB( 0, 0,100);
		break;

	default: //RGB(255,255,  0)
		drawcolor[0] = RGB( 15, 15, 15);
		drawcolor[1] = RGB(255,255, 0);
		drawcolor[3] = RGB( 40, 40, 0);
		drawcolor[4] = RGB(100,100, 0);
		break;
	}

	//generate random start data
	srand(frame[GOL_X_SIZE / 2][GOL_Y_SIZE / 2] + analogRead(0));
	for (x = 0; x < GOL_X_SIZE; x++) {
		c = (rand() | (rand() << 16)) & 0xAAAAAAAA; //0xAAAAAAAA 0x33333333 0xA924A924
		for (y = 0; y < GOL_Y_SIZE; y++) {
			if (c & (1 << y)) {
				frame[x][y] = NEW_CELL;
			} else {
				frame[x][y] = 0;
			}
		}
	}

//  draw_gol(lcd);
	//redraw cells
	for (x = 0, px = 0; x < GOL_X_SIZE; x++) {
		for (y = 0, py = 0; y < GOL_Y_SIZE; y++) {
			lcd.fillRect(px + 1, py + 1, px + (lcd.getWidth() / GOL_X_SIZE) - 2,
					py + (lcd.getHeight() / GOL_Y_SIZE) - 2, drawcolor[DEAD_COLOR]);
			py += (lcd.getHeight() / GOL_Y_SIZE);
		}
		px += (lcd.getWidth() / GOL_X_SIZE);
	}
}

void drawGenerationText(MI0283QT2 lcd) {
	uint16_t x;
	//draw current generation
	x = lcd.drawText(0, 0, "Gen.", 1, RGB(50,50,50), drawcolor[DEAD_COLOR]);
	lcd.drawInteger(x, 0, (int) generation, 10, 1, RGB(50,50,50), drawcolor[DEAD_COLOR]);
}
