/**
 ****************************************************************************************
 *
 * @file i2c_impl.h
 *
 * @brief I2C Implementation for LIS2DH.
 *
 * Copyright (c) 2023 Renesas Electronics Corporation. All rights reserved.
 * 
 * The MIT License (MIT)
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ***************************************************************************************
 */

#ifndef _I2C_IMPL_H_
#define _I2C_IMPL_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup
 *
 * @brief
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include "lis2dh_driver.h"
#include "i2c.h"

status_t i2c_accel_read_reg(uint32_t address, uint8_t *byte);

status_t i2c_accel_write_reg(uint32_t address, uint8_t byte);

status_t i2c_accel_read_fifo(uint8_t Reg_start, AxesRaw_t Data[32], uint16_t burst_num);

void i2c_accel_set_cb(i2c_complete_cb_t cb);

void i2c_accel_configure_pins_for_sleep(void);
 
/// @} APP

#endif //_I2C_IMPL_H_

