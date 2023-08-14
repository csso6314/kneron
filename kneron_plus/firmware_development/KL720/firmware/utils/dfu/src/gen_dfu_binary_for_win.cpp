// gen_dfu_binary_for_win.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_DEPRECATE
//#include "stdafx.h"
#include <iostream>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>



typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;

enum CPU_TYPE {
    CPU_SCPU = 1,
    CPU_NCPU,
};

#define SCPU_IMAGE_SIZE             0x20000    // 128KB
#define NCPU_IMAGE_SIZE             0x200000   // 2MB


#define FW_INFO_SIZE       0x20000               //NOR: 4KB, NAND:128KB
#define MAX_ALL_MODEL_SIZE (100 * 1024 * 1024)   // 100MB

/* git_info, ksrs file parser for version control */
const char* GIT_ID_FILE = "./git_id.txt";
const char* GIT_CMD = "git log -1 --abbrev=8 --pretty=format:0x%h > git_id.txt: 2>nul";
/* NCPU's KSRS ID creates by DFU program */
/* SCPU's KSRS_ID created by SCPU pre_build.bat */
const char* KSRS_ID_FILE = "ksrs_id.txt";
const char* KSRS_CMD = "KSR_server_handler.exe -r 1 --skip_id_print 1: 2>nul";


/* Calculate the 32-bit checksum of object using 32-bit size block
** If address is not 32-bit aligned, use the fraction of the block w/ zero's
** replacing excluded bytes.  (Note: we use Little Endian)
*/
u32 kdp_gen_sum32(u8 *data, u32 size)
{
    u32 sum, *ddr, i = (u32)data & 0x03;
    ddr = (u32*)((u32)data & 0xFFFFFFFC);  // point to the first 32-bit aligned block
    if (i) { // starting address misaligned ?
        size = size + i - 4;
        sum = *ddr;
        sum >>= i * 8;
        sum <<= i * 8;
        ddr++;
    } else
        sum = 0;
    for (i = 0; i < (size & 0xFFFFFFFC); i += 4) {
        sum += *ddr;
        ddr++;
    }
    i = size & 3;
    if (i) { // ending address misaligned ?
        size = *ddr;
        size <<= (4 - i) * 8;
        size >>= (4 - i) * 8;
        sum += size;
    }
    return(sum);
}

/*
 * @brief get git version number
 */
u32 get_git_id(void)
{
    char buf[64];
    uint32_t val = 0;
    char *filename;
    FILE *fin;
    filename = (char*)GIT_ID_FILE;
    system(GIT_CMD);
    if ((fin = fopen(filename, "r")) == NULL) {
        /* system does not install git */
        return 0;
    }
    if (fgets(buf, 256, fin) != NULL) {
        val = strtoul(buf, NULL, 16);
    }
    fclose(fin);
    remove(filename);
    return val;
}
/**
 * @brief get KSRS ID
 *      NCPU's KSRS ID creates by DFU program
 *      SCPU's KSRS_ID created by SCPU pre_build.bat
 */
