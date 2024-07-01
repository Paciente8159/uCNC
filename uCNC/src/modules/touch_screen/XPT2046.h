/*
 * Copyright (c) 2015-2016  Spiros Papadimitriou
 *
 * This file is part of github.com/spapadim/XPT2046 and is released
 * under the MIT License: https://opensource.org/licenses/MIT
 *
 * This software is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.
 */

#ifndef _XPT2046_h_
#define _XPT2046_h_

// On my display driver ICs i,j mapped to (width-y),x
//  Flipping can be handled by order of calibration points, but not swapping

class XPT2046
{
public:
	enum rotation_t : uint8_t
	{
		ROT0,
		ROT90,
		ROT180,
		ROT270
	};
	enum adc_ref_t : uint8_t
	{
		MODE_SER,
		MODE_DFR
	};

	XPT2046(uint8_t cs_pin, uint8_t irq_pin);

	void begin(uint16_t width, uint16_t height); // width and height with no rotation!
	void setRotation(rotation_t rot) { _rot = rot; }
	void setSPISettings(uint32_t speed, uint8_t mode)
	{
		_spi_speed = speed;
		_spi_mode = mode;
	}

	// Calibration needs to be done with no rotation, on both display and touch drivers
	void getCalibrationPoints(uint16_t &x1, uint16_t &y1, uint16_t &x2, uint16_t &y2);
	void setCalibration(uint16_t vi1, uint16_t vj1, uint16_t vi2, uint16_t vj2);

	bool isTouching() const { return (digitalRead(_irq_pin) == LOW); }

	void getRaw(uint16_t &vi, uint16_t &vj, adc_ref_t mode = MODE_DFR, uint8_t max_samples = 0xff) const;
	void getPosition(uint16_t *x, uint16_t *y, adc_ref_t mode = MODE_DFR, uint8_t max_samples = 0xff) const;

	void powerDown() const;

private:
	static const uint8_t CTRL_LO_DFR = 0b0011;
	static const uint8_t CTRL_LO_SER = 0b0100;
	static const uint8_t CTRL_HI_X = 0b1001 << 4;
	static const uint8_t CTRL_HI_Y = 0b1101 << 4;

	static const uint16_t ADC_MAX = 0x07ff; // 12 bits

	uint16_t _width, _height;
	rotation_t _rot;
	uint8_t _cs_pin, _irq_pin;
	uint32_t _spi_speed;
	uint8_t _spi_mode;

	int32_t _cal_dx, _cal_dy, _cal_dvi, _cal_dvj;
	uint16_t _cal_vi1, _cal_vj1;

	uint16_t _readLoop(uint8_t ctrl, uint8_t max_samples) const;
};

#ifdef __cplusplus
extern "C"
{
#endif

#define XPT2046_ROT0 0
#define XPT2046_ROT90 1
#define XPT2046_ROT180 2
#define XPT2046_ROT270 2

	void xpt2046_init(uint16_t width, uint16_t height, uint8_t rotation, uint32_t spi_speed, uint8_t mode);
	bool xpt2046_get_position(uint16_t *x, uint16_t *y);

#ifdef __cplusplus
}
#endif

#endif // _XPT2046_h_
