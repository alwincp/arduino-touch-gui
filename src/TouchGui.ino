/*
 * TouchGui.ino
 *
 *  Created on: 31.01.2012
 *      Author: Armin Joachimsmeyer
 *      Email:   armin.joachimsmeyer@gmx.de
 *      License: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *      Version: 1.0.0
 *
 *      Demo of the libs:
 *      TouchButton
 *      TouchSlider
 *      Chart
 *
 *      and the "Apps" ;-)
 *      Draw lines
 *      Game of life
 *      Show ADS7846 A/D channels
 *      Display font
 *
 *      For Arduino Uno
 *      with mSD-Shield and MI0283QT Adapter from www.watterott.net
 *      and the ADS7846 and MI0283QT2 libs from
 *		https://github.com/watterott/mSD-Shield/downloads
 *
 *		MI0283QT2/fonts.h must be a version with disabled "#define FONT_END7F" to show all font characters
 *		ADS7846/ADS7846.cpp must be the modified version in order to show the ADS7846 channels
 *
 */
/*
 * Main switches for program
 */
//#define DEBUG // enable debug button and debug output
#define RTC_EXISTS  //if a DS1307 is connected to the I2C bus
#include <Arduino.h>

#include <MI0283QT2.h>
#include <ADS7846.h>
#include <TouchButton.h>
#include <TouchButtonAutorepeat.h>
#include <TouchSlider.h>
#include "GameOfLife.h"
#include <Chart.h>

#ifdef __cplusplus
extern "C" {
#ifdef RTC_EXISTS
#include <i2cmaster.h>
#endif
}
#endif

// saves 500 bytes ROM and 40 bytes RAM
#define TOUCH_SAVE_SPACE

#ifdef DEBUG
TouchButton TouchButtonDebug;
void doDebug(TouchButton * const aTheTouchedButton, int aValue);
#endif

/*
 * LCD and touch panel stuff
 */
#define TP_EEPROMADDR (E2END -1 - sizeof(CAL_MATRIX)) //eeprom address for calibration data - 28 bytes
ADS7846 TouchPanel;
void printTPData(void);

MI0283QT2 TFTDisplay;
#define DISPLAY_HEIGHT 240
#define DISPLAY_WIDTH 320
#define BACKGROUND_COLOR COLOR_WHITE

/*
 * Loop control
 */
bool StartNewTouch = true;
unsigned long LastMillis = 0;
unsigned long LoopMillis = 0;

void createButtonsAndSliders(void);

// Global button handler
void doButtons(TouchButton * const aTheTochedButton, int aValue);

//Global slider handler
uint8_t doSliders(TouchSlider * const aTheTochedSlider, uint8_t aSliderValue);

/*
 * Draw stuff
 */
TouchButton TouchButtonDraw;
void doDraw(TouchButton * const aTheTouchedButton, int aValue);
void drawLine(const bool aNewStart, unsigned int color);

TouchButton TouchButtonClear_Continue;

TouchButton TouchButtonDrawColor[5];
const uint16_t DrawColors[5] = { COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW };
void doDrawColor(TouchButton * const aTheTouchedButton, int aValue);
unsigned int DrawColor = COLOR_BLACK;

/*
 * Game of life stuff
 */
void showGolSettings(void);

TouchButton TouchButtonGolDying;
TouchSlider TouchSliderGolDying;
void doGolDying(TouchButton * const aTheTouchedButton, int aValue);
void syncronizeGolSliderAndButton(void);
const char * mapValueGol(uint8_t aValue);

TouchButton TouchButtonNew;
void doClearAndNewContinue(TouchButton * const aTheTouchedButton, int aValue);
void initNewGameOfLife(void);

TouchSlider TouchSliderGolSpeed;

char sStringGOL[] = "GameOfLife";
// to slow down game of life
unsigned long GolDelay = 0;
bool GolShowDying = true;
bool GolRunning = false;
bool GolInitialized = false;
#define GOL_DIE_THRESHOLD 5
#define GOL_DIE_MAX 20
extern unsigned int drawcolor[5]; // see gameOfLife.ino

/**
 * Menu stuff
 */
TouchButton TouchButtonHome;
void showMenu(void);

TouchButton TouchButtonChart;
void showCharts(void);
Chart ChartExample;

TouchButton TouchButtonFont;
void showFont(void);