uint32_t get_ksrs_id(CPU_TYPE cpu)
{
    char buf[64];
    char *filename;
    uint32_t val = 0;
    FILE *fin;
    filename = (char*)KSRS_ID_FILE;
    if (cpu == CPU_NCPU) {
        system(KSRS_CMD);     
    }
    if ((fin = fopen(filename, "r")) == NULL) {
        return 0;
    }
    if (fgets(buf, 256, fin) != NULL) {
        val = strtol(buf, NULL, 10);
    }
    fclose(fin);
    return val;
}
int gen_dfu_scpu(char * in_file, char * out_file)
{
    FILE *pfIn, *pfOut;
    u32 sum32, git_id, ksrs_id;

    pfIn = fopen(in_file, "rb");
    if (pfIn == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, in_file);
        return -1;
    }

    pfOut = fopen(out_file, "wb");
    if (pfOut == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, out_file);
        return -1;
    }

    fseek(pfIn, 0, SEEK_END);
    int inFileLen = ftell(pfIn);
    if (inFileLen > SCPU_IMAGE_SIZE) {
        printf("Error: intput SCPU file is larger than %d bytes\n", SCPU_IMAGE_SIZE);
        return -1;
    }
    fseek(pfIn, 0, SEEK_SET);

    u8 * buf = (u8 *)malloc(SCPU_IMAGE_SIZE);
    if (buf == NULL) {
        printf("Error: failed to alloc memory\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    long result = fread(buf, 1, inFileLen, pfIn);
    if (result != inFileLen) {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        free(buf);
        return 0;
    }

    fclose(pfIn);

    memset(buf + inFileLen, 0, SCPU_IMAGE_SIZE - inFileLen);

    git_id = get_git_id();
    ksrs_id = get_ksrs_id(CPU_SCPU);
    *(u32 *)(buf + SCPU_IMAGE_SIZE - 20) = CPU_SCPU;
    *(u32 *)(buf + SCPU_IMAGE_SIZE - 16) = ksrs_id;
    *(u32 *)(buf + SCPU_IMAGE_SIZE - 12) = git_id;
    *(u32 *)(buf + SCPU_IMAGE_SIZE - 8) = inFileLen;
    sum32 = kdp_gen_sum32(buf, SCPU_IMAGE_SIZE - 4);
    *(u32 *)(buf + SCPU_IMAGE_SIZE - 4) = sum32;

    fwrite(buf, SCPU_IMAGE_SIZE, 1, pfOut);
    fclose(pfOut);
    free(buf);
    return 0;
}

int gen_dfu_ncpu(char * in_file, char * out_file)
{
    FILE *pfIn, *pfOut;
    u32 sum32, git_id, ksrs_id;

    pfIn = fopen(in_file, "rb");
    if (pfIn == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, in_file);
        return -1;
    }

    pfOut = fopen(out_file, "wb");
    if (pfOut == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, out_file);
        return -1;
    }

    fseek(pfIn, 0, SEEK_END);
    int inFileLen = ftell(pfIn);
    if (inFileLen > NCPU_IMAGE_SIZE) {
        printf("Error: intput NCPU file is larger than %d bytes\n", NCPU_IMAGE_SIZE);
        return -1;
    }
    fseek(pfIn, 0, SEEK_SET);

    u8 * buf = (u8 *)malloc(NCPU_IMAGE_SIZE);
    if (buf == NULL) {
        printf("Error: failed to alloc memory\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    long result = fread(buf, 1, inFileLen, pfIn);
    if (result != inFileLen) {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        free(buf);
        return 0;
    }

    fclose(pfIn);

    memset(buf + inFileLen, 0, NCPU_IMAGE_SIZE - inFileLen);

    git_id = get_git_id();
    ksrs_id = get_ksrs_id(CPU_NCPU);
    *(u32 *)(buf + NCPU_IMAGE_SIZE - 20) = CPU_NCPU;
    *(u32 *)(buf + NCPU_IMAGE_SIZE - 16) = ksrs_id;
    *(u32 *)(buf + NCPU_IMAGE_SIZE - 12) = git_id;
    *(u32 *)(buf + NCPU_IMAGE_SIZE - 8) = inFileLen;
    sum32 = kdp_gen_sum32(buf, NCPU_IMAGE_SIZE - 4);
    *(u32 *)(buf + NCPU_IMAGE_SIZE - 4) = sum32;

    fwrite(buf, NCPU_IMAGE_SIZE, 1, pfOut);
    fclose(pfOut);
    free(buf);
    return 0;
}

int gen_dfu_model(char * fw_info_file, char * all_model_file, char * out_file)
{
    FILE *pfFW, *pfModel, *pfOut;
    u32 sum32;
    u32 total_size;

    pfFW = fopen(fw_info_file, "rb");
    if (pfFW == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, fw_info_file);
        return -1;
    }

    pfModel = fopen(all_model_file, "rb");
    if (pfModel == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, all_model_file);
        return -1;
    }

    pfOut = fopen(out_file, "wb");
    if (pfOut == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, out_file);
        return -1;
    }

    fseek(pfFW, 0, SEEK_END);
    u32 FwFileLen = ftell(pfFW);
    if (FwFileLen > FW_INFO_SIZE) {
        printf("Error: intput fw_info file is larger than 232 bytes\n");
        return -1;
    }
    fseek(pfFW, 0, SEEK_SET);

    fseek(pfModel, 0, SEEK_END);
    u32 ModelFileLen = ftell(pfModel);
    if (ModelFileLen > MAX_ALL_MODEL_SIZE) {
        printf("Error: intput all_model_info file is larger than %d bytes\n", MAX_ALL_MODEL_SIZE);
        return -1;
    }
    fseek(pfModel, 0, SEEK_SET);

    total_size = FW_INFO_SIZE + ModelFileLen + 4;   // 4 bytes for sum32 word

    if ((total_size % 4) != 0) {
        total_size = ((total_size + 3) / 4) * 4;
    }

    u8 * buf = (u8 *)malloc(total_size);
    if (buf == NULL) {
        printf("Error: failed to alloc memory\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    long result = fread(buf, 1, FwFileLen, pfFW);
    if (result != FwFileLen) {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        free(buf);
        return 0;
    }

    fclose(pfFW);

    memset(buf + FwFileLen, 0xff, FW_INFO_SIZE - FwFileLen);

    u8 * pModel = buf + FW_INFO_SIZE;

    result = fread(pModel, 1, ModelFileLen, pfModel);
    if (result != ModelFileLen) {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        free(buf);
        return 0;
    }

    fclose(pfModel);

    sum32 = kdp_gen_sum32(buf, total_size - 4);
    *(u32 *)(buf + total_size - 4) = sum32;

    fwrite(buf, total_size, 1, pfOut);
    fclose(pfOut);
    free(buf);
    return 0;
}

/*
convert the SPL binary file to the fixed size 8K for flash convenience

*/

int gen_dfu_spl(char * in_file, char * out_file)
{
    FILE *pfIn, *pfOut;

    pfIn = fopen(in_file, "rb");
    if (pfIn == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, in_file);
        return -1;
    }

    pfOut = fopen(out_file, "wb");
    if (pfOut == NULL) {
        printf("Error @ %s:%d: failed on open file %s\n", __func__, __LINE__, out_file);
        return -1;
    }

    fseek(pfIn, 0, SEEK_END);
    int inFileLen = ftell(pfIn);
    if (inFileLen > 8192) {
        printf("Error: intput SPL file is larger than 8192 bytes\n");
        return -1;
    }
    fseek(pfIn, 0, SEEK_SET);

    u8 * buf = (u8 *)malloc(8192);
    if (buf == NULL) {
        printf("Error: failed to alloc memory\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    long result = fread(buf, 1, inFileLen, pfIn);
    if (result != inFileLen) {
        printf("Error @ %s:%d: file read less bytes\n", __func__, __LINE__);
        free(buf);
        return 0;
    }

    fclose(pfIn);

    memset(buf + inFileLen, 0, 8192 - inFileLen);

    fwrite(buf, 8192, 1, pfOut);
    fclose(pfOut);
    free(buf);
    return 0;
}


/* usage: generate_dfu_binary -scpu scpu_in_file scpu_out_file
or generate_dfu_binary -ncpu ncpu_in_file ncpu_out_file
or generate_dfu_binary -model fw_info_file_in  all_model_file_in model_out_file
*/

int main(int argc, char* argv[])
{

    char *in_file, *out_file, *fw_info_file, *all_model_file;

    if ((argc != 4) && (argc != 5)) {
        printf("[KL720]This utility converts scpu/ncpu/model binary to dfu format output\n");
        printf("usage 1: generate_dfu_binary -scpu scpu_in_file scpu_out_file\n");
        printf("usage 2: generate_dfu_binary -ncpu ncpu_in_file ncpu_out_file\n");
        printf("usage 3: generate_dfu_binary -model fw_info_file_in  all_model_file_in model_out_file\n");
        printf("usage 4: generate_dfu_binary -spl spl_file_in  spl_out_file\n");
        return 0;
    }

    if (strcmp(argv[1], "-scpu") == 0) {
        in_file = argv[2];
        out_file = argv[3];
        gen_dfu_scpu(in_file, out_file);
        return 0;
    }

    if (strcmp(argv[1], "-ncpu") == 0) {
        in_file = argv[2];
        out_file = argv[3];
        gen_dfu_ncpu(in_file, out_file);
        return 0;
    }

    if (strcmp(argv[1], "-model") == 0) {
        fw_info_file = argv[2];
        all_model_file = argv[3];
        out_file = argv[4];
        gen_dfu_model(fw_info_file, all_model_file, out_file);
        return 0;
    }

    if (strcmp(argv[1], "-spl") == 0) {
        in_file = argv[2];
        out_file = argv[3];
        gen_dfu_spl(in_file, out_file);
        return 0;
    }

}


