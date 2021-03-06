//////////////////////////////////////////////////////////////////////////////////
// main.c for Cosmos+ OpenSSD
// Copyright (c) 2016 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Youngjin Jo <yjjo@enc.hanyang.ac.kr>
//				  Sangjin Lee <sjlee@enc.hanyang.ac.kr>
//				  Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//				  Kibin Park <kbpark@enc.hanyang.ac.kr>
//
// This file is part of Cosmos+ OpenSSD.
//
// Cosmos+ OpenSSD is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// Cosmos+ OpenSSD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Cosmos+ OpenSSD; see the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Company: ENC Lab. <http://enc.hanyang.ac.kr>
// Engineer: Sangjin Lee <sjlee@enc.hanyang.ac.kr>
//			 Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//			 Kibin Park <kbpark@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: Main
// File Name: main.c
//
// Version: v1.0.2
//
// Description:
//   - initializes caches, MMU, exception handler
//   - calls nvme_main function
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.2
//   - An address region (0x0020_0000 ~ 0x179F_FFFF) is used to uncached & nonbuffered region
//   - An address region (0x1800_0000 ~ 0x3FFF_FFFF) is used to cached & buffered region
//
// * v1.0.1
//   - Paging table setting is modified for QSPI or SD card boot mode
//     * An address region (0x0010_0000 ~ 0x001F_FFFF) is used to place code, data, heap and stack sections
//     * An address region (0x0010_0000 ~ 0x001F_FFFF) is setted a cached&bufferd region
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_exception.h"
#include "xil_io.h"
#include "sleep.h"


#include "xil_cache.h"
//#include "xil_exception.h"
//#include "xil_mmu.h"
#include "xparameters.h"
//#include "xscugic_hw.h"
//#include "xscugic.h"
//#include "xil_printf.h"
#include "nvme/debug.h"

#include "nvme/nvme.h"
#include "nvme/nvme_main.h"
#include "nvme/host_lld.h"

#include "memory_map.h"
#include "generate.h"
#include "nvme/nvme_io_cmd.h"
//XScuGic GicInstance;

#define Generate     1
#define Decode       2
#define Schedule     3



int main()
{
	init_platform();
	print("Hello World\n\r");
	//unsigned int u;

	//XScuGic_Config *IntcConfig;

	Xil_ICacheDisable();
	Xil_DCacheDisable();
	//Xil_DisableMMU();

	// Paging table set
	#define MB (1024*1024)
	//for (u = 0; u < 4096; u++)
	//{
	//	if (u < 0x2)
	//		Xil_SetTlbAttributes(u * MB, 0xC1E); // cached & buffered
	//	else if (u < 0x180)
	//		Xil_SetTlbAttributes(u * MB, 0xC12); // uncached & nonbuffered
	//	else if (u < 0x400)
	//		Xil_SetTlbAttributes(u * MB, 0xC1E); // cached & buffered
	//	else
	//		Xil_SetTlbAttributes(u * MB, 0xC12); // uncached & nonbuffered
	//}

	//Xil_EnableMMU();
	/*    //lhk 0827
	Xil_ICacheEnable();
	Xil_DCacheEnable();
	xil_printf("[!] DCache and ICache has been enabled.\r\n");
    */

	xil_printf("\r\n Hello KCUSSD !!! \r\n");


	//Xil_ExceptionInit();

	//IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	//XScuGic_CfgInitialize(&GicInstance, IntcConfig, IntcConfig->CpuBaseAddress);
	//Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
	//							(Xil_ExceptionHandler)XScuGic_InterruptHandler,
	//							&GicInstance);

	//XScuGic_Connect(&GicInstance, 61,
	//				(Xil_ExceptionHandler)dev_irq_handler,
	//				(void *)0);

	//XScuGic_Enable(&GicInstance, 61);

	// Enable interrupts in the Processor.
	//Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	//Xil_ExceptionEnable();

	//dev_irq_init();

	//nvme_main();
    unsigned int exeLlr;
    unsigned int status;
    unsigned int count;
    count=1;
    status = Generate;
	xil_printf("!!! Wait until FTL reset complete !!! \r\n");

	InitFTL();

	xil_printf("\r\nFTL reset complete!!! \r\n");
	//xil_printf("Turn on the host PC \r\n");
    //eraseblock_60h_d0h(NSC_0_BASEADDR,1,0);
    //eraseblock_60h_d0h(NSC_1_BASEADDR,1,0);

    eraseblock_60h_d0h(NSC_0_BASEADDR,1,0x7f800);
    eraseblock_60h_d0h(NSC_1_BASEADDR,1,0x7f800);
    xil_printf("erase two channels complete! \r\n");
   
    while(1)
    {
		exeLlr = 1;

        
        if(status == Generate)
        {
            generateReQ(count);
            exeLlr=0;
            status = Decode;

        }
		else if(status == Decode)
		{
			NVME_COMMAND* nvmeCmd_ptr;
			nvmeCmd_ptr = (NVME_COMMAND *)(NVME_REQ_SIM_ADDR + (count-1)*sizeof(NVME_COMMAND));
			//unsigned int cmdValid;

			//cmdValid = get_nvme_cmd(&nvmeCmd.qID, &nvmeCmd.cmdSlotTag, &nvmeCmd.cmdSeqNum, nvmeCmd.cmdDword);

			//if(cmdValid == 1)
			//{
				//if(nvmeCmd.qID == 0)
				//{
					//handle_nvme_admin_cmd(&nvmeCmd);
				//}
				//else
				//{
					handle_nvme_io_cmd(nvmeCmd_ptr);
					ReqTransSliceToLowLevel();
					exeLlr=0;
					status = Schedule;
					count++;
				//}
			//}
		}
		else if(status == Schedule)//exeLlr && (notCompletedNandReqCnt || blockedReqCnt))//(nvmeDmaReqQ.headReq != REQ_SLOT_TAG_NONE) ||
		{
			CheckDoneNvmeDmaReq();
			SchedulingNandReq();
			status = Generate;
			while((nvmeDmaReqQ.headReq != REQ_SLOT_TAG_NONE) || notCompletedNandReqCnt || blockedReqCnt)
			{
				CheckDoneNvmeDmaReq();
				SchedulingNandReq();
			}
			if(count==5)
				break;
		}
    }
	xil_printf("done\r\n");
	cleanup_platform();
	return 0;
}
