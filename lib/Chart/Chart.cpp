/*
 * Chart.cpp
 *
 * Draws charts for LCD screen
 * Charts have axes and a data area
 * Data can be printed as pixels or line or area
 * Labels and grid are optional
 *
 *  Created on:  14.02.2012
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
 * 		drawPixel()
 * 		drawLine()
 * 		FONT_WIDTH
 * 		FONT_HEIGHT
 *
 * 	Ram usage:
 * 		2 Byte + 56 Bytes per chart
 *
 * 	Code size:
 * 		3 kByte
 *
 */

#include "Chart.h"

// abstraction of interface
#define TOUCH_LCD_WIDTH sTheLcd.getWidth()
#define TOUCH_LCD_HEIGHT sTheLcd.getHeight()

MI0283QT2 Chart::sTheLcd;

Chart::~Chart() {
//makes no sense on Arduino
}

Chart::Chart() {
	mChartBackgroundColor = CHART_DEFAULT_BACKGROUND_COLOR;
	mAxesColor = CHART_DEFAULT_AXES_COLOR;
	mGridColor = CHART_DEFAULT_GRID_COLOR;
	mLabelColor = CHART_DEFAULT_LABEL_COLOR;
	mXLabelIncrementValue = 0;
	mYLabelIncrementValue = 0;
	mXLabelIncrementValueFloat = 0;
	mYLabelIncrementValueFloat = 0;
}

void Chart::init(const MI0283QT2 aTheLCD) {
	sTheLcd = aTheLCD;
}

void Chart::initChartColors(const uint16_t aAxesColor, const uint16_t aGridColor, const uint16_t aLabelColor,
		const uint16_t aBackgroundColor) {
	mAxesColor = aAxesColor;
	mGridColor = aGridColor;
	mLabelColor = aLabelColor;
	mChartBackgroundColor = aBackgroundColor;
}

/*
 * aPositionX and aPositionY are the 0 coordinates of the grid
 */
uint8_t Chart::initChart(const uint16_t aPositionX, const uint16_t aPositionY, const uint16_t aWidthX,
		const uint16_t aHeightY, const uint8_t aAxesSize, const bool aHasGrid, const uint8_t aGridXResolution,
		const uint8_t aGridYResolution) {
	mPositionX = aPositionX;
	mPositionY = aPositionY;
	mWidthX = aWidthX;
	mHeightY = aHeightY;
	mAxesSize = aAxesSize;
	mHasGrid = aHasGrid;
	mGridXResolution = aGridXResolution;
	mGridYResolution = aGridYResolution;

	return checkParameterValues();
}

uint8_t Chart::checkParameterValues() {
	uint8_t tRetValue = 0;
	// also check for zero :-)
	if (mAxesSize - 1 > CHART_MAX_AXES_SIZE) {
		mAxesSize = CHART_MAX_AXES_SIZE;
		tRetValue = CHART_ERROR_AXES_SIZE;
	}
	uint16_t t2AxesSize = 2 * mAxesSize;
	if (mPositionX < t2AxesSize - 1) {
		mPositionX = t2AxesSize - 1;
		mWidthX = 100;
		tRetValue = CHART_ERROR_POS_X;
	}
	if (mPositionY > TOUCH_LCD_HEIGHT - t2AxesSize) {
		mPositionY = TOUCH_LCD_HEIGHT - t2AxesSize;
		tRetValue = CHART_ERROR_POS_Y;
	}
	if (mPositionX + mWidthX > TOUCH_LCD_WIDTH) {
		mPositionX = 0;
		mWidthX = 100;
		tRetValue = CHART_ERROR_WIDTH;
	}

	if (mHeightY > mPositionY + 1) {
		mHeightY = mPositionY + 1;
		tRetValue = CHART_ERROR_HEIGHT;
	}

	if (mGridXResolution > mWidthX) {
		mGridXResolution = mWidthX / 2;
		tRetValue = CHART_ERROR_GRID_X_RESOLUTION;
	}
	return tRetValue;

}

void Chart::initXLabelInt(const int aXLabelStartValue, const int aXLabelIncrementValue,
		const uint8_t aXMinStringWidth) {
	mXLabelStartValue = aXLabelStartValue;
	mXLabelIncrementValue = aXLabelIncrementValue;
	mXMinStringWidth = aXMinStringWidth;
}

