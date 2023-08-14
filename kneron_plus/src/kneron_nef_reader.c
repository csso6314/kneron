/**
 * @file        kneron_nef_reader.c
 * @brief       NEF model related functions - read NEF
 * @version     0.1
 * @date        2021-03-22
 *
 * @copyright   Copyright (c) 2021 Kneron Inc. All rights reserved.
 */

// #define DEBUG_PRINT

#include "internal_func.h"
#include "kneron_nef_reader.h"
#include <stdio.h>

#ifdef DEBUG_PRINT
#define dbg_print(format, ...) { printf(format, ##__VA_ARGS__); fflush(stdout); }
#else
#define dbg_print(format, ...)
#endif

#define err_print(format, ...) { printf(format, ##__VA_ARGS__); fflush(stdout); }

static const uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t crc_cal(uint8_t *buf, uint32_t size)
{
	const uint8_t *p = buf;
	uint32_t crc;

	crc = ~0U;
	while (size--)
	    crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	return crc ^ ~0U;
}

int get_NEFHeader(Kneron_NEFContent_table_t* table_p, kp_metadata_t *mdata) {
    if (table_p == NULL) {
        return -1;
    }

    Kneron_NEFHeader_table_t nef_header = Kneron_NEFContent_header(*table_p);

    if (NULL == nef_header || NULL == mdata) {
        return -1;
    }

    Kneron_SchemaVersion_table_t nef_schema_version = Kneron_NEFHeader_schema_version(nef_header);

    if (nef_schema_version == NULL) {
        return -1;
    }

    mdata->crc =                            Kneron_NEFHeader_crc(nef_header);
    mdata->kn_num =                         Kneron_NEFHeader_kn_number(nef_header);
    mdata->enc_type =                       Kneron_NEFHeader_encryption_mode(nef_header);
    mdata->nef_schema_version.major =       Kneron_SchemaVersion_major_num(nef_schema_version);
    mdata->nef_schema_version.minor =       Kneron_SchemaVersion_minor_num(nef_schema_version);
    mdata->nef_schema_version.revision =    Kneron_SchemaVersion_revision_num(nef_schema_version);
    mdata->platform =                       (char *)Kneron_NEFHeader_platform(nef_header);
    mdata->tc_ver =                         (char *)Kneron_NEFHeader_toolchain_version(nef_header);
    mdata->compiler_ver =                   (char *)Kneron_NEFHeader_compiler_version(nef_header);

    /**
     * After MEF Header Schema v2.2.1 the NPUTarget Add definition DEFAULT=0 (All target shift 1, KL520 0->1, KL720 1->2, KL530 2->3...etc)
     */
    if ((mdata->nef_schema_version.major >= 2) &&
        (mdata->nef_schema_version.minor >= 2) &&
        (mdata->nef_schema_version.revision >= 1))
        mdata->target =                     Kneron_NEFHeader_target(nef_header);
    else
        mdata->target =                     Kneron_NEFHeader_target(nef_header) + 1; // align latest NPUTarget definition

#ifdef DEBUG_PRINT
    size_t platform_len =                   sizeof(char) * (flatbuffers_string_len(Kneron_NEFHeader_platform(nef_header)));
    size_t tc_ver_len =                     sizeof(char) * (flatbuffers_string_len(Kneron_NEFHeader_toolchain_version(nef_header)));
    size_t compiler_ver_len =               sizeof(char) * (flatbuffers_string_len(Kneron_NEFHeader_compiler_version(nef_header)));

    dbg_print("mdata->platform %s %lld %lld\n", mdata->platform, strlen(mdata->platform), platform_len);
    dbg_print("mdata->tc_ver %s %lld %lld\n", mdata->tc_ver, strlen(mdata->tc_ver), tc_ver_len);
    dbg_print("mdata->compiler_ver %s %lld %lld\n", mdata->compiler_ver, strlen(mdata->compiler_ver), compiler_ver_len);
    dbg_print("mdata->nef_schema_version %u.%u.%u\n", mdata->nef_schema_version.major, mdata->nef_schema_version.minor, mdata->nef_schema_version.revision);
#endif

    return 0;
}

int get_ModelInfo(Kneron_NEFContent_table_t* table_p) {
    if (table_p == NULL) {
        return -1;
    }

    Kneron_ModelInfo_vec_t model_info_v = Kneron_NEFContent_model_info(*table_p);
    if (model_info_v == NULL) {
        return -1;
    }
    //size_t size = Kneron_ModelInfo_vec_len(model_info_v);
    //printf("total model size:%ld\n", size);
    //Kneron_ModelInfo_table_t model_info;
    //for (int i = 0; i < size; i++) {
        //model_info = Kneron_ModelInfo_vec_at(model_info_v, i);
        //printf("model name: %s\n", Kneron_ModelInfo_name(model_info));
        //printf("model id: %d\n", Kneron_ModelInfo_id(model_info));
        //printf("model version: %s\n", Kneron_ModelInfo_version(model_info));
    //}
    return 0;
}

void dump_to_bin(flatbuffers_uint8_vec_t* vp, char* path) {
    int size = flatbuffers_uint8_vec_len(*vp);
    assert(size);
    FILE* fp = NULL;
    fp = fopen(path, "wb");
    if (NULL == fp) {
        printf("create dump_ota.bin fail\n");
        exit(0);
    }

    fwrite(*vp, size, 1, fp);
    fclose(fp);
    //printf("dump file: %s\n", path);
}

int get_nef_info(Kneron_NEFContent_table_t* table_p, kp_nef_info_t *nef_info) {
    if (table_p == NULL) {
        return -1;
    }

    Kneron_ModelBin_table_t model_bin = Kneron_NEFContent_model_bin(*table_p);
    if (NULL ==  model_bin || NULL == nef_info) {
        return -1;
    }

    flatbuffers_uint8_vec_t fw_info = Kneron_ModelBin_fw_info(model_bin);
    nef_info->fw_info_addr = (char *)fw_info;
    nef_info->fw_info_size = flatbuffers_uint8_vec_len(fw_info);
    // dump_to_bin(&fw_info, "dump_fw_info.bin");

    flatbuffers_uint8_vec_t all_models = Kneron_ModelBin_all_models(model_bin);
    nef_info->all_models_addr = (char *)all_models;
    nef_info->all_models_size = flatbuffers_uint8_vec_len(all_models);
    // dump_to_bin(&all_models, "dump_all_models.bin");
    return 0;
}

int read_nef(char* nef_data,
             uint32_t nef_size,
             kp_metadata_t *metadata,
             kp_nef_info_t *nef_info) {
    int nef_crc = crc_cal((uint8_t *)nef_data, nef_size-4);
    int crc = *(uint32_t *)&(nef_data[nef_size-4]);

    if (crc != nef_crc) {
        err_print("Bad model.\n");
        return -1;
    }

    int ret = 0;
    Kneron_NEFContent_table_t table = Kneron_NEFContent_as_root(nef_data);

    if (table == NULL) {
        return -1;
    }
    ret = get_NEFHeader(&table, metadata);

    if (ret != 0) {
        return -1;
    }
    ret = get_ModelInfo(&table);

    if (ret != 0) {
        return -1;
    }
    ret = get_nef_info(&table, nef_info);

    if (ret != 0) {
        return -1;
    }

    // update target chip information from header
    nef_info->target = metadata->target;

    return 0;
}
