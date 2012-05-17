/*
 * TouchButton.h
 *
 * Renders touch buttons for lcd
 * A button can be a simple clickable text
 * or a box with or without text
 *
 *  Created on:  30.01.2012
 *      Author:  Armin Joachimsmeyer
 *      Email:   armin.joachimsmeyer@gmx.de
 *      License: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *      Version: 1.0.0
 */

#ifndef TOUCHBUTTON_H_
#define TOUCHBUTTON_H_

#include "TouchGui.h"

#include <MI0283QT2.h>

#define TOUCHBUTTON_DEFAULT_COLOR 			RGB( 180, 180, 180)
#define TOUCHBUTTON_DEFAULT_CAPTION_COLOR 	COLOR_BLACK
#define TOUCHBUTTON_DEFAULT_TOUCH_BORDER 	2 // extension of touch region
// Error codes
#define TOUCHBUTTON_ERROR_X_RIGHT 			-1
#define TOUCHBUTTON_ERROR_Y_BOTTOM 			-2
#define TOUCHBUTTON_ERROR_CAPTION_TOO_LONG	-3
#define TOUCHBUTTON_ERROR_CAPTION_TOO_HIGH	-4
#define TOUCHBUTTON_ERROR_NOT_INITIALIZED   -64

#ifdef TOUCHGUI_SAVE_SPACE
extern MI0283QT2 TFTDisplay;
#endif

class TouchButton {
public:

	~TouchButton();
	TouchButton();
#ifndef TOUCHGUI_SAVE_SPACE
	static void init(const MI0283QT2 aTheLCD);
#endif

	static void setDefaultTouchBorder(const uint8_t aDefaultTouchBorder);
	static void setDefaultButtonColor(const uint16_t aDefaultButtonColor);
	static void setDefaultCaptionColor(const uint16_t aDefaultCaptionColor);
	static bool checkAllButtons(const uint16_t aTouchPositionX, const uint16_t aTouchPositionY);
	static void activateAllButtons();
	static void deactivateAllButtons();

	int8_t initSimpleButton(const uint16_t aPositionX, const uint16_t aPositionY, const uint8_t aWidthX,
			const uint8_t aHeightY, const char *aCaption, const uint8_t aCaptionSize, const int16_t aValue,
			void (*aOnTouchHandler)(TouchButton* const, int16_t));
	int8_t initSimpleButtonPGM(const uint16_t aPositionX, const uint16_t aPositionY, const uint8_t aWidthX,
			const uint8_t aHeightY, PGM_P aCaption, const uint8_t aCaptionSize, const int16_t aValue,
			void(*aOnTouchHandler)(TouchButton * const, int16_t));
	int8_t initButton(const uint16_t aPositionX, const uint16_t aPositionY, const uint8_t aWidthX,
			const uint8_t aHeightY, const char *aCaption, const uint8_t aCaptionSize, const uint8_t aTouchBorder,
			const uint16_t aButtonColor, const uint16_t aCaptionColor, const int16_t aValue,
			void (*aOnTouchHandler)(TouchButton* const, int16_t));
	bool checkButton(const uint16_t aTouchPositionX, const uint16_t aTouchPositionY);
	int8_t drawButton(void);
	int8_t drawCaption(void);
	int8_t setPosition(const uint16_t aPositionX, const uint16_t aPositionY);
	void setColor(const uint16_t aColor);
	void setCaption(const char *aCaption);
	void setCaptionPGM(PGM_P aCaption);
	void setCaptionColor(const uint16_t aColor);
	void setValue(const int16_t aValue);
	const char *getCaption(void) const;
	uint16_t getPositionX(void) const;
	uint16_t getPositionY(void) const;
	uint16_t getPositionXRight(void) const;
	uint16_t getPositionYBottom(void) const;
	void activate();
	void deactivate();
	void toString(char *aStringBuffer) const;
	uint8_t getTouchBorder() const;
	void setTouchBorder(uint8_t const touchBorder);
private:
#ifndef TOUCHGUI_SAVE_SPACE
	static MI0283QT2 TFTDisplay;
#endif
	static TouchButton *sListStart;
	static uint16_t sDefaultButtonColor;
	static uint16_t sDefaultCaptionColor;
	static uint8_t sDefaultTouchBorder;

	uint16_t mButtonColor;
	uint16_t mCaptionColor;
	uint16_t mPositionX;
	uint16_t mPositionY;
	uint8_t mWidth;
	uint8_t mHeight;
	uint16_t mPositionXRight;
	uint16_t mPositionYBottom;
	bool mOnlyCaption;
	bool mPGMCaption;
	const char *mCaption;
	uint8_t mCaptionSize;
	uint8_t mTouchBorder;
	int16_t mValue;
	bool mIsActive;
	TouchButton *mNextObject;

protected:
	uint8_t getCaptionLength(char * aCaptionPointer);
	void (*mOnTouchHandler)(TouchButton * const, int16_t);

};

#endif /* TOUCHBUTTON_H_ */
