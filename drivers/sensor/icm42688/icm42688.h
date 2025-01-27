/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_ICM42688_H_
#define ZEPHYR_DRIVERS_SENSOR_ICM42688_H_

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <stdlib.h>

/**
 * @brief Accelerometer power modes
 */
enum icm42688_accel_mode {
	ICM42688_ACCEL_OFF,
	ICM42688_ACCEL_LP = 2,
	ICM42688_ACCEL_LN = 3,
};

/**
 * @brief Gyroscope power modes
 */
enum icm42688_gyro_mode {
	ICM42688_GYRO_OFF,
	ICM42688_GYRO_STANDBY,
	ICM42688_GYRO_LN = 3,
};

/**
 * @brief Accelerometer scale options
 */
enum icm42688_accel_fs {
	ICM42688_ACCEL_FS_16G,
	ICM42688_ACCEL_FS_8G,
	ICM42688_ACCEL_FS_4G,
	ICM42688_ACCEL_FS_2G,
};

/**
 * @brief Gyroscope scale options
 */
enum icm42688_gyro_fs {
	ICM42688_GYRO_FS_2000,
	ICM42688_GYRO_FS_1000,
	ICM42688_GYRO_FS_500,
	ICM42688_GYRO_FS_250,
	ICM42688_GYRO_FS_125,
	ICM42688_GYRO_FS_62_5,
	ICM42688_GYRO_FS_31_25,
	ICM42688_GYRO_FS_15_625,
};

/**
 * @brief Accelerometer data rate options
 */
enum icm42688_accel_odr {
	ICM42688_ACCEL_ODR_32000 = 1,
	ICM42688_ACCEL_ODR_16000,
	ICM42688_ACCEL_ODR_8000,
	ICM42688_ACCEL_ODR_4000,
	ICM42688_ACCEL_ODR_2000,
	ICM42688_ACCEL_ODR_1000,
	ICM42688_ACCEL_ODR_200,
	ICM42688_ACCEL_ODR_100,
	ICM42688_ACCEL_ODR_50,
	ICM42688_ACCEL_ODR_25,
	ICM42688_ACCEL_ODR_12_5,
	ICM42688_ACCEL_ODR_6_25,
	ICM42688_ACCEL_ODR_3_125,
	ICM42688_ACCEL_ODR_1_5625,
	ICM42688_ACCEL_ODR_500,
};

/**
 * @brief Gyroscope data rate options
 */
enum icm42688_gyro_odr {
	ICM42688_GYRO_ODR_32000 = 1,
	ICM42688_GYRO_ODR_16000,
	ICM42688_GYRO_ODR_8000,
	ICM42688_GYRO_ODR_4000,
	ICM42688_GYRO_ODR_2000,
	ICM42688_GYRO_ODR_1000,
	ICM42688_GYRO_ODR_200,
	ICM42688_GYRO_ODR_100,
	ICM42688_GYRO_ODR_50,
	ICM42688_GYRO_ODR_25,
	ICM42688_GYRO_ODR_12_5,
	ICM42688_GYRO_ODR_500 = 0xF
};

/**
 * @brief All sensor configuration options
 */
struct icm42688_cfg {
	enum icm42688_accel_mode accel_mode;
	enum icm42688_accel_fs accel_fs;
	enum icm42688_accel_odr accel_odr;
	/* TODO accel signal processing options */

	enum icm42688_gyro_mode gyro_mode;
	enum icm42688_gyro_fs gyro_fs;
	enum icm42688_gyro_odr gyro_odr;
	/* TODO gyro signal processing options */

	bool temp_dis;
	/* TODO temp signal processing options */

	/* TODO timestamp options */

	bool fifo_en;
	uint16_t fifo_wm;
	bool fifo_hires;
	/* TODO additional FIFO options */

	/* TODO interrupt options */
};

/**
 * @brief Device data (struct device)
 */
struct icm42688_dev_data {
	struct icm42688_cfg cfg;
};

/**
 * @brief Device config (struct device)
 */
