#ifndef __KDEV_FLASH_GD_H__
#define __KDEV_FLASH_GD_H__

/*******************************************
 * for spi flash id definition
 ********************************************/
#define FLASH_WB_DEV			0xEF
#define FLASH_GD_DEV			0xC8
#define FLASH_MXIC_DEV			0xC2
#define FLASH_Micron_DEV		0x20
#define FLASH_ZBIT_DEV          0x5E

#define FLASH_IS_WB_DEV(x)		((x>>16)&0xFF == FLASH_WB_DEV)
#define FLASH_IS_MXIC_DEV(x)	((x>>16)&0xFF == FLASH_MXIC_DEV)
#define FLASH_IS_GD_DEV(x)	    ((x>>16)&0xFF == FLASH_GD_DEV)

#define WB_W25Q01JV_ID	    	0x2140 //1G
#define WB_W25Q512JV_ID	    	0x2040 //512M bit
#define WB_W25L256JV_ID	    	0x1940 //256M bit
#define WB_W25Q128BVFG_ID		0x1840 //128M bit
#define WB_W25Q64CV_ID_9F		0x1740
#define WB_W25Q32CV_ID_9F		0x1640
#define WB_W25P80_ID_9F			0x1420 //8M bit
#define WB_W25P16_ID_9F			0x1520 //16M bit
#define WB_W25P32_ID_9F			0x1620 //32M bit
#define WB_W25P16_ID_90			0x14
#define MX_MX25L51245G  		0x1A20
#define MX_MX25L25645G  		0x1920
#define MX_MX25L12845EM1		0x1820
#define MX_MX25L6405D			0x1720
#define GD_GD25Q64CSIG_ID	   	0x1740
#define GD_GD25Q127CSIG_ID	   	0x1840
#define GD_GD25Q256_ID	       	0x1940
#define GD_GD25Q256_ID_9F      	0x1940
#define GD25Q512MC_ID_9F        0x2040

#define INVALID_CHIP_ID			0xFFFF
#define INVALID_MANU_ID			0xFF

#define FLASH_SIZE_32MB_ID		0x19
#define FLASH_SIZE_16MB_ID		0x18
#define FLASH_SIZE_8MB_ID		0x17
#define FLASH_SIZE_4MB_ID		0x16
#define FLASH_SIZE_2MB_ID		0x15
#define FLASH_SIZE_1MB_ID		0x14
#define FLASH_SIZE_512MB_ID     0x20
#define FLASH_SIZE_SHIFT        (FLASH_SIZE_512MB_ID - 0x1A)

#define FLASH_3BYTE_ADDR_MAX    0x4000 //16Mbytes

//Status Register 2
#define SR2_OTP_E				BIT6
#define SR2_ECC_E				BIT4
#define SR2_BUF					BIT3
//Status Register 3
#define SR3_ECC_2				BIT6
#define SR3_ECC_1				BIT5
#define SR3_ECC_0				BIT4
#define SR5_ECCSE_1				BIT5
#define SR5_ECCSE_0				BIT4
#define SR3_P_FAIL				BIT3
#define SR3_E_FAIL				BIT2
#define SR3_ECC_CHECK			(SR3_ECC_1|SR3_ECC_0)
#define SR3_ECC_CHECK_F			(SR3_ECC_2|SR3_ECC_1|SR3_ECC_0)
/*******************************************
 * for function prototype definition
 ********************************************/
#endif/* __KDEV_FLASH_GD_H__ */