void Chart::initXLabelFloat(const float aXLabelStartValue, const float aXLabelIncrementValue,
		uint8_t aXMinStringWidthIncDecimalPoint, uint8_t aXNumVarsAfterDecimal) {
	mXLabelStartValueFloat = aXLabelStartValue;
	mXLabelIncrementValueFloat = aXLabelIncrementValue;
	mXNumVarsAfterDecimal = aXNumVarsAfterDecimal;
	mXMinStringWidth = aXMinStringWidthIncDecimalPoint;
}

void Chart::initYLabelInt(const int aYLabelStartValue, const int aYLabelIncrementValue,
		const uint8_t aYMinStringWidth) {
	mYLabelStartValue = aYLabelStartValue;
	mYLabelIncrementValue = aYLabelIncrementValue;
	mYMinStringWidth = aYMinStringWidth;
}
void Chart::initYLabelFloat(const float aYLabelStartValue, const float aYLabelIncrementValue,
		uint8_t aYMinStringWidthIncDecimalPoint, uint8_t aYNumVarsAfterDecimal) {
	mYLabelStartValueFloat = aYLabelStartValue;
	mYLabelIncrementValueFloat = aYLabelIncrementValue;
	mYNumVarsAfterDecimal = aYNumVarsAfterDecimal;
	mYMinStringWidth = aYMinStringWidthIncDecimalPoint;
}

/*
 * Render the chart on the lcd
 */
uint8_t Chart::drawChart(void) {
	uint8_t tRetValue = drawAxes(false);
	drawGrid();
	return tRetValue;
}

void Chart::drawGrid(void) {
	if (!mHasGrid) {
		return;
	}
	uint16_t tOffset;
// draw vertical lines
	for (tOffset = mGridXResolution; tOffset <= mWidthX; tOffset += mGridXResolution) {
		sTheLcd.fillRect(mPositionX + tOffset, mPositionY - 1, mPositionX + tOffset, mPositionY - mHeightY + 1,
				mGridColor);
	}
// draw horizontal lines
	for (tOffset = mGridYResolution; tOffset <= mHeightY; tOffset += mGridYResolution) {
		sTheLcd.fillRect(mPositionX + 1, mPositionY - tOffset, mPositionX + mWidthX - 1, mPositionY - tOffset,
				mGridColor);
	}

}

/*
 * render axes
 * renders indicators if labels but no grid are specified
 * render labels only if at least one increment value != 0
 * returns -11 in no space for X labels or - 12 if no space for Y labels on lcd
 */
uint8_t Chart::drawAxes(bool aClearLabelsBefore) {
	uint8_t tRetValue = drawXAxis(aClearLabelsBefore);
	tRetValue += drawYAxis(aClearLabelsBefore);
	return tRetValue;
}

/**
 * render X AXIS only if integer or float increment value != 0
 */
uint8_t Chart::drawXAxis(bool aClearLabelsBefore) {

	char tLabelStringBuffer[32];


// draw X line
	sTheLcd.fillRect(mPositionX - mAxesSize + 1, mPositionY, mPositionX + mWidthX - 1, mPositionY + mAxesSize - 1,
			mAxesColor);

	if (mXLabelIncrementValue != 0 || mXLabelIncrementValueFloat != 0.0) {
		uint16_t tOffset;
		uint16_t tNumberYTop = mPositionY + mAxesSize + 1;
		if (!mHasGrid) {
			tNumberYTop += mAxesSize;
			// draw indicators
			for (tOffset = 0; tOffset <= mWidthX; tOffset += mGridXResolution) {
				sTheLcd.fillRect(mPositionX + tOffset, mPositionY + mAxesSize, mPositionX + tOffset, tNumberYTop - 2,
						mGridColor);
			}
		}

		// draw labels
		if (tNumberYTop > TOUCH_LCD_HEIGHT - FONT_HEIGHT) {
			// no space for labels
			return -10;
		}
		// first offset is negative
		tOffset = 1 - ((FONT_WIDTH * mXMinStringWidth) / 2);
		if (aClearLabelsBefore) {
			// clear label space before
			sTheLcd.fillRect(mPositionX + tOffset, tNumberYTop, mPositionX + mWidthX - 1, tNumberYTop + FONT_HEIGHT - 1,
					mChartBackgroundColor);
		}

		// initialize both variables to avoid compiler warnings
		long tValue= mXLabelStartValue;
		float tValueFloat = mXLabelStartValueFloat;
		if (mXLabelIncrementValue != 0) {
			ltoa(tValue, tLabelStringBuffer, 10);
		} else {
			dtostrf(tValueFloat, mXMinStringWidth, mXNumVarsAfterDecimal, tLabelStringBuffer);
		}
		sTheLcd.drawText(mPositionX + tOffset, tNumberYTop, tLabelStringBuffer, 1, mLabelColor, mChartBackgroundColor);
		tOffset += mGridXResolution;

		for (; tOffset <= mWidthX; tOffset += mGridXResolution) {
			if (mXLabelIncrementValue != 0) {
				tValue += mXLabelIncrementValue;
				ltoa(tValue, tLabelStringBuffer, 10);
			} else {
				tValueFloat += mXLabelIncrementValueFloat;
				dtostrf(tValueFloat, mXMinStringWidth, mXNumVarsAfterDecimal, tLabelStringBuffer);
			}
			sTheLcd.drawText(mPositionX + tOffset, tNumberYTop, tLabelStringBuffer, 1, mLabelColor,
					mChartBackgroundColor);
		}
	}
	return 0;
// TODO draw axis title
}

