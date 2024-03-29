/*
 * TouchButton.cpp
 *
 * Renders touch buttons for lcd
 * A button can be a simple clickable text
 * or a box with or without text or even an invisible touch area
 *
 *  Created on:  30.01.2012
 *      Author:  Armin Joachimsmeyer
 *      Email:   armin.joachimsmeyer@gmx.de
 *      License: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *      Version: 1.0.0
 *
 *  LCD interface used:
 * 		getHeight()
 * 		getWidth()
 * 		fillRect()
 * 		drawText()
 * 		FONT_WIDTH
 * 		FONT_HEIGHT
 *
 * 	Ram usage:
 * 		7 byte + 27 bytes per button
 *
 * 	Code size:
 * 		1,5 kByte
 *
 */

#include "TouchButton.h"

// abstraction of interface
#define TOUCH_LCD_WIDTH TFTDisplay.getWidth()
#define TOUCH_LCD_HEIGHT TFTDisplay.getHeight()

#ifndef TOUCHGUI_SAVE_SPACE
MI0283QT2 TouchButton::TFTDisplay;
#endif

TouchButton * TouchButton::sListStart = NULL;
uint8_t TouchButton::sDefaultTouchBorder = TOUCHBUTTON_DEFAULT_TOUCH_BORDER;
uint16_t TouchButton::sDefaultButtonColor = TOUCHBUTTON_DEFAULT_COLOR;
uint16_t TouchButton::sDefaultCaptionColor = TOUCHBUTTON_DEFAULT_CAPTION_COLOR;

TouchButton::~TouchButton() {
	// makes no sense on Arduino
}

TouchButton::TouchButton() {
	mTouchBorder = sDefaultTouchBorder;
	mButtonColor = sDefaultButtonColor;
	mCaptionColor = sDefaultCaptionColor;
	mIsActive = false;
	mOnlyCaption = false;
	mPGMCaption = false;
	mNextObject = NULL;
	if (sListStart == NULL) {
		// first button
		sListStart = this;
	} else {
		// put object in button list
		TouchButton * tObjectPointer = sListStart;
		// search last list element
		while (tObjectPointer->mNextObject != NULL) {
			tObjectPointer = tObjectPointer->mNextObject;
		}
		//insert actual button in last element
		tObjectPointer->mNextObject = this;
	}
}

void TouchButton::setDefaultTouchBorder(const uint8_t aDefaultTouchBorder) {
	sDefaultTouchBorder = aDefaultTouchBorder;
}
void TouchButton::setDefaultButtonColor(const uint16_t aDefaultButtonColor) {
	sDefaultButtonColor = aDefaultButtonColor;
}
void TouchButton::setDefaultCaptionColor(const uint16_t aDefaultCaptionColor) {
	sDefaultCaptionColor = aDefaultCaptionColor;
}

#ifndef TOUCHGUI_SAVE_SPACE
void TouchButton::init(const MI0283QT2 aTheLCD) {
	TFTDisplay = aTheLCD;
}
#endif

/*
 * Set parameters (exept colors aund tochborder size) for touch button
 * if aWidthX == 0 render only text no background box
 * if aCaptionSize == 0 dont render anything, just check touch area -> transparent button ;-)
 */
int8_t TouchButton::initSimpleButton(const uint16_t aPositionX, const uint16_t aPositionY, const uint8_t aWidthX,
		const uint8_t aHeightY, const char * aCaption, const uint8_t aCaptionSize, const int16_t aValue,
		void(*aOnTouchHandler)(TouchButton * const, int16_t)) {
	return initButton(aPositionX, aPositionY, aWidthX, aHeightY, aCaption, aCaptionSize, sDefaultTouchBorder,
			sDefaultButtonColor, sDefaultCaptionColor, aValue, aOnTouchHandler);
}

/*
 * Set parameters (exept colors aund tochborder size) for touch button
 * if aWidthX == 0 render only text no background box
 * if aCaptionSize == 0 dont render anything, just check touch area -> transparent button ;-)
 */
int8_t TouchButton::initSimpleButtonPGM(const uint16_t aPositionX, const uint16_t aPositionY, const uint8_t aWidthX,
		const uint8_t aHeightY, PGM_P aCaption, const uint8_t aCaptionSize, const int16_t aValue,
		void(*aOnTouchHandler)(TouchButton * const, int16_t)) {
	mPGMCaption = true;
	return initButton(aPositionX, aPositionY, aWidthX, aHeightY, aCaption, aCaptionSize, sDefaultTouchBorder,
			sDefaultButtonColor, sDefaultCaptionColor, aValue, aOnTouchHandler);
}

