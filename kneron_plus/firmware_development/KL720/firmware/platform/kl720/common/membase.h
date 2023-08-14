/* Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 */

/******************************************************************************
*  Filename:
*  ---------
*  membase.h
*
*  Description:
*  ------------
*
*
******************************************************************************/

#ifndef _MEMBASE_H_
#define _MEMBASE_H_

/******************************************************************************
Head Block of The File
******************************************************************************/
// Sec 0: Comment block of the file

// Sec 1: Include File

// Sec 2: Constant Definitions, Imported Symbols, miscellaneous

#define SPIF_XIP_BASE               0x10000000

#define SiRAM_MEM_BASE              0x1FFC0000
#define SiRAM_MEM_SIZE              0x20000         //128KB
#define SdRAM_MEM_BASE              0x1FFE0000
#define SdRAM_MEM_SIZE              0x20000         //128KB

#define NiRAM_MEM_BASE              0x6F000000      
#define NiRAM_MEM_SIZE              0x20000         //128KB 
#define NdRAM_MEM_BASE              0x6F080000      
#define NdRAM_MEM_SIZE              0x80000         //512KB
#define NCPU_FW_SIZE                0x200000        //2MB
#define NCPU_FW_IRAM_SIZE           NiRAM_MEM_SIZE
#define NCPU_FW_DDR_BASE            0x80020000  
#define NCPU_FW_DDR_SIZE            (NCPU_FW_SIZE-NCPU_FW_IRAM_SIZE)      //2MB - 128KB

// IPC memory: 8KB
#define DDR_MEM_IPC_ADDR            0x8021E000

//DDR memory address space means the addressing capability.
//For KL720, it's 1.75G bytes.
//The physical DDR size is determined by the DDR chip(s) in the silicon package.
//For example, Winbond 128MBytes, Micron 1GBytes
//DDR size will be different from projects.
#define DDR_MEM_PHY_BASE            0x80000000      // DDR physical address base
#define DDR_MEM_PHY_SIZE            0x08000000      // chip default 128MB
#define DDR_MEM_BASE                0x80220000      // for all models and heap
#define DDR_MEM_ADDR_SPACE          0x70000000      // DDRC capability from 0x80000000 until 0xE0000000 for CM4

#define DDR_MAGIC_BASE              0x80210000      // for usb_boot

/******************************************************************************
Declaration of External Variables & Functions
******************************************************************************/
// Sec 3: declaration of external variable

// Sec 4: declaration of external function prototype

/******************************************************************************
Declaration of data structure
******************************************************************************/
// Sec 5: structure, uniou, enum, linked list

/******************************************************************************
Declaration of Global Variables & Functions
******************************************************************************/
// Sec 6: declaration of global variable

// Sec 7: declaration of global function prototype

/******************************************************************************
Declaration of static Global Variables & Functions
******************************************************************************/
// Sec 8: declaration of static global variable

// Sec 9: declaration of static function prototype

/******************************************************************************
// Sec 10: C Functions
******************************************************************************/




#endif //_MEMBASE_H_