#define MENU_TOP 15
#define MENU_LEFT 30
// space between buttons
#define BUTTON_SPACING 20

#define APPLICATION_MENU 0
#define APPLICATION_SETTINGS 1
#define APPLICATION_DRAW 2
#define APPLICATION_GAME_OF_LIFE 3
#define APPLICATION_CHART 4
#define APPLICATION_ADS7846_CHANNELS 5

int mActualApplication = APPLICATION_MENU;

/*
 * Settings stuff
 */
TouchButton TouchButtonSettings;
void showSettings(void);

TouchButton TouchButtonCalibration_GameOfLife;

TouchSlider TouchSliderBacklight;
uint8_t doBacklight(TouchSlider * const aTheTouchedSlider, const uint8_t aBrightness);
const char * mapBacklightPowerValue(uint8_t aBrightness);
uint8_t BacklightValue = 60;

TouchButtonAutorepeat TouchButtonAutorepeatBacklightIncrease;
TouchButtonAutorepeat TouchButtonAutorepeatBacklightDecrease;
void doChangeBacklight(TouchButton * const aTheTouchedButton, int aValue);

TouchSlider TouchSliderDemo1;
TouchSlider TouchSliderAction;
TouchSlider TouchSliderActionWithoutBorder;
unsigned int ActionSliderValue = 0;
#define ACTION_SLIDER_MAX 100
bool ActionSliderUp = true;

/*
 * ADS7846 channels stuff
 */
TouchButton TouchButtonADS7846Channels;
void ADS7846DisplayChannels(void);

const prog_char Temperature0[] PROGMEM = "Temperature 0";
const prog_char Temperature1[] PROGMEM = "Temperature 1";
const prog_char PosX[] PROGMEM = "X Position";
const prog_char PosY[] PROGMEM = "Y Position";
const prog_char PosZ1[] PROGMEM = "Z Position 1";
const prog_char PosZ2[] PROGMEM = "Z Position 2";
const prog_char Vcc[] PROGMEM = "VCC";
const prog_char Aux[] PROGMEM = "Aux Input";
const char * ADS7846ChannelsText[] = { PosX, PosY, PosZ1, PosZ2, Temperature0, Temperature1, Vcc, Aux };
unsigned char ADS7846Channels[] = { 1, 5, 3, 4, 0, 7, 2, 6 };

/*
 * RTC Stuff
 */
#ifdef RTC_EXISTS
#define DS1307_ADDR 0xD0 // 0x68 shifted left
uint8_t bcd2bin(uint8_t val);
uint8_t bin2bcd(uint8_t val);
void showRTCTime(void);
void setRTCTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dayOfWeek, uint8_t day, uint8_t month, uint16_t year);
#endif

// a string buffer for any purpose...
char StringBuffer[128];

void setup() {

	//Serial.begin(115200);
	//init display
	TFTDisplay.init(4); //spi-clk = Fcpu/4

	//init touch controller
	TouchPanel.init();

	//touch-panel calibration
	TouchPanel.service();
	if (TouchPanel.getPressure() > 5) {
		//clear screen
		TFTDisplay.clear(COLOR_WHITE);
		TouchPanel.doCalibration(&TFTDisplay, TP_EEPROMADDR, 0); //dont check EEPROM for calibration data
	} else {
		TouchPanel.doCalibration(&TFTDisplay, TP_EEPROMADDR, 1); //check EEPROM for calibration data
	}

#ifndef TOUCH_SAVE_SPACE
	// initialize buttons, slider and chart
	TouchButton::init(TFTDisplay);
	TouchSlider::init(TFTDisplay);
	Chart::init(TFTDisplay);
#endif
	createButtonsAndSliders();

	//show menu
	showMenu();

#ifdef RTC_EXISTS
	// activate internal pullups for twi.
	digitalWrite(SDA, 1);
	digitalWrite(SCL, 1);
	i2c_init(); //Set speed

	//only set the date+time once
	//setRTCTime(0, 0, 1, 2, 26, 3, 2012); //08:00:00 24.12.2011 //sec, min, hour, ,dayOfWeek, day, month, year
#endif

	LastMillis = millis();
}