/*
 * Set parameters for touch button
 * if aWidthX == 0 render only text no background box
 * if aCaptionSize == 0 dont render anything, just check touch area -> transparent button ;-)
 */
int8_t TouchButton::initButton(const uint16_t aPositionX, const uint16_t aPositionY, const uint8_t aWidthX,
		const uint8_t aHeightY, const char * aCaption, const uint8_t aCaptionSize, const uint8_t aTouchBorder,
		const uint16_t aButtonColor, const uint16_t aCaptionColor, const int16_t aValue,
		void(*aOnTouchHandler)(TouchButton * const, int16_t)) {

	mWidth = aWidthX;
	mHeight = aHeightY;
	mButtonColor = aButtonColor;
	mCaptionColor = aCaptionColor;
	mTouchBorder = aTouchBorder;
	mCaption = aCaption;
	mCaptionSize = aCaptionSize;
	if (aWidthX == 0) {
		mOnlyCaption = true;
	}
	mOnTouchHandler = aOnTouchHandler;
	mValue = aValue;
//	if (TFTDisplay == NULL) {
//		return TOUCHSLIDER_ERROR_NOT_INITIALIZED;
//	}
	return setPosition(aPositionX, aPositionY);
}

int8_t TouchButton::setPosition(const uint16_t aPositionX, const uint16_t aPositionY) {
	int8_t tRetValue = 0;
	mPositionX = aPositionX;
	mPositionY = aPositionY;
	if (mOnlyCaption) {
		// print only string, no enclosing box
		uint8_t tLength = getCaptionLength((char *) mCaption);
		mPositionXRight = aPositionX + tLength - 1;
		mPositionYBottom = aPositionY + (FONT_HEIGHT * mCaptionSize) - 1;
	} else {
		mPositionXRight = aPositionX + mWidth - 1;
		mPositionYBottom = aPositionY + mHeight - 1;
	}

	// check values
	if (mPositionXRight > TOUCH_LCD_WIDTH) {
		mPositionXRight = TOUCH_LCD_WIDTH - 1;
		tRetValue = TOUCHBUTTON_ERROR_X_RIGHT;
	}
	if (mPositionYBottom >= TOUCH_LCD_HEIGHT) {
		mPositionYBottom = TOUCH_LCD_HEIGHT - 1;
		tRetValue = TOUCHBUTTON_ERROR_Y_BOTTOM;
	}
	return tRetValue;
}

/*
 * renders the button on lcd
 */
int8_t TouchButton::drawButton() {
	if (!mOnlyCaption) {
		// Draw rect
		TFTDisplay.fillRect(mPositionX, mPositionY, mPositionXRight, mPositionYBottom, mButtonColor);
	}
	return drawCaption();
}

/*
 * renders the button on lcd
 */
int8_t TouchButton::drawCaption() {
	mIsActive = true;

	int8_t tRetValue = 0;
	if (mOnlyCaption) {
		// print only string
		TFTDisplay.drawText(mPositionX, mPositionY, (char *) mCaption, mCaptionSize, mCaptionColor, mButtonColor);
	} else if (mCaptionSize > 0) { // dont render anything if caption size == 0
		if (mCaption != NULL) {
			uint16_t tXCaptionPosition;
			uint16_t tYCaptionPosition;
			// try to position the string in the middle of the box
			uint8_t tLength = getCaptionLength((char *) mCaption);
			if (mPositionX + tLength >= mPositionXRight) { // unsigned arithmetic
				// String too long here
				tXCaptionPosition = mPositionX;
				tRetValue = TOUCHBUTTON_ERROR_CAPTION_TOO_LONG;
			} else {
				tXCaptionPosition = mPositionX + 1 + ((mWidth - tLength) / 2);
			}

			if (mPositionY + (FONT_HEIGHT * mCaptionSize) >= mPositionYBottom) { // unsigned arithmetic
				// Font height to big
				tYCaptionPosition = mPositionY;
				tRetValue = TOUCHBUTTON_ERROR_CAPTION_TOO_HIGH;
			} else {
				tYCaptionPosition = mPositionY + 1 + ((mHeight - (FONT_HEIGHT * mCaptionSize)) / 2);
			}
			if (mPGMCaption) {
				TFTDisplay.drawTextPGM(tXCaptionPosition, tYCaptionPosition, (char *) mCaption, mCaptionSize,
						mCaptionColor, mButtonColor);
			} else {
				TFTDisplay.drawText(tXCaptionPosition, tYCaptionPosition, (char *) mCaption, mCaptionSize,
						mCaptionColor, mButtonColor);
			}
		}
	}
	return tRetValue;
}