struct icm42688_dev_cfg {
	struct spi_dt_spec spi;
	struct gpio_dt_spec gpio_int1;
	struct gpio_dt_spec gpio_int2;
};

/**
 * @brief Reset the sensor
 *
 * @param dev icm42688 device pointer
 *
 * @retval 0 success
 * @retval -EINVAL Reset status or whoami register returned unexpected value.
 */
int icm42688_reset(const struct device *dev);

/**
 * @brief (Re)Configure the sensor with the given configuration
 *
 * @param dev icm42688 device pointer
 * @param cfg icm42688_cfg pointer
 *
 * @retval 0 success
 * @retval -errno Error
 */
int icm42688_configure(const struct device *dev, struct icm42688_cfg *cfg);

/**
 * @brief Reads all channels
 *
 * Regardless of what is enabled/disabled this reads all data registers
 * as the time to read the 14 bytes at 1MHz is going to be 112 us which
 * is less time than a SPI transaction takes to setup typically.
 *
 * @param dev icm42688 device pointer
 * @param buf 14 byte buffer to store data values (7 channels, 2 bytes each)
 *
 * @retval 0 success
 * @retval -errno Error
 */
int icm42688_read_all(const struct device *dev, uint8_t data[14]);

/**
 * @brief Convert icm42688 accelerometer value to useful g values
 *
 * @param cfg icm42688_cfg current device configuration
 * @param in raw data value in int32_t format
 * @param out_g whole G's output in int32_t
 * @param out_ug micro (1/1000000) of a G output as uint32_t
 */
static inline void icm42688_accel_g(struct icm42688_cfg *cfg, int32_t in, int32_t *out_g,
				    uint32_t *out_ug)
{
	int32_t sensitivity = 0; /* value equivalent for 1g */

	switch (cfg->accel_fs) {
	case ICM42688_ACCEL_FS_2G:
		sensitivity = 16384;
		break;
	case ICM42688_ACCEL_FS_4G:
		sensitivity = 8192;
		break;
	case ICM42688_ACCEL_FS_8G:
		sensitivity = 4096;
		break;
	case ICM42688_ACCEL_FS_16G:
		sensitivity = 2048;
		break;
	}

	/* Whole g's */
	*out_g = in / sensitivity;

	/* Micro g's */
	*out_ug = ((abs(in) - (abs((*out_g)) * sensitivity)) * 1000000) / sensitivity;
}

/**
 * @brief Convert icm42688 gyroscope value to useful deg/s values
 *
 * @param cfg icm42688_cfg current device configuration
 * @param in raw data value in int32_t format
 * @param out_dps whole deg/s output in int32_t
 * @param out_udps micro (1/1000000) deg/s as uint32_t
 */
static inline void icm42688_gyro_dps(struct icm42688_cfg *cfg, int32_t in, int32_t *out_dps,
				     uint32_t *out_udps)
{
	int64_t sensitivity = 0; /* value equivalent for 10x gyro reading deg/s */

	switch (cfg->gyro_fs) {
	case ICM42688_GYRO_FS_2000:
		sensitivity = 164;
		break;
	case ICM42688_GYRO_FS_1000:
		sensitivity = 328;
		break;
	case ICM42688_GYRO_FS_500:
		sensitivity = 655;
		break;
	case ICM42688_GYRO_FS_250:
		sensitivity = 1310;
		break;
	case ICM42688_GYRO_FS_125:
		sensitivity = 2620;
		break;
	case ICM42688_GYRO_FS_62_5:
		sensitivity = 5243;
		break;
	case ICM42688_GYRO_FS_31_25:
		sensitivity = 10486;
		break;
	case ICM42688_GYRO_FS_15_625:
		sensitivity = 20972;
		break;
	}

	int32_t in10 = in * 10;

	/* Whole deg/s */
	*out_dps = in10 / sensitivity;

	/* Micro deg/s */
	*out_udps = ((int64_t)(llabs(in10) - (llabs((*out_dps)) * sensitivity)) * 1000000LL) /
		    sensitivity;
}