void loop() {
	bool tGuiTouched = false;

	//service routine for touch panel
	// get tp values
	TouchPanel.service();

	// count milliseconds for loop control
	unsigned long tMillis = millis();
	LoopMillis += tMillis - LastMillis;
	LastMillis = tMillis;

	//touch press?
	if (TouchPanel.getPressure() > 6) {
		if (mActualApplication == APPLICATION_MENU || mActualApplication == APPLICATION_DRAW) {
			printTPData();
		}

		/*
		 * check if button or slider is touched
		 */
		tGuiTouched = TouchButton::checkAllButtons(TouchPanel.getX(), TouchPanel.getY());
		if (!tGuiTouched) {
			tGuiTouched = TouchSlider::checkAllSliders(TouchPanel.getX(), TouchPanel.getY());
		}

		if (!tGuiTouched) {
			/**
			 * switch for different "apps"
			 */
			switch (mActualApplication) {
			case APPLICATION_DRAW:
				//draw line
				drawLine(StartNewTouch, DrawColor);
				break;
			case APPLICATION_GAME_OF_LIFE:
				if (GolRunning) {
					showGolSettings();
				}
				break;
			}
		}

		StartNewTouch = false;

	} else {
		/**
		 * no touch here
		 * switch for different "apps"
		 */
		switch (mActualApplication) {
		case APPLICATION_SETTINGS:
			// Moving slider bar :-)
			if (LoopMillis >= 20) {
				LoopMillis = 0;
				if (ActionSliderUp) {
					ActionSliderValue++;
					if (ActionSliderValue == ACTION_SLIDER_MAX) {
						ActionSliderUp = false;
					}
				} else {
					ActionSliderValue--;
					if (ActionSliderValue == 0) {
						ActionSliderUp = true;
					}
				}
				TouchSliderAction.setActualValue(ActionSliderValue);
				TouchSliderActionWithoutBorder.setActualValue(ACTION_SLIDER_MAX - ActionSliderValue);
			}
			break;

		case APPLICATION_GAME_OF_LIFE:
			if (GolRunning && LoopMillis >= GolDelay) {
				//game of live "app"
				play_gol(TFTDisplay);
				draw_gol(TFTDisplay);
				drawGenerationText(TFTDisplay);
				LoopMillis = 0;
			}
			break;
		}
		StartNewTouch = true;
	} // touch press?

	/*
	 * Actions independent of touch
	 */
	if (mActualApplication == APPLICATION_ADS7846_CHANNELS) {
		ADS7846DisplayChannels();
	}

#ifdef RTC_EXISTS
	if (mActualApplication == APPLICATION_MENU) {
		if (LoopMillis > 200) {
			LoopMillis = 0;
			//RTC
			showRTCTime();
		}
	}
#endif

}

/*
 * buttons are positioned relative to sliders an vice versa
 */
