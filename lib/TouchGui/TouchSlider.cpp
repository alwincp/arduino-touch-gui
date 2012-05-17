/*
 * TouchSlider.cpp
 *
 * Slider which returns value as unsigned byte value
 *
 *  Created on: 30.01.2012
 *      Author: Armin Joachimsmeyer
 *      Email:   armin.joachimsmeyer@gmx.de
 *      License: GPL v3 (http://www.gnu.org/licenses/gpl.html)
 *      Version: 1.0.0
 *
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
 * 		15 byte + 39 bytes per slider
 *
 * 	Code size:
 * 		2,8 kByte
 *
 */

#include "TouchSlider.h"

// abstraction of interface
#define TOUCH_LCD_WIDTH TFTDisplay.getWidth()
#define TOUCH_LCD_HEIGHT TFTDisplay.getHeight()

#ifndef TOUCHGUI_SAVE_SPACE
MI0283QT2 TouchSlider::TFTDisplay;
#endif

TouchSlider * TouchSlider::sListStart = NULL;
uint16_t TouchSlider::sDefaultSliderColor = TOUCHSLIDER_DEFAULT_SLIDER_COLOR;
uint16_t TouchSlider::sDefaultBarColor = TOUCHSLIDER_DEFAULT_BAR_COLOR;
uint16_t TouchSlider::sDefaultBarThresholdColor = TOUCHSLIDER_DEFAULT_BAR_THRESHOLD_COLOR;
uint16_t TouchSlider::sDefaultBarBackgroundColor = TOUCHSLIDER_DEFAULT_BAR_BACK_COLOR;
uint16_t TouchSlider::sDefaultCaptionColor = TOUCHSLIDER_DEFAULT_CAPTION_COLOR;
uint16_t TouchSlider::sDefaultValueColor = TOUCHSLIDER_DEFAULT_VALUE_COLOR;
uint16_t TouchSlider::sDefaultValueCaptionBackgroundColor = TOUCHSLIDER_DEFAULT_CAPTION_VALUE_BACK_COLOR;

int8_t TouchSlider::sDefaultTouchBorder = TOUCHSLIDER_DEFAULT_TOUCH_BORDER;

TouchSlider::~TouchSlider() {
//makes no sense on Arduino
}

TouchSlider::TouchSlider() {
	mBarThresholdColor = sDefaultBarThresholdColor;
	mBarBackgroundColor = sDefaultBarBackgroundColor;
	mCaptionColor = sDefaultCaptionColor;
	mValueColor = sDefaultValueColor;
	mValueCaptionBackgroundColor = sDefaultValueCaptionBackgroundColor;
	mTouchBorder = sDefaultTouchBorder;
	mNextObject = NULL;
	if (sListStart == NULL) {
		// first button
		sListStart = this;
	} else {
		// put object in button list
		TouchSlider * tObjectPointer = sListStart;
		// search last list element
		while (tObjectPointer->mNextObject != NULL) {
			tObjectPointer = tObjectPointer->mNextObject;
		}
		//insert actual button in last element
		tObjectPointer->mNextObject = this;
	}
}

#ifndef TOUCHGUI_SAVE_SPACE
/*
 * Static initialization of slider
 */
void TouchSlider::init(const MI0283QT2 aTheLCD) {
	TFTDisplay = aTheLCD;
}
#endif

/*
 * Static initialization of slider default colors
 */
void TouchSlider::setDefaults(const int8_t aDefaultTouchBorder, const uint16_t aDefaultSliderColor,
		const uint16_t aDefaultBarColor, const uint16_t aDefaultBarThresholdColor,
		const uint16_t aDefaultBarBackgroundColor, const uint16_t aDefaultCaptionColor,
		const uint16_t aDefaultValueColor, const uint16_t aDefaultValueCaptionBackgroundColor) {
	sDefaultSliderColor = aDefaultSliderColor;
	sDefaultBarColor = aDefaultBarColor;
	sDefaultBarThresholdColor = aDefaultBarThresholdColor;
	sDefaultBarBackgroundColor = aDefaultBarBackgroundColor;
	sDefaultCaptionColor = aDefaultCaptionColor;
	sDefaultValueColor = aDefaultValueColor;
	sDefaultValueCaptionBackgroundColor = aDefaultValueCaptionBackgroundColor;
	sDefaultTouchBorder = aDefaultTouchBorder;
}

