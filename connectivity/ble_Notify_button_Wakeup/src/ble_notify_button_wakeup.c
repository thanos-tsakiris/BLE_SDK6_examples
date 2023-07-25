/**
 ****************************************************************************************
 *
 * @file ble_notify_button_wakeup.c
 *
 * @brief Multi-button wake-up with BLE notification source file.
 *
 * Copyright (c) 2012-2021 Renesas Electronics Corporation and/or its affiliates
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
#include "rwip_config.h"             // SW configuration


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "custs1.h"
#include "custs1_task.h"
#include "gpio.h"
#include "user_custs1_def.h"
#include "user_periph_setup.h"
#include "ble_notify_button_wakeup.h"
#include "wkupct_quadec.h"

/*
 * DEFINITIONS
 ****************************************************************************************
 */

#define WAKEUP_PINS                 (WKUPCT_PIN_SELECT(GPIO_SW2_PORT, GPIO_SW2_PIN) | WKUPCT_PIN_SELECT(GPIO_SW3_PORT, GPIO_SW3_PIN))
#define WAKEUP_POLARITIES           (WKUPCT_PIN_POLARITY(GPIO_SW2_PORT, GPIO_SW2_PIN, WKUPCT_PIN_POLARITY_LOW) | WKUPCT_PIN_POLARITY(GPIO_SW3_PORT, GPIO_SW3_PIN, WKUPCT_PIN_POLARITY_LOW))
#define WAKEUP_EVENTS_TO_TRIGGER    1
#define WAKEUP_DEBOUNCE_MS          30
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
*/ 

// Wake-up callback function declarations
void user_move_wkup_cb(void);
void user_stop_wkup_cb(void);
 
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/

/**
 ****************************************************************************************
 * @brief Callback function of the wake-up controller. This function will check which
 *        button was pressed and it will assign the appropriate callback to be called 
 *        when the BLE in operational
 * @return void
 ****************************************************************************************
 */
void app_wakeup_press_cb(void)
{
#if !defined (__DA14531__)
    if (GetBits16(SYS_STAT_REG, PER_IS_DOWN))
#endif
    {
        periph_init();
    }

    arch_ble_force_wakeup();	

    // If wakeup is generated by button SW3
    if(!GPIO_GetPinStatus(GPIO_SW3_PORT, GPIO_SW3_PIN))						
    {
        app_easy_wakeup_set(user_move_wkup_cb);
    }
    // If wakeup is generated by button SW2
    else if (!GPIO_GetPinStatus(GPIO_SW2_PORT, GPIO_SW2_PIN))
    {
        app_easy_wakeup_set(user_stop_wkup_cb);
    }

    wkupct_enable_irq(WAKEUP_PINS, WAKEUP_POLARITIES, WAKEUP_EVENTS_TO_TRIGGER,
                      WAKEUP_DEBOUNCE_MS);
    
    // Invoke the chosen callback when the BLE stack is operational
    app_easy_wakeup();

}

/**
 ****************************************************************************************
 * @brief Callback function to send the message "Move!". This callback will be called when
 *        the SW3 button is pressed
 * @return void
 ****************************************************************************************
 */
void user_move_wkup_cb(void)
{
    const char up_cmd[] = {'M', 'o', 'v', 'e', '!'};
    const size_t up_cmd_len = sizeof(up_cmd)/sizeof(up_cmd[0]);
	
    // Prepare the message to be sent
    struct custs1_val_ntf_ind_req *req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
                                                          prf_get_task_from_id(TASK_ID_CUSTS1),
                                                          TASK_APP,
                                                          custs1_val_ntf_ind_req,
                                                          DEF_SVC1_CTRL_POINT_CHAR_LEN);
		
    req->handle = SVC1_IDX_CONTROL_POINT_VAL;
    req->length = up_cmd_len;
    req->notification = true;
    memcpy(req->value, &up_cmd, up_cmd_len);
	
    // Send the message
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief Callback function to send the message "Stop!". This callback will be called when
 *        the SW2 button is pressed
 * @return void
 ****************************************************************************************
 */