/*
 * If aDoIncrement = true increment X values , else decrement
 * redraw Axis
 * return new X value
 */
int Chart::stepXLabelInt(const bool aDoIncrement) {
	if (aDoIncrement) {
		mXLabelStartValue += mXLabelIncrementValue;
	} else if (mXLabelStartValue > 0) {
		mXLabelStartValue -= mXLabelIncrementValue;
	}
	drawXAxis(true);
	return mXLabelStartValue;
}

/*
 * Increment X Values if aDoIncrement = true
 * redraw Axis
 * return false if decrement would set value below 0
 */
bool Chart::stepXLabelFloat(const bool aDoIncrement) {
	if (aDoIncrement) {
		mXLabelStartValueFloat += mXLabelIncrementValueFloat;
	} else {
		mXLabelStartValueFloat -= mXLabelIncrementValueFloat;
	}
	if (mXLabelStartValueFloat < 0) {
		mXLabelStartValueFloat = 0;
	}
	drawXAxis(true);
	return true;
}

/*
 * render Y AXIS only if integer or float increment value != 0
 */
uint8_t Chart::drawYAxis(bool aClearLabelsBefore) {

	char tLabelStringBuffer[32];

//draw y line
	sTheLcd.fillRect(mPositionX - mAxesSize + 1, mPositionY - mHeightY + 1, mPositionX, mPositionY - 1, mAxesColor);

	if (mYLabelIncrementValue != 0 || mYLabelIncrementValueFloat != 0.0) {
		uint16_t tOffset;
		uint16_t tNumberXLeft = mPositionX - mAxesSize - 1;
		if (!mHasGrid) {
			tNumberXLeft -= mAxesSize;
			// draw indicators
			for (tOffset = 0; tOffset <= mHeightY; tOffset += mGridYResolution) {
				sTheLcd.fillRect(tNumberXLeft + 2, mPositionY - tOffset, mPositionX - mAxesSize, mPositionY - tOffset,
						mGridColor);
			}
		}
		tNumberXLeft -= (mYMinStringWidth * FONT_WIDTH);

		// draw labels (numbers)
		// unsigned arithmetic
		if (tNumberXLeft > TOUCH_LCD_WIDTH) {
			// no space for labels
			return -11;
		}

		// first offset is negative
		tOffset = FONT_HEIGHT / 2;
		if (aClearLabelsBefore) {
			// clear label space before
			sTheLcd.fillRect(tNumberXLeft, mPositionY - mHeightY + 1, mPositionX - mAxesSize - 1,
					mPositionY - tOffset + FONT_HEIGHT, mChartBackgroundColor);
		}

		// convert to string
		// initialize both variables to avoid compiler warnings
		long tValue = mYLabelStartValue;
		float tValueFloat = mYLabelStartValueFloat;
		if (mYLabelIncrementValue != 0) {
			ltoa(tValue, tLabelStringBuffer, 10);
		} else {
			dtostrf(tValueFloat, mYMinStringWidth, mYNumVarsAfterDecimal, tLabelStringBuffer);
		}
		sTheLcd.drawText(tNumberXLeft, mPositionY - tOffset, tLabelStringBuffer, 1, mLabelColor, mChartBackgroundColor);
		tOffset += mGridYResolution;

		for (; tOffset <= mHeightY; tOffset += mGridYResolution) {
			if (mYLabelIncrementValue != 0) {
				tValue += mYLabelIncrementValue;
				ltoa(tValue, tLabelStringBuffer, 10);
			} else {
				tValueFloat += mYLabelIncrementValueFloat;
				dtostrf(tValueFloat, mYMinStringWidth, mYNumVarsAfterDecimal, tLabelStringBuffer);
			}
			sTheLcd.drawText(tNumberXLeft, mPositionY - tOffset, tLabelStringBuffer, 1, mLabelColor,
					mChartBackgroundColor);
		}
	}
	return 0;
}