void TouchSlider::setDefaultSliderColor(const uint16_t aDefaultSliderColor) {
	sDefaultSliderColor = aDefaultSliderColor;
}

void TouchSlider::setDefaultBarColor(const uint16_t aDefaultBarColor) {
	sDefaultBarColor = aDefaultBarColor;
}

void TouchSlider::initSliderColors(const uint16_t aSliderColor, const uint16_t aBarColor,
		const uint16_t aBarThresholdColor, const uint16_t aBarBackgroundColor, const uint16_t aCaptionColor,
		const uint16_t aValueColor, const uint16_t aValueCaptionBackgroundColor) {
	mSliderColor = aSliderColor;
	mBarColor = aBarColor;
	mBarThresholdColor = aBarThresholdColor;
	mBarBackgroundColor = aBarBackgroundColor;
	mCaptionColor = aCaptionColor;
	mValueColor = aValueColor;
	mValueCaptionBackgroundColor = aValueCaptionBackgroundColor;
}

/*
 * Static convenience method - activate all sliders
 */
void TouchSlider::activateAllSliders() {
	TouchSlider * tObjectPointer = sListStart;
	while (tObjectPointer != NULL) {
		tObjectPointer->activate();
		tObjectPointer = tObjectPointer->mNextObject;
	}
}

/*
 * Static convenience method - deactivate all sliders
 */
void TouchSlider::deactivateAllSliders() {
	TouchSlider * tObjectPointer = sListStart;
	while (tObjectPointer != NULL) {
		tObjectPointer->deactivate();
		tObjectPointer = tObjectPointer->mNextObject;
	}
}
/**
 *  for simple predefined slider
 */
int8_t TouchSlider::initSimpleSlider(const uint16_t aPositionX, const uint16_t aPositionY, const uint8_t aSizeX,
		const char * aCaption, const bool aShowValue, uint8_t (*aOnChangeHandler)(TouchSlider * const, const uint8_t),
		const char * (*aValueHandler)(uint8_t)) {
	return initSlider(aPositionX, aPositionY, aSizeX, TOUCHSLIDER_DEFAULT_MAX_VALUE, true, aCaption,
			(TOUCHSLIDER_DEFAULT_MAX_VALUE / 4) * 3, TOUCHSLIDER_DEFAULT_THRESHOLD_VALUE, aShowValue,
			sDefaultTouchBorder, aOnChangeHandler, aValueHandler);
}

/**
 *  with all parameters except color
 *  position determines upper left corner
 *  returns false if parameters are not consistent ie. are internally modified
 *  or if not enough space to draw caption or value
 */