/*
 * Check if touch event is in button area
 * if yes - call callback function and return true
 * if no - return false
 */
bool TouchButton::checkButton(const uint16_t aTouchPositionX, const uint16_t aTouchPositionY) {
	uint16_t tPositionBorderX = mPositionX - mTouchBorder;
	if (mTouchBorder > mPositionX) {
		tPositionBorderX = 0;
	}
	uint16_t tPositionBorderY = mPositionY - mTouchBorder;
	if (mTouchBorder > mPositionY) {
		tPositionBorderY = 0;
	}
	if (!mIsActive || aTouchPositionX < tPositionBorderX || aTouchPositionX > mPositionXRight + mTouchBorder
			|| aTouchPositionY < tPositionBorderY || aTouchPositionY > mPositionYBottom + mTouchBorder) {
		return false;
	}
	/*
	 *  Touch position is in button - call callback function
	 */
	if (mOnTouchHandler != NULL) {
		mOnTouchHandler(this, mValue);
	}
	return true;
}

/*
 * Static convenience method - checks all buttons for matching touch position.
 */
bool TouchButton::checkAllButtons(const uint16_t aTouchPositionX, const uint16_t aTouchPositionY) {
	TouchButton * tObjectPointer = sListStart;
	// this will crash if no button is created
	if (tObjectPointer->checkButton(aTouchPositionX, aTouchPositionY)) {
		return true;
	}
	// walk through list
	while (tObjectPointer->mNextObject != NULL) {
		tObjectPointer = tObjectPointer->mNextObject;
		if (tObjectPointer->checkButton(aTouchPositionX, aTouchPositionY)) {
			return true;
		}
	}
	return false;
}

/*
 * Static convenience method - deactivate all buttons (e.g. before switching screen)
 */
void TouchButton::deactivateAllButtons() {
	TouchButton * tObjectPointer = sListStart;
	// walk through list
	while (tObjectPointer != NULL) {
		tObjectPointer->deactivate();
		tObjectPointer = tObjectPointer->mNextObject;
	}
}

/*
 * Static convenience method - activate all buttons
 */
void TouchButton::activateAllButtons() {
	TouchButton * tObjectPointer = sListStart;
	// walk through list
	while (tObjectPointer != NULL) {
		tObjectPointer->activate();
		tObjectPointer = tObjectPointer->mNextObject;
	}
}

uint8_t TouchButton::getCaptionLength(char * aCaptionPointer) {
	uint8_t tLength = 0;
	if (mPGMCaption) {
		while (pgm_read_byte(aCaptionPointer++) != 0) {
			tLength += (FONT_WIDTH * mCaptionSize);
		}
	} else {
		while (*aCaptionPointer++ != 0) {
			tLength += (FONT_WIDTH * mCaptionSize);
		}
	}
	return tLength;
}
# ifdef DEBUG
/*
 * for debug purposes
 * needs char aStringBuffer[23+<CaptionLength>]
 */
void TouchButton::toString(char * aStringBuffer) const {
	sprintf(aStringBuffer, "X=%03u Y=%03u X1=%03u Y1=%03u B=%02u %s", mPositionX, mPositionY, mPositionXRight,
			mPositionYBottom, mTouchBorder, mCaption);
}
# endif

const char * TouchButton::getCaption() const {
	return mCaption;
}

/*
 * Set caption
 */
void TouchButton::setCaption(const char * aCaption) {
	mPGMCaption = false;
	mCaption = aCaption;
}
void TouchButton::setCaptionPGM(PGM_P aCaption) {
	mPGMCaption = true;
	mCaption = aCaption;
}

/*
 * changes box color and redraws button
 */
void TouchButton::setColor(const uint16_t aColor) {
	mButtonColor = aColor;
}

void TouchButton::setCaptionColor(const uint16_t aColor) {
	mCaptionColor = aColor;
}

void TouchButton::setValue(const int16_t aValue) {
	mValue = aValue;

}
uint16_t TouchButton::getPositionX() const {
	return mPositionX;
}

uint16_t TouchButton::getPositionY() const {
	return mPositionY;
}
uint16_t TouchButton::getPositionXRight() const {
	return mPositionXRight;
}

uint16_t TouchButton::getPositionYBottom() const {
	return mPositionYBottom;
}

/*
 * activate for touch checking
 */
void TouchButton::activate() {
	mIsActive = true;
}

/*
 * deactivate for touch checking
 */
void TouchButton::deactivate() {
	mIsActive = false;
}

void TouchButton::setTouchBorder(uint8_t touchBorder) {
	mTouchBorder = touchBorder;
}

