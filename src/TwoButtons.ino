/*
 * TouchGui.ino
 *
 *  Created on: 31.01.2012
 *      Author: Armin Joachimsmeyer
 *      Email:   armin.joachimsmeyer@gmx.de
 *      License: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *      Version: 1.0.0
 *
 *      Demo of the TouchButton lib
 *
 *      For Arduino Uno
 *      with mSD-Shield and MI0283QT Adapter from www.watterott.net
 *      and the ADS7846 and MI0283QT2 libs from
 *		https://github.com/watterott/mSD-Shield/downloads
 *
 */

#include <Arduino.h>

//#define DEBUG // enable debug output
#define RTC_EXISTS  //if a DS1307 is connected to the I2C bus
#define DISPLAY_HEIGHT 240
#define DISPLAY_WIDTH 320
#include <MI0283QT2.h>
#include <ADS7846.h>
#include <TouchButton.h>
#include <TouchButtonAutorepeat.h>

#ifdef RTC_EXISTS
#ifdef __cplusplus
extern "C" {
#include <i2cmaster.h>
}
#endif
#define DS1307_ADDR 0xD0 // 0x68 shifted left
uint8_t bcd2bin(uint8_t val);
uint8_t bin2bcd(uint8_t val);
void showRTCTime(void);
void setRTCTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dayOfWeek, uint8_t day, uint8_t month, uint16_t year);
#endif

/*
 * LCD and touch panel stuff
 */
#define TP_EEPROMADDR (E2END -1 - sizeof(CAL_MATRIX)) //eeprom address for calibration data - 28 bytes
#ifdef DEBUG
void printTPData(void);
#endif
void printRGB(const uint16_t aColor, const uint16_t aXPos, const uint16_t aYPos);


MI0283QT2 TFTDisplay;
ADS7846 TouchPanel;

bool StartNewTouch = true;
#define BACKGROUND_COLOR COLOR_WHITE

/*
 * Loop control
 */

// a string buffer for any purpose...
char StringBuffer[128];

#define BUTTON_WIDTH 180
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 30

#define CAPTION_COLOR_INCREMENT RGB(0x08,0x20,0x60)
#define BUTTON_BACKGROUND_COLOR_INCREMENT RGB(0x08,0x10,0x40)
TouchButtonAutorepeat TouchButtonCaptionAutorepeat;
TouchButton TouchButtonBackground;

// Callback touch handler
void doButtons(TouchButton * const aTheTochedButton, int aValue);

void setup() {

	//Serial.begin(115200);
	//init display
	TFTDisplay.init(4); //spi-clk = Fcpu/4
	TFTDisplay.clear(BACKGROUND_COLOR);

	//init touch controller
	TouchPanel.init();
	//touch-panel calibration
	TouchPanel.service();
	if (TouchPanel.getPressure() > 5) {
		//clear screen
		TouchPanel.doCalibration(&TFTDisplay, TP_EEPROMADDR, 0); //dont check EEPROM for calibration data
	} else {
		TouchPanel.doCalibration(&TFTDisplay, TP_EEPROMADDR, 1); //check EEPROM for calibration data
	}

	// Create  2 buttons
	int8_t tErrorValue = 0;
	tErrorValue += TouchButtonCaptionAutorepeat.initButton(20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, "Caption", 2,
			TOUCHBUTTON_DEFAULT_TOUCH_BORDER, TOUCHBUTTON_DEFAULT_COLOR, COLOR_BLUE, COLOR_BLUE, &doButtons);
	TouchButtonCaptionAutorepeat.setButtonAutorepeatTiming(1000, 200, 23000, 50, &StartNewTouch);
	tErrorValue += TouchButtonBackground.initButton(20,
			TouchButtonCaptionAutorepeat.getPositionYBottom() + BUTTON_SPACING, BUTTON_WIDTH, BUTTON_HEIGHT,
			"Background", 2, TOUCHBUTTON_DEFAULT_TOUCH_BORDER, COLOR_BLUE, TOUCHBUTTON_DEFAULT_CAPTION_COLOR,
			COLOR_BLUE, &doButtons);
	TouchButtonCaptionAutorepeat.drawButton();
	TouchButtonBackground.drawButton();

	if (tErrorValue != 0) {
		// Show Error
		TFTDisplay.clear(BACKGROUND_COLOR);
		TFTDisplay.drawText(40, 100, (char *) "Error on button rendering", 2, COLOR_RED, BACKGROUND_COLOR);
		TFTDisplay.drawText(40, 120, tErrorValue, 2, COLOR_RED, BACKGROUND_COLOR);
		delay(5000);
	}
	// activate internal pullups for twi.
	digitalWrite(SDA, 1);
	digitalWrite(SCL, 1);
	i2c_init(); //Set speed
	setRTCTime(30, 19, 19, 6, 28, 4, 2012); //08:00:00 24.12.2011 //sec, min, hour, ,dayOfWeek, day, month, year

}