void createButtonsAndSliders(void) {
	/**
	 * create and position all BUTTONS initially
	 */
	int8_t tErrorValue = 0;
	tErrorValue += TouchButtonDraw.initSimpleButton(20, 20, 120, 30, "Draw", 2, 0, &doDraw);
	TouchButtonCalibration_GameOfLife.initSimpleButton(TouchButtonDraw.getPositionXRight() + BUTTON_SPACING, 20, 120,
			30, sStringGOL, 1, 0, &doButtons);

	tErrorValue += TouchButtonChart.initSimpleButton(20, TouchButtonDraw.getPositionYBottom() + BUTTON_SPACING, 120, 30,
			"Chart", 2, 0, &doButtons);

	tErrorValue += TouchButtonSettings.initSimpleButton(TouchButtonChart.getPositionXRight() + BUTTON_SPACING,
			TouchButtonCalibration_GameOfLife.getPositionYBottom() + BUTTON_SPACING, 120, 30, "Settings", 1, 0,
			&doButtons);

	tErrorValue += TouchButtonFont.initSimpleButton(20, TouchButtonSettings.getPositionYBottom() + BUTTON_SPACING, 120,
			30, "Font", 2, 0, &doButtons);

	tErrorValue += TouchButtonADS7846Channels.initSimpleButton(TouchButtonFont.getPositionXRight() + BUTTON_SPACING,
			TouchButtonSettings.getPositionYBottom() + BUTTON_SPACING, 120, 30, "ADS7846", 1, 0, &doButtons);

#ifdef DEBUG
// DEBUG button - lower left corner - ONLY caption, no background
	tErrorValue += TouchButtonDebug.initSimpleButton(0, DISPLAY_HEIGHT - (2 * FONT_HEIGHT) - 1, 0, 0, "DEBUG", 2, 0,
			&doDebug);
#endif
	tErrorValue += TouchButtonNew.initSimpleButton(0, DISPLAY_HEIGHT - (2 * FONT_HEIGHT) - 1, 0, 0, "New", 2, 0,
			&doClearAndNewContinue);
// in Game of life, button is NOT redefined, only caption is changed => Handler must be aware of both usages
	tErrorValue += TouchButtonClear_Continue.initSimpleButton(TouchButtonNew.getPositionXRight() + 45,
			DISPLAY_HEIGHT - (2 * FONT_HEIGHT) - 1, 0, 0, "CLEAR", 2, 0, &doClearAndNewContinue);
// HOME text button - lower right corner
	tErrorValue += TouchButtonHome.initSimpleButton(DISPLAY_WIDTH - (8 * FONT_WIDTH) - 1,
			DISPLAY_HEIGHT - (2 * FONT_HEIGHT) - 1, 0, 0, "MENU", 2, 0, &doButtons);

	/*
	 * Backlight buttons
	 */
	tErrorValue += TouchButtonAutorepeatBacklightIncrease.initSimpleButton(30, MENU_TOP,
			TOUCHSLIDER_DEFAULT_SIZE * TOUCHSLIDER_OVERALL_SIZE_FACTOR, 2 * FONT_HEIGHT, "+", 1, 1, &doChangeBacklight);
	tErrorValue += TouchSliderBacklight.initSlider(30, TouchButtonAutorepeatBacklightIncrease.getPositionYBottom() + 4,
			TOUCHSLIDER_DEFAULT_SIZE, 110, true, "Backlight", BacklightValue, 100, true,
			TOUCHSLIDER_DEFAULT_TOUCH_BORDER, &doBacklight, &mapBacklightPowerValue);
	tErrorValue += TouchButtonAutorepeatBacklightDecrease.initSimpleButton(30,
			TouchSliderBacklight.getPositionYBottom() + 30, TOUCHSLIDER_DEFAULT_SIZE * TOUCHSLIDER_OVERALL_SIZE_FACTOR,
			2 * FONT_HEIGHT, "-", 1, -1, &doChangeBacklight);
	TouchButtonAutorepeatBacklightIncrease.setButtonAutorepeatTiming(600, 100, 2000, 20, &StartNewTouch);
	TouchButtonAutorepeatBacklightDecrease.setButtonAutorepeatTiming(600, 100, 2000, 20, &StartNewTouch);

	/**
	 * SLIDER
	 */
	tErrorValue += TouchSliderGolSpeed.initSlider(100, MENU_TOP, TOUCHSLIDER_DEFAULT_SIZE, 75, true, "Gol-Speed", 75,
			75, true, TOUCHSLIDER_DEFAULT_TOUCH_BORDER, &doSliders, &mapValueGol);
// slider as switch
	tErrorValue += TouchSliderGolDying.initSlider(100, TouchSliderGolSpeed.getPositionYBottom() + (3 * FONT_HEIGHT),
			TOUCHSLIDER_DEFAULT_SIZE, GOL_DIE_MAX, true, "Gol-show die", GOL_DIE_MAX, GOL_DIE_MAX, false, 20,
			&doSliders, NULL);

// ON OFF button relative to slider
	tErrorValue += TouchButtonGolDying.initSimpleButton(100,
			TouchSliderGolDying.getPositionYBottom() + FONT_HEIGHT + 10, 25, 25, NULL, 1, 0, &doGolDying);

// self moving sliders
	tErrorValue += TouchSliderActionWithoutBorder.initSlider(180,
			TouchButtonCalibration_GameOfLife.getPositionYBottom() + 30, 10, ACTION_SLIDER_MAX, false, NULL, 0,
			ACTION_SLIDER_MAX, true, 3, NULL, NULL);
	tErrorValue += TouchSliderAction.initSlider(TouchSliderActionWithoutBorder.getPositionXRight() + 15,
			TouchButtonCalibration_GameOfLife.getPositionYBottom() + 25, 10, ACTION_SLIDER_MAX, true, NULL, 0,
			ACTION_SLIDER_MAX, true, 0, NULL, NULL);

// Color buttons
	uint16_t tPosY = 0;
	for (uint8_t i = 0; i < 5; ++i) {
		tErrorValue += TouchButtonDrawColor[i].initButton(0, tPosY, 30, 30, NULL, 1, TOUCHBUTTON_DEFAULT_TOUCH_BORDER,
				DrawColors[i], 0, i, &doDrawColor);
		tPosY += 30;
	}

	if (tErrorValue != 0) {
		// Show Error
		TFTDisplay.clear(BACKGROUND_COLOR);
		TFTDisplay.drawText(40, 100, (char *) "Error on rendering", 2, COLOR_RED, BACKGROUND_COLOR);
		TFTDisplay.drawText(40, 120, tErrorValue, 2, COLOR_RED, BACKGROUND_COLOR);
		delay(5000);
	}
}