int8_t TouchSlider::initSlider(const uint16_t aPositionX, const uint16_t aPositionY, const uint8_t aSizeX,
		const uint8_t aMaxValue, const bool aShowBorder, const char * aCaption, const uint8_t aInitalValue,
		const uint8_t aThresholdValue, const bool aShowValue, const int8_t aTouchBorder,
		uint8_t (*aOnChangeHandler)(TouchSlider * const, const uint8_t), const char * (*aValueHandler)(uint8_t)) {

	int8_t tRetValue = 0;

	mSliderColor = sDefaultSliderColor;
	mBarColor = sDefaultBarColor;
	/**
	 * Copy parameter
	 */
	mPositionX = aPositionX;
	mPositionY = aPositionY;
	mShowBorder = aShowBorder;
	mCaption = aCaption;
	mShowValue = aShowValue;
	mSize = aSizeX;
	mMaxValue = aMaxValue;
	mActualValue = aInitalValue;
	mThresholdValue = aThresholdValue;
	mTouchBorder = aTouchBorder;
	mOnChangeHandler = aOnChangeHandler;
	mValueHandler = aValueHandler;

	tRetValue = checkParameterValues();

	uint8_t tSizeOfBorders = 2 * mSize;
	if (!mShowBorder) {
		tSizeOfBorders = 0;
	}

	/*
	 * compute lower right corner and validate
	 */
	mPositionXRight = mPositionX + ((tSizeOfBorders + mSize) * TOUCHSLIDER_SIZE_FACTOR) - 1;
	if (mPositionXRight >= TOUCH_LCD_WIDTH) {
		// simple fallback
		mSize = 1;
		mPositionX = 0;
		mPositionXRight = mPositionX + ((tSizeOfBorders + mSize) * TOUCHSLIDER_SIZE_FACTOR) - 1;
		tRetValue = TOUCHSLIDER_ERROR_X_RIGHT;
	}
	mPositionYBottom = mPositionY + mMaxValue + tSizeOfBorders - 1;
	if (mPositionYBottom >= TOUCH_LCD_HEIGHT) {
		// simple fallback
		mSize = 1;
		mPositionY = 0;
		mPositionYBottom = mPositionY + mMaxValue + 1;
		tRetValue = TOUCHSLIDER_ERROR_Y_BOTTOM;
	}
	return tRetValue;
}

int8_t TouchSlider::drawSlider() {
	mIsActive = true;

	if (mShowBorder) {
		drawBorder();
	}

	// Fill middle bar with initial value
	drawBar();

	int8_t tRetValue = printCaption();
	if (tRetValue < 0) {
		// value will be also unable to print -> just return now
		return tRetValue;
	}
	// Print value as string
	return printValue();
}

void TouchSlider::drawBorder() {
// Create left border
	TFTDisplay.fillRect(mPositionX, mPositionY, mPositionX + (TOUCHSLIDER_SIZE_FACTOR * mSize) - 1, mPositionYBottom,
			mSliderColor);
// Create right border
	TFTDisplay.fillRect(mPositionX + (2 * TOUCHSLIDER_SIZE_FACTOR * mSize), mPositionY, mPositionXRight,
			mPositionYBottom, mSliderColor);

// Create value bar upper border
	TFTDisplay.fillRect(mPositionX + (TOUCHSLIDER_SIZE_FACTOR * mSize), mPositionY,
			mPositionX + (2 * TOUCHSLIDER_SIZE_FACTOR * mSize) - 1, mPositionY + mSize - 1, mSliderColor);

// Create value bar lower border
	TFTDisplay.fillRect(mPositionX + (TOUCHSLIDER_SIZE_FACTOR * mSize), mPositionYBottom - mSize + 1,
			mPositionX + (2 * TOUCHSLIDER_SIZE_FACTOR * mSize) - 1, mPositionYBottom, mSliderColor);
}

/*
 * (re)draws the middle bar according to actual value
 */
void TouchSlider::drawBar() {

	uint8_t tBorderSize = 0;
	if (mShowBorder) {
		tBorderSize = mSize;
	}
// draw background bar
	if (mActualValue < mMaxValue) {
		TFTDisplay.fillRect(mPositionX + (tBorderSize * TOUCHSLIDER_SIZE_FACTOR), mPositionY + tBorderSize,
				mPositionX + ((tBorderSize + mSize) * TOUCHSLIDER_SIZE_FACTOR) - 1,
				mPositionYBottom - tBorderSize - mActualValue, mBarBackgroundColor);
	}

// Draw value bar
	if (mActualValue > 0) {
		uint16_t tColor = mBarColor;
		if (mActualValue > mThresholdValue) {
			tColor = mBarThresholdColor;
		}
		TFTDisplay.fillRect(mPositionX + (tBorderSize * TOUCHSLIDER_SIZE_FACTOR),
				mPositionYBottom - tBorderSize - mActualValue + 1,
				mPositionX + ((tBorderSize + mSize) * TOUCHSLIDER_SIZE_FACTOR) - 1, mPositionYBottom - tBorderSize,
				tColor);
	}
}