/**
 * @brief Convert icm42688 accelerometer value to useful m/s^2 values
 *
 * @param cfg icm42688_cfg current device configuration
 * @param in raw data value in int32_t format
 * @param out_ms meters/s^2 (whole) output in int32_t
 * @param out_ums micrometers/s^2 output as uint32_t
 */
static inline void icm42688_accel_ms(struct icm42688_cfg *cfg, int32_t in, int32_t *out_ms,
				     uint32_t *out_ums)
{
	int64_t sensitivity = 0; /* value equivalent for 1g */

	switch (cfg->accel_fs) {
	case ICM42688_ACCEL_FS_2G:
		sensitivity = 16384;
		break;
	case ICM42688_ACCEL_FS_4G:
		sensitivity = 8192;
		break;
	case ICM42688_ACCEL_FS_8G:
		sensitivity = 4096;
		break;
	case ICM42688_ACCEL_FS_16G:
		sensitivity = 2048;
		break;
	}

	/* Convert to micrometers/s^2 */
	int64_t in_ms = in * SENSOR_G;

	/* meters/s^2 whole values */
	*out_ms = in_ms / (sensitivity * 1000000LL);

	/* micrometers/s^2 */
	*out_ums = ((llabs(in_ms) - (llabs(*out_ms) * sensitivity * 1000000LL)) / sensitivity);
}

/**
 * @brief Convert icm42688 gyroscope value to useful rad/s values
 *
 * @param cfg icm42688_cfg current device configuration
 * @param in raw data value in int32_t format
 * @param out_rads whole rad/s output in int32_t
 * @param out_urads microrad/s as uint32_t
 */
static inline void icm42688_gyro_rads(struct icm42688_cfg *cfg, int32_t in, int32_t *out_rads,
				      uint32_t *out_urads)
{
	int64_t sensitivity = 0; /* value equivalent for 10x gyro reading deg/s */

	switch (cfg->gyro_fs) {
	case ICM42688_GYRO_FS_2000:
		sensitivity = 164;
		break;
	case ICM42688_GYRO_FS_1000:
		sensitivity = 328;
		break;
	case ICM42688_GYRO_FS_500:
		sensitivity = 655;
		break;
	case ICM42688_GYRO_FS_250:
		sensitivity = 1310;
		break;
	case ICM42688_GYRO_FS_125:
		sensitivity = 2620;
		break;
	case ICM42688_GYRO_FS_62_5:
		sensitivity = 5243;
		break;
	case ICM42688_GYRO_FS_31_25:
		sensitivity = 10486;
		break;
	case ICM42688_GYRO_FS_15_625:
		sensitivity = 20972;
		break;
	}

	int64_t in10_rads = (int64_t)in * SENSOR_PI * 10LL;

	/* Whole rad/s */
	*out_rads = in10_rads / (sensitivity * 180LL * 1000000LL);

	/* microrad/s */
	*out_urads = ((llabs(in10_rads) - (llabs((*out_rads)) * sensitivity * 180LL * 1000000LL))) /
		     (sensitivity * 180LL);
}

/**
 * @brief Convert icm42688 temp value to useful celsius values
 *
 * @param cfg icm42688_cfg current device configuration
 * @param in raw data value in int32_t format
 * @param out_c whole celsius output in int32_t
 * @param out_uc micro (1/1000000) celsius as uint32_t
 */
static inline void icm42688_temp_c(int32_t in, int32_t *out_c, uint32_t *out_uc)
{
	int64_t sensitivity = 13248; /* value equivalent for x100 1c */

	int64_t in100 = in * 100;

	/* Whole celsius */
	*out_c = in100 / sensitivity;

	/* Micro celsius */
	*out_uc = ((llabs(in100) - (llabs(*out_c)) * sensitivity) * 1000000LL) / sensitivity;

	/* Shift whole celsius 25 degress */
	*out_c = *out_c + 25;
}
#endif /* ZEPHYR_DRIVERS_SENSOR_ICM42688_H_ */