uint8_t doSliders(TouchSlider * const aTheTouchedSlider, uint8_t aSliderValue) {
	if (aTheTouchedSlider == &TouchSliderGolSpeed) {
		aSliderValue = (aSliderValue / 25) * 25;
	}
	if (aTheTouchedSlider == &TouchSliderGolDying) {
		if (aSliderValue > GOL_DIE_THRESHOLD) {
			GolShowDying = false;
			aSliderValue = GOL_DIE_MAX;
		} else {
			GolShowDying = true;
			aSliderValue = 0;
		}
		syncronizeGolSliderAndButton();
	}
	return aSliderValue;
}

void doButtons(TouchButton * const aTheTouchedButton, int aValue) {
	TouchButton::deactivateAllButtons();
	TouchSlider::deactivateAllSliders();
	if (aTheTouchedButton == &TouchButtonChart) {
		showCharts();
		return;
	}

	if (aTheTouchedButton == &TouchButtonCalibration_GameOfLife) {
		if (mActualApplication == APPLICATION_SETTINGS) {
			//Calibration Button pressed -> calibrate touch panel
			TouchPanel.doCalibration(&TFTDisplay, TP_EEPROMADDR, 0);
			showSettings();
			return;
		}
		// Game of Life button pressed
		showGolSettings();
		mActualApplication = APPLICATION_GAME_OF_LIFE;
		return;
	}
	if (aTheTouchedButton == &TouchButtonSettings) {
		// Settings button pressed
		showSettings();
		return;
	}
	if (aTheTouchedButton == &TouchButtonHome) {
		// Home button pressed
		showMenu();
		return;
	}
	if (aTheTouchedButton == &TouchButtonFont) {
		// Home button pressed
		showFont();
		return;
	}
	if (aTheTouchedButton == &TouchButtonADS7846Channels) {
		mActualApplication = APPLICATION_ADS7846_CHANNELS;
		TFTDisplay.clear(BACKGROUND_COLOR);
		uint16_t tPosY = MENU_TOP;
		// draw text
		for (uint8_t i = 0; i < 8; ++i) {
			TFTDisplay.drawTextPGM(90, tPosY, ADS7846ChannelsText[i], 2, COLOR_RED, BACKGROUND_COLOR);
			tPosY += FONT_HEIGHT * 2;
		}

		TouchButtonHome.drawButton();
	}
}

void checkBacklightValue(void) {
	if (BacklightValue > 100) {
		BacklightValue = 100;
	}
	if (BacklightValue < 5) {
		BacklightValue = 5;
	}
}
uint8_t doBacklight(TouchSlider * const aTheTouchedSlider, const uint8_t aBrightness) {
	BacklightValue = (aBrightness / 5) * 5;
	checkBacklightValue();
	TFTDisplay.led(BacklightValue);
	return (aBrightness / 5) * 5;
}

void doChangeBacklight(TouchButton * const aTheTouchedButton, int aValue) {
	BacklightValue += aValue;
	checkBacklightValue();
	TouchSliderBacklight.setActualValue(BacklightValue);
	TFTDisplay.led(BacklightValue);
}

const char * mapBacklightPowerValue(uint8_t aBrightness) {
	sprintf(StringBuffer, "%03u", BacklightValue);
	return StringBuffer;
}