/*
 * Print caption
 */
int8_t TouchSlider::printCaption() {
	if (mCaption == NULL) {
		return 0;
	}
	uint16_t tLength = 0;
	char * tPtr = (char *) mCaption;
	while (*tPtr != 0) {
		tPtr++;
		tLength += FONT_WIDTH;
	}
	if (tLength == 0) {
		mCaption = NULL;
		return TOUCHSLIDER_ERROR_CAPTION_LENGTH;
	}

	uint8_t tButtonWidth = mSize * TOUCHSLIDER_SIZE_FACTOR;
	if (mShowBorder) {
		tButtonWidth = 3 * tButtonWidth;
	}
// try to position the string in the middle below slider
	uint16_t tXOffset = (tButtonWidth / 2) - (tLength / 2);
	uint16_t tCaptionPositionX = mPositionX + tXOffset;
// unsigned arithmetic overflow handling
	if (tCaptionPositionX > mPositionXRight) {
		tCaptionPositionX = 0;
	}
// space for caption?
	uint16_t tCaptionPositionY = mPositionYBottom + mSize;
	if ((tCaptionPositionY + FONT_HEIGHT) > TOUCH_LCD_HEIGHT) {
		// no space for caption
		return TOUCHSLIDER_ERROR_CAPTION_HEIGTH;
	}

	TFTDisplay.drawText(tCaptionPositionX, tCaptionPositionY, (char *) mCaption, 1, mCaptionColor,
			mValueCaptionBackgroundColor);
	return 0;

}
/*
 * Print value left aligned to slider below caption (if existent)
 */
int8_t TouchSlider::printValue() {
	if (!mShowValue) {
		return 0;
	}
	const char * pValueAsString;
	uint16_t tValuePositionY;
	if (mCaption != NULL) {
		tValuePositionY = mPositionYBottom + mSize + FONT_HEIGHT;
	} else {
		tValuePositionY = mPositionYBottom + mSize;
	}

	if ((tValuePositionY + FONT_HEIGHT) > TOUCH_LCD_HEIGHT) {
		// no space for caption
		mShowValue = false;
		return TOUCHSLIDER_ERROR_VALUE_TOO_HIGH;
	}
	if (mValueHandler == NULL) {
		char tValueAsString[4];
		pValueAsString = &tValueAsString[0];
		sprintf(tValueAsString, "%03u", mActualValue);
	} else {
		// mValueHandler has to provide the char array
		pValueAsString = mValueHandler(mActualValue);
	}
	TFTDisplay.drawText(mPositionX, tValuePositionY, (char *) pValueAsString, 1, mValueColor,
			mValueCaptionBackgroundColor);
	return 0;
}

/*
 * Check if touch event is in slider
 * if yes - set bar and value call callback function and return true
 * if no - return false
 */