void loop() {
	bool tGuiTouched = false;

	//service routine for touch panel
	TouchPanel.service();
#ifdef DEBUG
	printTPData();
#endif
	//touch press?
	if (TouchPanel.getPressure() > 6) {
		/*
		 * check if button is touched
		 */
		tGuiTouched = TouchButton::checkAllButtons(TouchPanel.getX(), TouchPanel.getY());
		if (tGuiTouched) {
//			printTPData();
		}
		StartNewTouch = false;
	} else {
		StartNewTouch = true;
	}
#ifdef RTC_EXISTS
	showRTCTime();
#endif
}

void doButtons(TouchButton * const aTheTouchedButton, int aValue) {
	printRGB(aValue, 10, 200);
	if (aTheTouchedButton == &TouchButtonCaptionAutorepeat) {
		aValue += CAPTION_COLOR_INCREMENT;
		aTheTouchedButton->setCaptionColor(aValue);
		aTheTouchedButton->setValue(aValue);
		aTheTouchedButton->drawCaption();
		return;
	}
	if (aTheTouchedButton == &TouchButtonBackground && StartNewTouch) {
		aValue += BUTTON_BACKGROUND_COLOR_INCREMENT;
		aTheTouchedButton->setColor(aValue);
		aTheTouchedButton->setValue(aValue);
		aTheTouchedButton->drawButton();
		return;
	}
}


//show rgb data of touched button
void printRGB(const uint16_t aColor, uint16_t aXPos, const uint16_t aYPos) {
	//RED
	sprintf(StringBuffer, "R=%02X", (aColor & 0xF800) >> 8);
	aXPos = TFTDisplay.drawText(aXPos, aYPos, StringBuffer, 2, COLOR_RED, BACKGROUND_COLOR);
	//GREEN
	sprintf(StringBuffer, "G=%02X", (aColor & 0x07E0) >> 3);
	aXPos += 2 * FONT_WIDTH;
	aXPos = TFTDisplay.drawText(aXPos, aYPos, StringBuffer, 2, COLOR_GREEN, BACKGROUND_COLOR);
	//BLUE
	sprintf(StringBuffer, "B=%02X", (aColor & 0x001F) << 3);
	aXPos += 2 * FONT_WIDTH;
	TFTDisplay.drawText(aXPos, aYPos, StringBuffer, 2, COLOR_BLUE, BACKGROUND_COLOR);
}

#ifdef DEBUG
//show touchpanel data
void printTPData(void) {
	sprintf(StringBuffer, "X:%03i|%04i Y:%03i|%04i P:%03i", TouchPanel.getX(), TouchPanel.getXraw(), TouchPanel.getY(),
			TouchPanel.getYraw(), TouchPanel.getPressure());
	TFTDisplay.drawText(20, 2, StringBuffer, 1, COLOR_BLACK, BACKGROUND_COLOR);
}
#endif

#ifdef RTC_EXISTS

uint8_t bcd2bin(uint8_t val) {
	return val - 6 * (val >> 4);
}
uint8_t bin2bcd(uint8_t val) {
	return val + 6 * (val / 10);
}

#define TIMEOUT 500
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