// value map function for game of life speed slider
const char * mapValueGol(uint8_t aValue) {
	aValue = aValue / 25;
	switch (aValue) {
	case 0:
		GolDelay = 8000;
		return "slowest";
	case 1:
		GolDelay = 2000;
		return "slow   ";
	case 2:
		GolDelay = 500;
		return "normal ";
	case 3:
		GolDelay = 0;
		return "fast   ";
	}
	return "fast   ";
}

void doGolDying(TouchButton * const aTheTouchedButton, int aValue) {
	/*
	 * GolDying button pressed
	 */
	if (StartNewTouch) {
		if (GolShowDying == false) {
			GolShowDying = true;
		} else {
			GolShowDying = false;
		}
		syncronizeGolSliderAndButton();
	}
}

void showGolSettings(void) {
	/*
	 * Settings button pressed
	 */
	GolRunning = false;
	TFTDisplay.clear(BACKGROUND_COLOR);
	TouchButtonHome.drawButton();
	TouchSliderGolDying.drawSlider();
	TouchButtonGolDying.drawButton();
	syncronizeGolSliderAndButton();
	TouchSliderGolSpeed.drawSlider();
// redefine Buttons
	TouchButtonNew.drawButton();
	TouchButtonClear_Continue.setCaption("CONTINUE");
	TouchButtonClear_Continue.drawButton();
}

void doClearAndNewContinue(TouchButton * const aTheTouchedButton, int aValue) {
	if (mActualApplication == APPLICATION_DRAW) {
		doDraw(aTheTouchedButton, aValue);
		return;
	}
// deactivate gui elements except new, continue and menu
	TouchSliderGolDying.deactivate();
	TouchButtonGolDying.deactivate();
	TouchSliderGolSpeed.deactivate();
	TouchButtonClear_Continue.deactivate();
	if (aTheTouchedButton == &TouchButtonNew || !GolInitialized) {
		initNewGameOfLife();
	}
	TFTDisplay.clear(BACKGROUND_COLOR);
	LoopMillis = GolDelay;
	GolRunning = true;
}

void initNewGameOfLife(void) {
	GolInitialized = true;
	init_gol(TFTDisplay);
	drawcolor[0] = BACKGROUND_COLOR;
	if (!GolShowDying) {
		// change colors
		drawcolor[2] = BACKGROUND_COLOR;
		drawcolor[3] = BACKGROUND_COLOR;
		drawcolor[4] = BACKGROUND_COLOR;
	}
}

void doDraw(TouchButton * const aTheTouchedButton, int aValue) {
	/*
	 * Draw button pressed
	 */
	TouchButton::deactivateAllButtons();
	TFTDisplay.clear(BACKGROUND_COLOR);
	TouchButtonHome.drawButton();
	TouchButtonClear_Continue.setCaption("CLEAR");
	TouchButtonClear_Continue.drawButton();
	for (uint8_t i = 0; i < 5; ++i) {
		TouchButtonDrawColor[i].drawButton();
	}
	mActualApplication = APPLICATION_DRAW;
}

void doDrawColor(TouchButton * const aTheTouchedButton, int aIndex) {
	DrawColor = DrawColors[aIndex];
}

#ifdef DEBUG
void doDebug(TouchButton * const aTheTouchedButton, int aValue) {
	/*
	 * Debug button pressed
	 */
	uint8_t * stackptr = (uint8_t *) malloc(4); // use stackptr temporarily
	uint8_t * heapptr = stackptr;// save value of heap pointer
	free(stackptr);
	stackptr = (uint8_t *) (SP);
	sprintf(StringBuffer, "TB=%02u TS=%02u CH=%02u Stack=%04u Heap=%04u", sizeof *aTheTouchedButton,
			sizeof TouchSliderBacklight, sizeof ChartExample, (unsigned int) stackptr, (unsigned int) heapptr);
	TFTDisplay.drawText(1, DISPLAY_HEIGHT - (3 * FONT_HEIGHT) - 1, StringBuffer, 1, COLOR_BLACK, BACKGROUND_COLOR);
}
#endif

