/**
 ****************************************************************************************
 *
 * @file user_wakeup.c
 *
 * @brief uder wakeup source code.
 *
 * Copyright (c) 2015-2021 Renesas Electronics Corporation and/or its affiliates
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "wkupct_quadec.h" 
#include "user_wakeup.h"
#include "user_periph_setup.h"
#include "uart_utils.h"
#include "user_barebone.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
bool sw2_source            __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
bool sw3_source            __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

/**
 ****************************************************************************************
 * @brief Button press callback function. Registered in WKUPCT driver.
 * @return void
 ****************************************************************************************
 */
void user_app_wakeup_press_cb(void)
{
#if !defined (__DA14531__)
    if (GetBits16(SYS_STAT_REG, PER_IS_DOWN))
        periph_init();
#endif
    
	if (arch_ble_ext_wakeup_get())              // Check if the device was in permanent sleep
	{
		arch_set_sleep_mode(ARCH_SLEEP_OFF);    // Disable sleep mode
		arch_ble_force_wakeup();                // Force the BLE to wake up
		arch_ble_ext_wakeup_off();              // Disable the permanent sleep flag
        // Read the status of the GPIO as soon as i wake up
        sw2_source = GPIO_GetPinStatus(GPIO_SW2_PORT, GPIO_SW2_PIN);
        sw3_source = GPIO_GetPinStatus(GPIO_SW3_PORT, GPIO_SW3_PIN);
        app_easy_wakeup();                      // Invoke corresponding actions as soon as the BLE is awake 
    }
	else
    {
#ifdef CFG_PRINTF_UART2
				printf_string(UART2,"\n\n\rSystem going to sleep");
#endif
		GPIO_SetInactive(LED_PORT, LED_PIN);
		arch_set_sleep_mode(ARCH_EXT_SLEEP_ON);				
		arch_ble_ext_wakeup_on();
        // Re-enable the wake up interrupt for SW2 and SW3 sources
        user_wakeup_example_init();
	}											  	
}


static void app_wakeup_cb(void)
{
    // If wakeup is generated by SW2
    if(!sw2_source)
    {
        GPIO_SetActive(LED_PORT, LED_PIN);										
#ifdef CFG_PRINTF_UART2
        printf_string(UART2,"\n\n\rWakeup source: SW2");
#endif
    }
    else if(!sw3_source)
    {
#ifdef CFG_PRINTF_UART2
        printf_string(UART2,"\n\n\rWakeup source: SW3");
#endif
    }
    
    // Clear the interrupt source
    sw2_source = false;
    sw3_source = false;
    
    user_app_adv_start();
    
    // Re-enable the wake up interrupt for SW2 and SW3 sources
    user_wakeup_example_init();
}

/**
 ****************************************************************************************
 * @brief Sets sw2 and sw3 as wake up trigger
 * @return void
 ****************************************************************************************
*/
void user_wakeup_example_init(void)
{
    app_easy_wakeup_set(app_wakeup_cb);
    wkupct_enable_irq((WKUPCT_PIN_SELECT(GPIO_SW2_PORT, GPIO_SW2_PIN) | WKUPCT_PIN_SELECT(GPIO_SW3_PORT, GPIO_SW3_PIN)), 						// When enabling more than one interruptsource use OR bitoperation. WKUPCT_PIN_SELECT will make sure the appropriate bit in the register is set. 
                        (WKUPCT_PIN_POLARITY(GPIO_SW2_PORT, GPIO_SW2_PIN, WKUPCT_PIN_POLARITY_LOW) | WKUPCT_PIN_POLARITY(GPIO_SW3_PORT, GPIO_SW3_PIN, WKUPCT_PIN_POLARITY_LOW)),	// When enabling more than one interrupt source use OR bitoperation. WKUPCT_PIN_POLARITY will make sure the appropriate bit in the register is set.
                        EVENTS_BEFORE_INTERRUPT,																																																													
                        DEBOUNCE_TIME);																																																												

    wkupct_register_callback(user_app_wakeup_press_cb);	// sets this function as wake-up interrupt callback
}

/// @} APP