bool TouchSlider::checkSlider(const uint16_t aTouchPositionX, const uint16_t aTouchPositionY) {
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
	uint8_t tHorizontalBorderSize = 0;
	if (mShowBorder) {
		tHorizontalBorderSize = mSize;
	}
	/*
	 *  Touch position is in slider (plus additional touch border) here
	 */
// adjust value according to size of upper and lower border
	uint8_t tActualTouchValue;
	if (aTouchPositionY > (mPositionYBottom - tHorizontalBorderSize)) {
		tActualTouchValue = 0;
	} else if (aTouchPositionY < (mPositionY + tHorizontalBorderSize)) {
		tActualTouchValue = mMaxValue;
	} else {
		tActualTouchValue = mPositionYBottom - tHorizontalBorderSize - aTouchPositionY + 1;
	}

	if (tActualTouchValue != mActualTouchValue) {
		mActualTouchValue = tActualTouchValue;
		if (mOnChangeHandler != NULL) {
			// call change handler and take the result as new value
			tActualTouchValue = mOnChangeHandler(this, tActualTouchValue);

			if (tActualTouchValue == mActualValue) {
				// value returned didnd change - do nothing
				return true;
			}
			if (tActualTouchValue > mMaxValue) {
				tActualTouchValue = mMaxValue;
			}
		}
		// value changed - store and redraw
		mActualValue = tActualTouchValue;
		drawBar();
		printValue();
	}
	return true;
}

/*
 * Static convenience method - checks all buttons in  array sManagesButtonsArray for events.
 */
bool TouchSlider::checkAllSliders(const uint16_t aTouchPositionX, const uint16_t aTouchPositionY) {
	TouchSlider * tObjectPointer = sListStart;
// this breaks if no button is created
	if (tObjectPointer->checkSlider(aTouchPositionX, aTouchPositionY)) {
		return true;
	}
// walk through list
	while (tObjectPointer->mNextObject != NULL) {
		tObjectPointer = tObjectPointer->mNextObject;
		if (tObjectPointer->checkSlider(aTouchPositionX, aTouchPositionY)) {
			return true;
		}
	}
	return false;
}

int8_t TouchSlider::getActualValue() const {
	return mActualValue;
}

/*
 * also redraws value bar
 */
void TouchSlider::setActualValue(int8_t actualValue) {
	mActualValue = actualValue;
	drawBar();
	printValue();
}

uint16_t TouchSlider::getPositionXRight() const {
	return mPositionXRight;
}

uint16_t TouchSlider::getPositionYBottom() const {
	return mPositionYBottom;
}

void TouchSlider::activate() {
	mIsActive = true;
}
void TouchSlider::deactivate() {
	mIsActive = false;
}

int8_t TouchSlider::checkParameterValues() {
	/**
	 * Check and copy parameter
	 */
	uint16_t tScreenWidth = TOUCH_LCD_WIDTH;
	uint16_t tScreenHeight = TOUCH_LCD_HEIGHT;
	int8_t tRetValue = 0;

	if (mPositionX >= (tScreenWidth - (3 * TOUCHSLIDER_SIZE_FACTOR))) {
		// simple fallback
		mPositionX = 0;
		tRetValue = TOUCHSLIDER_ERROR_POS_X;
	}

	if (mPositionY >= (tScreenHeight - 3)) {
		// simple fallback
		mPositionY = 0;
		tRetValue = TOUCHSLIDER_ERROR_POS_Y;
	}

	if (mSize == 0) {
		mSize = TOUCHSLIDER_DEFAULT_SIZE;
		tRetValue = TOUCHSLIDER_ERROR_SIZE_ZERO;
	} else if (mSize > 20) {
		mSize = TOUCHSLIDER_DEFAULT_SIZE;
		tRetValue = TOUCHSLIDER_ERROR_SIZE;
	}
	if (mMaxValue == 0) {
		tRetValue = TOUCHSLIDER_ERROR_MAX_VALUE;
		mMaxValue = 1;
	}
	if (mActualValue > mMaxValue) {
		tRetValue = TOUCHSLIDER_ERROR_ACTUAL_VALUE;
		mActualValue = mMaxValue;
	}

	return tRetValue;
}

void TouchSlider::setBarThresholdColor(uint16_t barThresholdColor) {
	mBarThresholdColor = barThresholdColor;
}

void TouchSlider::setSliderColor(uint16_t sliderColor) {
	mSliderColor = sliderColor;
}

void TouchSlider::setBarColor(uint16_t barColor) {
	mBarColor = barColor;
}