void showSettings(void) {
	/*
	 * Settings button pressed
	 */
	TFTDisplay.clear(BACKGROUND_COLOR);
	TouchButtonHome.drawButton();
#ifdef DEBUG
	TouchButtonDebug.drawButton();
#endif
	TouchButtonCalibration_GameOfLife.setCaption("TP-Calibration");
	TouchButtonCalibration_GameOfLife.drawButton();
	TouchSliderGolDying.drawSlider();
	TouchButtonGolDying.drawButton();
	syncronizeGolSliderAndButton();
	TouchButtonAutorepeatBacklightIncrease.drawButton();
	TouchSliderBacklight.drawSlider();
	TouchButtonAutorepeatBacklightDecrease.drawButton();
	TouchSliderGolSpeed.drawSlider();
	TouchSliderAction.drawSlider();
	TouchSliderActionWithoutBorder.drawSlider();
	mActualApplication = APPLICATION_SETTINGS;
}

void showMenu(void) {
	TouchButtonHome.deactivate();
	TFTDisplay.clear(BACKGROUND_COLOR);
#ifdef DEBUG
	TouchButtonDebug.drawButton();
#endif
	TouchButtonSettings.drawButton();
	TouchButtonChart.drawButton();
	TouchButtonDraw.drawButton();
	TouchButtonFont.drawButton();
	TouchButtonADS7846Channels.drawButton();
	TouchButtonCalibration_GameOfLife.setCaption(sStringGOL);
	TouchButtonCalibration_GameOfLife.drawButton();
	GolInitialized = false;
	mActualApplication = APPLICATION_MENU;
}

void showFont(void) {
	TFTDisplay.clear(BACKGROUND_COLOR);
	TouchButtonHome.drawButton();

	TFTDisplay.printOptions(1, COLOR_GREEN, BACKGROUND_COLOR);
	uint16_t tXPos;
	uint16_t tYPos = 10;
	unsigned char tChar = FONT_START;
	for (uint8_t i = 14; i != 0; i--) {
		tXPos = 10;
		for (uint8_t j = 16; j != 0; j--) {
			tXPos = TFTDisplay.drawChar(tXPos, tYPos, tChar, 1, COLOR_BLACK, COLOR_YELLOW) + 4;
			tChar++;
		}
		tYPos += FONT_HEIGHT + 4;
	}
}

void showCharts(void) {
	TFTDisplay.clear(BACKGROUND_COLOR);
	TouchButtonHome.drawButton();

// Chart with grid without labels
// reset labels
	ChartExample.setXLabelIncrementValueFloat(0);
	ChartExample.setYLabelIncrementValueFloat(0);
	ChartExample.setXLabelIncrementValue(0);
	ChartExample.setYLabelIncrementValue(0);
	ChartExample.initChartColors(COLOR_RED, CHART_DEFAULT_GRID_COLOR, COLOR_RED, BACKGROUND_COLOR);
	ChartExample.initChart(5, 100, 120, 80, 2, true, 20, 20);
	ChartExample.drawChart();

//generate random data
	srand(120 + analogRead(0));
	char * tPtr = StringBuffer;
	for (uint8_t i = 0; i < sizeof StringBuffer; i++) {
		*tPtr++ = 30 + (rand() >> 11);
	}
	ChartExample.drawChartData((uint8_t *) &StringBuffer, sizeof StringBuffer, COLOR_RED, CHART_MODE_PIXEL);

// Chart without grid with integer labels
	ChartExample.initXLabelInt(0, 12, 2);
	ChartExample.initYLabelInt(-20, 20, 3);
	ChartExample.initChart(170, 100, 140, 88, 2, false, 30, 15);
	ChartExample.drawChart();
// new data
	tPtr = StringBuffer;
	uint8_t tVal = 0;
	for (uint8_t i = 0; i < sizeof StringBuffer; i++) {
		tVal += rand() >> 14;
		*tPtr++ = tVal;
	}
	ChartExample.drawChartData((uint8_t *) StringBuffer, sizeof StringBuffer, COLOR_BLUE, CHART_MODE_LINE);

// reset to no integer label because chart object is reused
	ChartExample.setXLabelIncrementValue(0);
	ChartExample.setYLabelIncrementValue(0);
	ChartExample.initXLabelFloat(0, 0.5, 3, 1);
	ChartExample.initYLabelFloat(0, 0.2, 3, 1);
	ChartExample.initChart(30, DISPLAY_HEIGHT - 20, 100, 90, 2, true, 40, 20);
	ChartExample.drawChart();

	ChartExample.initXLabelInt(0, 20, 2);
	ChartExample.initYLabelFloat(0, 0.3, 3, 1);
	ChartExample.initChart(170, DISPLAY_HEIGHT - 40, 140, 70, 2, false, 30, 16);
	ChartExample.drawChart();
	ChartExample.drawChartData((uint8_t *) StringBuffer, sizeof StringBuffer, COLOR_BLUE, CHART_MODE_AREA);

	mActualApplication = APPLICATION_CHART;
}