void user_stop_wkup_cb(void)
{
    const char stop_cmd[] = {'S', 't', 'o', 'p', '!'};
    const size_t stop_cmd_len = sizeof(stop_cmd)/sizeof(stop_cmd[0]);
	
    // Prepare the message to be sent
    struct custs1_val_ntf_ind_req *req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
                                                          prf_get_task_from_id(TASK_ID_CUSTS1),
                                                          TASK_APP,
                                                          custs1_val_ntf_ind_req,
                                                          DEF_SVC1_CTRL_POINT_CHAR_LEN);
		
    req->handle = SVC1_IDX_CONTROL_POINT_VAL;
    req->length = stop_cmd_len;
    req->notification = true;
    memcpy(req->value, &stop_cmd, stop_cmd_len);

    // Send the message
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief User defined function which processes Write Notifications.
 * @param[in] msgid   Type of the message
 * @param[in] param   Pointer to the message to be processed
 * @param[in] dest_id Destination task id
 * @param[in] src_id  Source task id
 * @return void
 ****************************************************************************************
 */
void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
	switch(msgid)
	{
        // Check if the request is a Write Indication
        case CUSTS1_VAL_WRITE_IND:
        {
            const char hidden_msg_req[] = {'M', 'a', 'r', 'c', 'o', '?'};
            const char hidden_msg_resp[] = {'P', 'o', 'l', 'o', '!'};
            
            struct custs1_val_write_ind const *msg_param = (struct custs1_val_write_ind const *)(param);

            switch (msg_param->handle)
            {
                // Check if the message was destined to our custom service and if it matches the expected request 
                case SVC1_IDX_CONTROL_POINT_VAL:
                    if(!memcmp(&hidden_msg_req[0], &msg_param->value[0], msg_param->length)) {
                        size_t hidden_msg_resp_len = sizeof(hidden_msg_resp)/sizeof(hidden_msg_resp[0]);

                        struct custs1_val_ntf_ind_req *req = KE_MSG_ALLOC_DYN(CUSTS1_VAL_NTF_REQ,
                                                                              prf_get_task_from_id(TASK_ID_CUSTS1),
                                                                              TASK_APP,
                                                                              custs1_val_ntf_ind_req,
                                                                              DEF_SVC1_CTRL_POINT_CHAR_LEN);
                        // Send response 
                        req->handle = SVC1_IDX_CONTROL_POINT_VAL;
                        req->length = hidden_msg_resp_len;
                        req->notification = true;
                        memcpy(req->value, &hidden_msg_resp[0], hidden_msg_resp_len);
                        ke_msg_send(req);
                    }
                    break;

                default:
                    break;
                }
			}
			break;
			
			default:
                break;
	}
}

/**
 **************************************************************************************************
 * @brief User defined function which which will be called when the device establishes a connection
 * @param[in] connection_idx Connection ID number
 * @param[in] param          Pointer to GAPC_CONNECTION_REQ_IND message
 * @return void
 **************************************************************************************************
 */
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    // Enable the wakeup controller on connection
    wkupct_enable_irq(WAKEUP_PINS, WAKEUP_POLARITIES, WAKEUP_EVENTS_TO_TRIGGER,
                      WAKEUP_DEBOUNCE_MS);

    // Sets this function as wake-up interrupt callback	
    wkupct_register_callback(app_wakeup_press_cb);	

    default_app_on_connection(connection_idx, param);
}

/**
 *********************************************************************************************
 * @brief User defined function which which will be called when the device closes a connection
 * @param[in] param Pointer to GAPC_DISCONNECT_IND message
 * @return void
 *********************************************************************************************
 */
void user_on_disconnect( struct gapc_disconnect_ind const *param )
{
    default_app_on_disconnect(param);
}


/// @} APP