int Chart::stepYLabelInt(const bool aDoIncrement) {
	if (aDoIncrement) {
		mYLabelStartValue += mYLabelIncrementValue;
	} else if (mYLabelStartValue > 0) {
		mYLabelStartValue -= mYLabelIncrementValue;
	}
	drawYAxis(true);
	return mYLabelStartValue;
}

float Chart::stepYLabelFloat(const bool aDoIncrement) {
	if (aDoIncrement) {
		mYLabelStartValueFloat += mYLabelIncrementValueFloat;
	} else {
		mYLabelStartValueFloat -= mYLabelIncrementValueFloat;
	}
	if (mYLabelStartValueFloat < 0) {
		mYLabelStartValueFloat = 0;
	}
	drawYAxis(false);
	return mYLabelStartValueFloat;
}

/*
 * Clear chart area (axes are not included)
 */
void Chart::clear(void) {
	sTheLcd.fillRect(mPositionX + 1, mPositionY - 1, mPositionX + mWidthX - 1, mPositionY - mHeightY + 1,
			mChartBackgroundColor);

}

/*
 * Draws the line and returns false if clipping occurs
 */
bool Chart::drawChartData(uint8_t * aDataPointer, uint16_t aDataLength, const uint16_t aDataColor,
		const uint8_t aMode) {

	bool tRetValue = true;
	uint8_t tValue;

	if (aDataLength > mWidthX) {
		aDataLength = mWidthX;
		tRetValue = false;
	}

// used only in line mode
	uint8_t tLastValue = *aDataPointer;
	if (tLastValue > mHeightY - 1) {
		tLastValue = mHeightY - 1;
		tRetValue = false;
	}

	uint16_t tXpos = mPositionX;

	for (; aDataLength > 0; aDataLength--) {
		tValue = *aDataPointer++;
		if (tValue > mHeightY - 1) {
			tValue = mHeightY - 1;
			tRetValue = false;
		}
		if (aMode == CHART_MODE_PIXEL) {
			tXpos++;
			sTheLcd.drawPixel(tXpos, mPositionY - tValue, aDataColor);
		} else if (aMode == CHART_MODE_LINE) {
			sTheLcd.drawLine(tXpos, mPositionY - tLastValue, tXpos + 1, mPositionY - tValue, aDataColor);
			tXpos++;
			tLastValue = tValue;
		} else if (aMode == CHART_MODE_AREA) {
			tXpos++;
			sTheLcd.drawLine(tXpos, mPositionY, tXpos, mPositionY - tValue, aDataColor);
		}
	}
	return tRetValue;
}

uint16_t Chart::getHeightY() const {
	return mHeightY;
}

uint16_t Chart::getPositionX() const {
	return mPositionX;
}

uint16_t Chart::getPositionY() const {
	return mPositionY;
}

uint16_t Chart::getWidthX() const {
	return mWidthX;
}

void Chart::setHeightY(uint16_t heightY) {
	mHeightY = heightY;
}

void Chart::setPositionX(uint16_t positionX) {
	mPositionX = positionX;
}

void Chart::setPositionY(uint16_t positionY) {
	mPositionY = positionY;
}

void Chart::setWidthX(uint16_t widthX) {
	mWidthX = widthX;
}

void Chart::setXLabelStartValue(int xLabelStartValue) {
	mXLabelStartValue = xLabelStartValue;
}

void Chart::setXLabelStartValueFloat(float xLabelStartValueFloat) {
	mXLabelStartValueFloat = xLabelStartValueFloat;
}

void Chart::setYLabelStartValue(int yLabelStartValue) {
	mYLabelStartValue = yLabelStartValue;
}

void Chart::setYLabelStartValueFloat(float yLabelStartValueFloat) {
	mYLabelStartValueFloat = yLabelStartValueFloat;
}

void Chart::setXLabelIncrementValue(int xLabelIncrementValue) {
	mXLabelIncrementValue = xLabelIncrementValue;
}

void Chart::setXLabelIncrementValueFloat(float xLabelIncrementValueFloat) {
	mXLabelIncrementValueFloat = xLabelIncrementValueFloat;
}

void Chart::setYLabelIncrementValue(int yLabelIncrementValue) {
	mYLabelIncrementValue = yLabelIncrementValue;
}

void Chart::setYLabelIncrementValueFloat(float yLabelIncrementValueFloat) {
	mYLabelIncrementValueFloat = yLabelIncrementValueFloat;
}