void syncronizeGolSliderAndButton(void) {
	if (GolShowDying == false) {
		TouchButtonGolDying.setColor(TOUCHBUTTON_DEFAULT_COLOR);
		TouchButtonGolDying.drawButton();
		TouchSliderGolDying.setActualValue(0);
	} else {
		TouchButtonGolDying.setColor(TOUCHSLIDER_DEFAULT_BAR_COLOR);
		TouchButtonGolDying.drawButton();
		TouchSliderGolDying.setActualValue(GOL_DIE_MAX);
	}
}

//show touchpanel data
void printTPData(void) {
	sprintf(StringBuffer, "X:%03i|%04i Y:%03i|%04i P:%03i", TouchPanel.getX(), TouchPanel.getXraw(), TouchPanel.getY(),
			TouchPanel.getYraw(), TouchPanel.getPressure());
	TFTDisplay.drawText(20, 2, StringBuffer, 1, COLOR_BLACK, BACKGROUND_COLOR);
}

void drawLine(const bool aNewStart, unsigned int color) {
	static unsigned int last_x = 0, last_y = 0;
	if (aNewStart) {
		TFTDisplay.drawPixel(TouchPanel.getX(), TouchPanel.getY(), color);
	} else {
		TFTDisplay.drawLine(last_x, last_y, TouchPanel.getX(), TouchPanel.getY(), color);
	}
	last_x = TouchPanel.getX();
	last_y = TouchPanel.getY();
}

void ADS7846DisplayChannels(void) {
	uint16_t tPosY = MENU_TOP;
	int16_t tTemp;
	for (uint8_t i = 0; i < 8; ++i) {
		tTemp = TouchPanel.readChannel(ADS7846Channels[i], 32);
		sprintf(StringBuffer, "%04u", tTemp);
		TFTDisplay.drawText(15, tPosY, StringBuffer, 2, COLOR_RED, BACKGROUND_COLOR);
		tPosY += FONT_HEIGHT * 2;
	}
}

#ifdef RTC_EXISTS

uint8_t bcd2bin(uint8_t val) {
	return val - 6 * (val >> 4);
}
uint8_t bin2bcd(uint8_t val) {
	return val + 6 * (val / 10);
}

void showRTCTime(void) {
	uint8_t RtcBuf[7];

	/*
	 * get time from RTC
	 */
// write start address
	i2c_start(DS1307_ADDR + I2C_WRITE); // set device address and write mode
	i2c_write(0x00);
	i2c_stop();

// read 6 bytes from start address
	i2c_start(DS1307_ADDR + I2C_READ); // set device address and read mode
	for (uint8_t i = 0; i < 6; ++i) {
		RtcBuf[i] = bcd2bin(i2c_readAck());
	}
// read year and stop
	RtcBuf[6] = bcd2bin(i2c_readNak());
	i2c_stop();

//buf[3] is day of week
	sprintf_P(StringBuffer, PSTR("%02i.%02i.%04i %02i:%02i:%02i"), RtcBuf[4], RtcBuf[5], RtcBuf[6] + 2000, RtcBuf[2],
			RtcBuf[1], RtcBuf[0]);
	TFTDisplay.drawText(10, DISPLAY_HEIGHT - FONT_HEIGHT - 1, StringBuffer, 1, COLOR_RED, BACKGROUND_COLOR);
}

void setRTCTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dayOfWeek, uint8_t day, uint8_t month, uint16_t year) {
//Write Start Adress
	i2c_start(DS1307_ADDR + I2C_WRITE); // set device address and write mode
	i2c_write(0x00);
// write data
	i2c_write(bin2bcd(sec));
	i2c_write(bin2bcd(min));
	i2c_write(bin2bcd(hour));
	i2c_write(bin2bcd(dayOfWeek));
	i2c_write(bin2bcd(day));
	i2c_write(bin2bcd(month));
	i2c_write(bin2bcd(year - 2000));
	i2c_stop();
}

#endif

