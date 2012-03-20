/*
 * GameOfLife.h
 *
 *  Created on: 07.02.2012
 *      Author:
 */

#ifndef GAMEOFLIFE_H_
#define GAMEOFLIFE_H_

#include <MI0283QT2.h>

#define BG_COLOR    (0)
#define ALIVE_COLOR (1)
#define DEAD_COLOR  (2)
#define DIE1_COLOR  (3)
#define DIE2_COLOR  (4)

#define GOL_MAX_GEN (1000) //max generations
#define GOL_X_SIZE   (20)
#define GOL_Y_SIZE   (15)

#define DEL_CELL (3)
#define ON_CELL  (0xAA)
#define NEW_CELL (0xFF)

void init_gol(MI0283QT2 lcd);
void play_gol(MI0283QT2 lcd);
void draw_gol(MI0283QT2 lcd);
void drawGenerationText(MI0283QT2 lcd);

#endif /* GAMEOFLIFE_H_ */
