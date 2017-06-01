/** @file sys_main.c 
*   @brief Application main file
*   @date 08-Feb-2017
*   @version 04.06.01
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2016 Texas Instruments Incorporated - www.ti.com 
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include "usdcard.h"
#include "usdcard_tests.h"
#include "error.h"
#include <stdio.h>
#include "ti_fee.h"
#include "F021.h"
#define _L2FMC
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */


/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */

	Fapi_StatusType oReturnCheck = Fapi_Status_Success;

	// Initialize the peripherals
	gioInit();
	spiInit();
	hetInit();

	uint8 buffer = 0x55;
	uint32 address = 0x0000FFFE;

	printf("All devices started...\n");

	Fapi_initializeFlashBanks(16);
	oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);
	oReturnCheck = Fapi_enableMainBankSectors((uint16)0xFFFF);
	while(Fapi_checkFsmForReady() != Fapi_Status_FsmReady);
	oReturnCheck = Fapi_issueProgrammingCommand(&address, &buffer, (uint8)1, 0, 0, Fapi_AutoEccGeneration);
	while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy);
	printf("Return check %d\n", oReturnCheck);
	printf("FSM Status %d\n", Fapi_getFsmStatus());
	while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy);

	printf("Return of programming command %d\n", oReturnCheck);

	printf("After wait while\n");

	// Wait here if the tests are successful
	while(1);
/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
/* USER CODE END */
