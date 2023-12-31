#include "kdpio.h"

#define ADDR_NPU_CODE           0x30ff0000
#define ADDR_NPU_CLEN           0x30ff0004
#define ADDR_NPU_RUN            0x30ff0008
#define ADDR_NPU_ELEN           0x30ff000c
#define ADDR_NPU_VER            0x30ff0010
#define ADDR_NPU_MODE           0x30ff0014
#define ADDR_NPU_DMA            0x30ff0018
#define ADDR_NPU_RDMA0_SRC0     0x30ff001c
#define ADDR_NPU_RDMA0_SRC1     0x30ff0020
#define ADDR_NPU_RDMA0_SRC2     0x30ff0024
#define ADDR_NPU_RDMA0_DST      0x30ff0028
#define ADDR_NPU_RDMA0_BLK      0x30ff002c
#define ADDR_NPU_RDMA1_SRC0     0x30ff0030
#define ADDR_NPU_RDMA1_SRC1     0x30ff0034
#define ADDR_NPU_RDMA1_SRC2     0x30ff0038
#define ADDR_NPU_RDMA1_DST      0x30ff003c
#define ADDR_NPU_RDMA1_BLK      0x30ff0040
#define ADDR_NPU_RDMA2_SRC0     0x30ff0044
#define ADDR_NPU_RDMA2_SRC1     0x30ff0048
#define ADDR_NPU_RDMA2_SRC2     0x30ff004c
#define ADDR_NPU_RDMA2_SRC3     0x30ff0050
#define ADDR_NPU_RDMA2_DST      0x30ff0054
#define ADDR_NPU_RDMA2_BLK      0x30ff0058
#define ADDR_NPU_GETW0          0x30ff005c
#define ADDR_NPU_GETW1          0x30ff0060
#define ADDR_NPU_GETW2          0x30ff0064
#define ADDR_NPU_WDMA0_SRC      0x30ff0068
#define ADDR_NPU_WDMA0_DST0     0x30ff006c
#define ADDR_NPU_WDMA0_DST1     0x30ff0070
#define ADDR_NPU_WDMA0_DST2     0x30ff0074
#define ADDR_NPU_WDMA0_BLK      0x30ff0078
#define ADDR_NPU_WDMA1_SRC      0x30ff007c
#define ADDR_NPU_WDMA1_DST0     0x30ff0080
#define ADDR_NPU_WDMA1_DST1     0x30ff0084
#define ADDR_NPU_WDMA1_DST2     0x30ff0088
#define ADDR_NPU_WDMA1_BLK      0x30ff008c
#define ADDR_NPU_NMEM_FM0       0x30ff0090
#define ADDR_NPU_NMEM_FM1       0x30ff0094
#define ADDR_NPU_NMEM_FM2       0x30ff0098
#define ADDR_NPU_NMEM_PS0       0x30ff009c
#define ADDR_NPU_NMEM_PS1       0x30ff00a0
#define ADDR_NPU_NMEM_PS2       0x30ff00a4
#define ADDR_NPU_NMEM_ST0       0x30ff00a8
#define ADDR_NPU_NMEM_ST1       0x30ff00ac
#define ADDR_NPU_NMEM_ST2       0x30ff00b0
#define ADDR_NPU_NMEM_RDMA0     0x30ff00b4
#define ADDR_NPU_NMEM_RDMA1     0x30ff00b8
#define ADDR_NPU_NMEM_RDMA2     0x30ff00bc
#define ADDR_NPU_NMEM_WDMA0     0x30ff00c0
#define ADDR_NPU_NMEM_WDMA1     0x30ff00c4
#define ADDR_NPU_NMEM_WDMA2     0x30ff00c8
#define ADDR_NPU_NMEM_WT        0x30ff00cc
#define ADDR_NPU_NMEM_LB        0x30ff00d0
#define ADDR_NPU_CONV           0x30ff00d4
#define ADDR_NPU_FMAP0          0x30ff00d8
#define ADDR_NPU_FMAP1          0x30ff00dc
#define ADDR_NPU_ZPAD           0x30ff00e0
#define ADDR_NPU_NEXT0          0x30ff00e4
#define ADDR_NPU_NEXT1          0x30ff00e8
#define ADDR_NPU_NEXT2          0x30ff00ec
#define ADDR_NPU_LWT            0x30ff00f0
#define ADDR_NPU_STORE          0x30ff00f4
#define ADDR_NPU_TRIM           0x30ff00f8
#define ADDR_NPU_TRIM0          0x30ff00fc
#define ADDR_NPU_RESHAPE        0x30ff0100
#define ADDR_NPU_PCONV          0x30ff0104
#define ADDR_NPU_BN             0x30ff0108
#define ADDR_NPU_RELU           0x30ff010c
#define ADDR_NPU_RELU6          0x30ff0110
#define ADDR_NPU_POOL           0x30ff0114
#define ADDR_NPU_GAP            0x30ff0118
#define ADDR_NPU_FORMAT         0x30ff011c
#define ADDR_NPU_RESIZE         0x30ff0120
#define ADDR_NPU_RESIZE_SRC     0x30ff0124
#define ADDR_NPU_RESIZE_DST     0x30ff0128
#define ADDR_NPU_RESIZE_RATIO0    0x30ff012c
#define ADDR_NPU_RESIZE_RATIO1    0x30ff0130
#define ADDR_NPU_CHAN           0x30ff0134
#define ADDR_NPU_PREP0          0x30ff0138
#define ADDR_NPU_PREP1          0x30ff013c
#define ADDR_NPU_PREP2          0x30ff0140
#define ADDR_NPU_ENCRYPT        0x30ff0144
#define ADDR_NPU_KEY            0x30ff0148
#define ADDR_NPU_INTEN          0x30ff014c
#define ADDR_NPU_INT            0x30ff0150
#define ADDR_NPU_DBG0           0x30ff0154
#define ADDR_NPU_DBG1           0x30ff0158
#define ADDR_NPU_DBG2           0x30ff015c
#define ADDR_NPU_REGM           0x30ff0160
#define ADDR_NPU_DUMMY_0        0x30ff0164
#define ADDR_NPU_DUMMY_1        0x30ff0168
#define ADDR_NPU_DUMMY_2        0x30ff016c

#define VAL_ACL(x)    (((x)&0xffff))
#define VAL_ACH(x)    (((x)>>16)&0xffff)

#define ACL_NPU_MODE        0xa
#define ACH_NPU_MODE        0xb
#define ACL_NPU_RDMA0_SRC0  0xe
#define ACH_NPU_RDMA0_SRC0  0xf
#define ACL_NPU_RDMA0_SRC1  0x10
#define ACH_NPU_RDMA0_SRC1  0x11
#define ACL_NPU_RDMA0_SRC2  0x12
#define ACH_NPU_RDMA0_SRC2  0x13
#define ACL_NPU_RDMA0_DST   0x14
#define ACH_NPU_RDMA0_DST   0x15
#define ACL_NPU_RDMA0_BLK   0x16
#define ACH_NPU_RDMA0_BLK   0x17
#define ACL_NPU_RDMA1_SRC0  0x18
#define ACH_NPU_RDMA1_SRC0  0x19
#define ACL_NPU_RDMA1_SRC1  0x1a
#define ACH_NPU_RDMA1_SRC1  0x1b
#define ACL_NPU_RDMA1_SRC2  0x1c
#define ACH_NPU_RDMA1_SRC2  0x1d
#define ACL_NPU_RDMA1_DST   0x1e
#define ACH_NPU_RDMA1_DST   0x1f
#define ACL_NPU_RDMA1_BLK   0x20
#define ACH_NPU_RDMA1_BLK   0x21
#define ACL_NPU_RDMA2_SRC0  0x22
#define ACH_NPU_RDMA2_SRC0  0x23
#define ACL_NPU_RDMA2_SRC1  0x24
#define ACH_NPU_RDMA2_SRC1  0x25
#define ACL_NPU_RDMA2_SRC2  0x26
#define ACH_NPU_RDMA2_SRC2  0x27
#define ACL_NPU_RDMA2_SRC3  0x28
#define ACH_NPU_RDMA2_SRC3  0x29
#define ACL_NPU_RDMA2_DST   0x2a
#define ACH_NPU_RDMA2_DST   0x2b
#define ACL_NPU_RDMA2_BLK   0x2c
#define ACH_NPU_RDMA2_BLK   0x2d
#define ACL_NPU_GETW0       0x2e
#define ACH_NPU_GETW0       0x2f
#define ACL_NPU_GETW1       0x30
#define ACH_NPU_GETW1       0x31
#define ACL_NPU_GETW2       0x32
#define ACH_NPU_GETW2       0x33
#define ACL_NPU_WDMA0_SRC   0x34
#define ACH_NPU_WDMA0_SRC   0x35
#define ACL_NPU_WDMA0_DST0  0x36
#define ACH_NPU_WDMA0_DST0  0x37
#define ACL_NPU_WDMA0_DST1  0x38
#define ACH_NPU_WDMA0_DST1  0x39
#define ACL_NPU_WDMA0_DST2  0x3a
#define ACH_NPU_WDMA0_DST2  0x3b
#define ACL_NPU_WDMA0_BLK   0x3c
#define ACH_NPU_WDMA0_BLK   0x3d
#define ACL_NPU_WDMA1_SRC   0x3e
#define ACH_NPU_WDMA1_SRC   0x3f
#define ACL_NPU_WDMA1_DST0  0x40
#define ACH_NPU_WDMA1_DST0  0x41
#define ACL_NPU_WDMA1_DST1  0x42
#define ACH_NPU_WDMA1_DST1  0x43
#define ACL_NPU_WDMA1_DST2  0x44
#define ACH_NPU_WDMA1_DST2  0x45
#define ACL_NPU_WDMA1_BLK   0x46
#define ACH_NPU_WDMA1_BLK   0x47
#define ACL_NPU_NMEM_FM0    0x48
#define ACH_NPU_NMEM_FM0    0x49
#define ACL_NPU_NMEM_FM1    0x4a
#define ACH_NPU_NMEM_FM1    0x4b
#define ACL_NPU_NMEM_FM2    0x4c
#define ACH_NPU_NMEM_FM2    0x4d
#define ACL_NPU_NMEM_PS0    0x4e
#define ACH_NPU_NMEM_PS0    0x4f
#define ACL_NPU_NMEM_PS1    0x50
#define ACH_NPU_NMEM_PS1    0x51
#define ACL_NPU_NMEM_PS2    0x52
#define ACH_NPU_NMEM_PS2    0x53
#define ACL_NPU_NMEM_ST0    0x54
#define ACH_NPU_NMEM_ST0    0x55
#define ACL_NPU_NMEM_ST1    0x56
#define ACH_NPU_NMEM_ST1    0x57
#define ACL_NPU_NMEM_ST2    0x58
#define ACH_NPU_NMEM_ST2    0x59
#define ACL_NPU_NMEM_RDMA0  0x5a
#define ACH_NPU_NMEM_RDMA0  0x5b
#define ACL_NPU_NMEM_RDMA1  0x5c
#define ACH_NPU_NMEM_RDMA1  0x5d
#define ACL_NPU_NMEM_RDMA2  0x5e
#define ACH_NPU_NMEM_RDMA2  0x5f
#define ACL_NPU_NMEM_WDMA0  0x60
#define ACH_NPU_NMEM_WDMA0  0x61
#define ACL_NPU_NMEM_WDMA1  0x62
#define ACH_NPU_NMEM_WDMA1  0x63
#define ACL_NPU_NMEM_WDMA2  0x64
#define ACH_NPU_NMEM_WDMA2  0x65
#define ACL_NPU_NMEM_WT     0x66
#define ACH_NPU_NMEM_WT     0x67
#define ACL_NPU_NMEM_LB     0x68
#define ACH_NPU_NMEM_LB     0x69
#define ACL_NPU_CONV        0x6a
#define ACH_NPU_CONV        0x6b
#define ACL_NPU_FMAP0       0x6c
#define ACH_NPU_FMAP0       0x6d
#define ACL_NPU_FMAP1       0x6e
#define ACH_NPU_FMAP1       0x6f
#define ACL_NPU_ZPAD        0x70
#define ACH_NPU_ZPAD        0x71
#define ACL_NPU_NEXT0       0x72
#define ACH_NPU_NEXT0       0x73
#define ACL_NPU_NEXT1       0x74
#define ACH_NPU_NEXT1       0x75
#define ACL_NPU_NEXT2       0x76
#define ACH_NPU_NEXT2       0x77
#define ACL_NPU_LWT         0x78
#define ACH_NPU_LWT         0x79
#define ACL_NPU_STORE       0x7a
#define ACH_NPU_STORE       0x7b
#define ACL_NPU_TRIM        0x7c
#define ACH_NPU_TRIM        0x7d
#define ACL_NPU_TRIM0       0x7e
#define ACH_NPU_TRIM0       0x7f
#define ACL_NPU_RESHAPE     0x80
#define ACH_NPU_RESHAPE     0x81
#define ACL_NPU_PCONV       0x82
#define ACH_NPU_PCONV       0x83
#define ACL_NPU_BN          0x84
#define ACH_NPU_BN          0x85
#define ACL_NPU_RELU        0x86
#define ACH_NPU_RELU        0x87
#define ACL_NPU_RELU6       0x88
#define ACH_NPU_RELU6       0x89
#define ACL_NPU_POOL        0x8a
#define ACH_NPU_POOL        0x8b
#define ACL_NPU_GAP         0x8c
#define ACH_NPU_GAP         0x8d
#define ACL_NPU_FORMAT      0x8e
#define ACH_NPU_FORMAT      0x8f
#define ACL_NPU_RESIZE      0x90
#define ACH_NPU_RESIZE      0x91
#define ACL_NPU_RESIZE_SRC  0x92
#define ACH_NPU_RESIZE_SRC  0x93
#define ACL_NPU_RESIZE_DST  0x94
#define ACH_NPU_RESIZE_DST  0x95
#define ACL_NPU_RESIZE_RATIO0 0x96
#define ACH_NPU_RESIZE_RATIO0 0x97
#define ACL_NPU_RESIZE_RATIO1 0x98
#define ACH_NPU_RESIZE_RATIO1 0x99
#define ACL_NPU_CHAN        0x9a
#define ACH_NPU_CHAN        0x9b
#define ACL_NPU_PREP0       0x9c
#define ACH_NPU_PREP0       0x9d
#define ACL_NPU_PREP1       0x9e
#define ACH_NPU_PREP1       0x9f
#define ACL_NPU_PREP2       0xa0
#define ACH_NPU_PREP2       0xa1

#define NPU_CODE_a(x)                   (((x)&0xffffffff)<<0)
#define NPU_CLEN_l(x)                   (((x)&0xfffff)<<0)
#define NPU_RUN_conv(x)                 (((x)&0x1)<<23)
#define NPU_RUN_getw(x)                 (((x)&0x1)<<22)
#define NPU_RUN_wdma(x)                 (((x)&0x1)<<21)
#define NPU_RUN_rdma(x)                 (((x)&0x1)<<20)
#define NPU_RUN_wfc(x)                  (((x)&0x1)<<17)
#define NPU_RUN_busy(x)                 (((x)&0x1)<<16)
#define NPU_RUN_slp(x)                  (((x)&0x1)<<9)
#define NPU_RUN_ret(x)                  (((x)&0x1)<<8)
#define NPU_RUN_conti(x)                (((x)&0x1)<<1)
#define NPU_RUN_go(x)                   (((x)&0x1)<<0)
#define NPU_ELEN_l(x)                   (((x)&0xfffff)<<0)
#define NPU_VER_num(x)                  (((x)&0xffff)<<0)
#define NPU_MODE_8b_3x3_conv(x)         (((x)&0x1)<<0)
#define NPU_DMA_bl(x)                   (((x)&0x1fff)<<0)
#define NPU_RDMA0_SRC0_sa(x)            (((x)&0xffffffff)<<0)
#define NPU_RDMA0_SRC1_pitch(x)         (((x)&0xffffff)<<0)
#define NPU_RDMA0_SRC2_len(x)           (((x)&0xffff)<<0)
#define NPU_RDMA0_DST_dl(x)             (((x)&0xffff)<<0)
#define NPU_RDMA0_BLK_line(x)           (((x)&0xffff)<<0)
#define NPU_RDMA1_SRC0_sa(x)            (((x)&0xffffffff)<<0)
#define NPU_RDMA1_SRC1_pitch(x)         (((x)&0xffffff)<<0)
#define NPU_RDMA1_SRC2_len(x)           (((x)&0xffff)<<0)
#define NPU_RDMA1_DST_dl(x)             (((x)&0xffff)<<0)
#define NPU_RDMA1_BLK_line(x)           (((x)&0xffff)<<0)
#define NPU_RDMA2_SRC0_sa(x)            (((x)&0xffffffff)<<0)
#define NPU_RDMA2_SRC1_pitch(x)         (((x)&0xffffff)<<0)
#define NPU_RDMA2_SRC2_len(x)           (((x)&0xffff)<<0)
#define NPU_RDMA2_SRC3_base(x)          (((x)&0xffffffff)<<0)
#define NPU_RDMA2_DST_dl(x)             (((x)&0xffff)<<0)
#define NPU_RDMA2_BLK_line(x)           (((x)&0xffff)<<0)
#define NPU_GETW0_sa(x)                 (((x)&0xffffffff)<<0)
#define NPU_GETW1_len(x)                (((x)&0xfffffff)<<0)
#define NPU_GETW2_back(x)               (((x)&0x7fff)<<0)
#define NPU_WDMA0_SRC_sl(x)             (((x)&0xffff)<<0)
#define NPU_WDMA0_DST0_da(x)            (((x)&0xffffffff)<<0)
#define NPU_WDMA0_DST1_pitch(x)         (((x)&0xffffff)<<0)
#define NPU_WDMA0_DST2_len(x)           (((x)&0xffff)<<0)
#define NPU_WDMA0_BLK_line(x)           (((x)&0xffff)<<0)
#define NPU_WDMA1_SRC_sl(x)             (((x)&0xffff)<<0)
#define NPU_WDMA1_DST0_da(x)            (((x)&0xffffffff)<<0)
#define NPU_WDMA1_DST1_pitch(x)         (((x)&0xffffff)<<0)
#define NPU_WDMA1_DST2_len(x)           (((x)&0xffff)<<0)
#define NPU_WDMA1_BLK_line(x)           (((x)&0xffff)<<0)
#define NPU_NMEM_FM0_offset_x(x)        (((x)&0xfff)<<16)
#define NPU_NMEM_FM0_offset_y(x)        (((x)&0x3f)<<8)
#define NPU_NMEM_FM0_config(x)          (((x)&0xf)<<0)
#define NPU_NMEM_FM1_h(x)               (((x)&0xffff)<<16)
#define NPU_NMEM_FM1_w(x)               (((x)&0xfff)<<0)
#define NPU_NMEM_FM2_l(x)               (((x)&0xfff)<<0)
#define NPU_NMEM_PS0_offset_x(x)        (((x)&0xfff)<<16)
#define NPU_NMEM_PS0_offset_y(x)        (((x)&0x3f)<<8)
#define NPU_NMEM_PS0_config(x)          (((x)&0xf)<<0)
#define NPU_NMEM_PS1_h(x)               (((x)&0xffff)<<16)
#define NPU_NMEM_PS1_w(x)               (((x)&0xfff)<<0)
#define NPU_NMEM_PS2_l(x)               (((x)&0xfff)<<0)
#define NPU_NMEM_ST0_offset_x(x)        (((x)&0xfff)<<16)
#define NPU_NMEM_ST0_offset_y(x)        (((x)&0x3f)<<8)
#define NPU_NMEM_ST0_config(x)          (((x)&0xf)<<0)
#define NPU_NMEM_ST1_h(x)               (((x)&0xffff)<<16)
#define NPU_NMEM_ST1_w(x)               (((x)&0xfff)<<0)
#define NPU_NMEM_ST2_l(x)               (((x)&0xfff)<<0)
#define NPU_NMEM_RDMA0_offset_x(x)      (((x)&0xfff)<<16)
#define NPU_NMEM_RDMA0_offset_y(x)      (((x)&0x3f)<<8)
#define NPU_NMEM_RDMA0_config(x)        (((x)&0xf)<<0)
#define NPU_NMEM_RDMA1_h(x)             (((x)&0xffff)<<16)
#define NPU_NMEM_RDMA1_w(x)             (((x)&0xfff)<<0)
#define NPU_NMEM_RDMA2_l(x)             (((x)&0xfff)<<0)
#define NPU_NMEM_WDMA0_offset_x(x)      (((x)&0xfff)<<16)
#define NPU_NMEM_WDMA0_offset_y(x)      (((x)&0x3f)<<8)
#define NPU_NMEM_WDMA0_config(x)        (((x)&0xf)<<0)
#define NPU_NMEM_WDMA1_h(x)             (((x)&0xffff)<<16)
#define NPU_NMEM_WDMA1_w(x)             (((x)&0xfff)<<0)
#define NPU_NMEM_WDMA2_l(x)             (((x)&0xfff)<<0)
#define NPU_NMEM_WT_restart(x)          (((x)&0x1)<<31)
#define NPU_NMEM_WT_sa(x)               (((x)&0xfff)<<16)
#define NPU_NMEM_WT_ea(x)               (((x)&0xfff)<<0)
#define NPU_NMEM_LB_restart(x)          (((x)&0x1)<<31)
#define NPU_NMEM_LB_sa(x)               (((x)&0xfff)<<16)
#define NPU_NMEM_LB_ea(x)               (((x)&0xfff)<<0)
#define NPU_CONV_en(x)                  (((x)&0x1)<<31)
#define NPU_CONV_ch_resume(x)           (((x)&0x1)<<30)
#define NPU_CONV_full_ch(x)             (((x)&0x1)<<29)
#define NPU_CONV_ps_och(x)              (((x)&0x1)<<28)
#define NPU_CONV_add_shift(x)           (((x)&0x1)<<20)
#define NPU_CONV_pformat(x)             (((x)&0x3)<<16)
#define NPU_CONV_zero(x)                (((x)&0x1)<<10)
#define NPU_CONV_step(x)                (((x)&0x1)<<9)
#define NPU_CONV_hstride2(x)            (((x)&0x1)<<8)
#define NPU_CONV_mode(x)                (((x)&0xff)<<0)
#define NPU_FMAP0_row(x)                (((x)&0x1fff)<<16)
#define NPU_FMAP0_col(x)                (((x)&0x1fff)<<0)
#define NPU_FMAP1_ch(x)                 (((x)&0x1fff)<<0)
#define NPU_ZPAD_b(x)                   (((x)&0xf)<<12)
#define NPU_ZPAD_r(x)                   (((x)&0xf)<<8)
#define NPU_ZPAD_l(x)                   (((x)&0xf)<<4)
#define NPU_ZPAD_t(x)                   (((x)&0xf)<<0)
#define NPU_NEXT0_ch_end(x)             (((x)&0x1fff)<<16)
#define NPU_NEXT0_ch_start(x)           (((x)&0x1fff)<<0)
#define NPU_NEXT1_ch_total(x)           (((x)&0x1fff)<<16)
#define NPU_NEXT1_format(x)             (((x)&0xf)<<0)
#define NPU_NEXT2_line(x)               (((x)&0xfff)<<0)
#define NPU_LWT_prefetch(x)             (((x)&0x1)<<2)
#define NPU_LWT_w16b(x)                 (((x)&0x1)<<1)
#define NPU_LWT_decomp(x)               (((x)&0x1)<<0)
#define NPU_STORE_en(x)                 (((x)&0x1)<<31)
#define NPU_TRIM_wo(x)                  (((x)&0xf)<<24)
#define NPU_TRIM_wv(x)                  (((x)&0xf)<<20)
#define NPU_TRIM_w(x)                   (((x)&0xf)<<16)
#define NPU_TRIM_ho(x)                  (((x)&0xf)<<4)
#define NPU_TRIM_hm(x)                  (((x)&0xf)<<0)
#define NPU_TRIM0_hc(x)                 (((x)&0xffff)<<16)
#define NPU_TRIM0_wc(x)                 (((x)&0xffff)<<0)
#define NPU_RESHAPE_ch(x)               (((x)&0xffff)<<16)
#define NPU_RESHAPE_ch_max(x)           (((x)&0xffff)<<0)
#define NPU_PCONV_en(x)                 (((x)&0x1)<<31)
#define NPU_PCONV_shift(x)              (((x)&0x1f)<<8)
#define NPU_PCONV_pool(x)               (((x)&0x1)<<6)
#define NPU_PCONV_relu(x)               (((x)&0x1)<<5)
#define NPU_PCONV_bn(x)                 (((x)&0x1)<<4)
#define NPU_PCONV_order(x)              (((x)&0x7)<<0)
#define NPU_BN_mode(x)                  (((x)&0xff)<<0)
#define NPU_RELU_mode(x)                (((x)&0x7)<<0)
#define NPU_RELU6_clamp(x)              (((x)&0xffffffff)<<0)
#define NPU_POOL_zpad_r(x)              (((x)&0x3)<<22)
#define NPU_POOL_zpad_l(x)              (((x)&0x3)<<20)
#define NPU_POOL_zpad_b(x)              (((x)&0x3)<<18)
#define NPU_POOL_zpad_t(x)              (((x)&0x3)<<16)
#define NPU_POOL_stride(x)              (((x)&0x3)<<8)
#define NPU_POOL_size(x)                (((x)&0x1)<<3)
#define NPU_POOL_mode(x)                (((x)&0x3)<<0)
#define NPU_GAP_shift(x)                (((x)&0x1f)<<16)
#define NPU_GAP_mul(x)                  (((x)&0xffff)<<0)
#define NPU_FORMAT_en(x)                (((x)&0x1)<<31)
#define NPU_FORMAT_uv_sub_128(x)        (((x)&0x1)<<9)
#define NPU_FORMAT_y_sub_128(x)         (((x)&0x1)<<8)
#define NPU_FORMAT_mode(x)              (((x)&0xff)<<0)
#define NPU_RESIZE_en(x)                (((x)&0x1)<<31)
#define NPU_RESIZE_restart(x)           (((x)&0x1)<<30)
#define NPU_RESIZE_SRC_offset(x)        (((x)&0x3)<<0)
#define NPU_RESIZE_DST_h(x)             (((x)&0x1ff)<<16)
#define NPU_RESIZE_DST_w(x)             (((x)&0x1ff)<<0)
#define NPU_RESIZE_RATIO0_w(x)          (((x)&0xfffff)<<0)
#define NPU_RESIZE_RATIO1_h(x)          (((x)&0xfffff)<<0)
#define NPU_CHAN_en(x)                  (((x)&0x1)<<31)
#define NPU_CHAN_16b(x)                 (((x)&0x1)<<0)
#define NPU_PREP0_sub(x)                (((x)&0xffff)<<16)
#define NPU_PREP0_pad(x)                (((x)&0xffff)<<0)
#define NPU_PREP1_pad_r(x)              (((x)&0xff)<<24)
#define NPU_PREP1_pad_l(x)              (((x)&0xff)<<16)
#define NPU_PREP1_pad_b(x)              (((x)&0xff)<<8)
#define NPU_PREP1_pad_t(x)              (((x)&0xff)<<0)
#define NPU_PREP2_shift(x)              (((x)&0xf)<<0)
#define NPU_ENCRYPT_wt(x)               (((x)&0x1)<<1)
#define NPU_ENCRYPT_inst(x)             (((x)&0x1)<<0)
#define NPU_KEY_k(x)                    (((x)&0xffffffff)<<0)
#define NPU_INTEN_err_inst(x)           (((x)&0x1)<<11)
#define NPU_INTEN_err_wt(x)             (((x)&0x1)<<10)
#define NPU_INTEN_err_config(x)         (((x)&0x1)<<9)
#define NPU_INTEN_err_ahb(x)            (((x)&0x1)<<8)
#define NPU_INTEN_int(x)                (((x)&0xff)<<0)
#define NPU_INT_err_inst(x)             (((x)&0x1)<<11)
#define NPU_INT_err_wt(x)               (((x)&0x1)<<10)
#define NPU_INT_err_config(x)           (((x)&0x1)<<9)
#define NPU_INT_err_ahb(x)              (((x)&0x1)<<8)
#define NPU_INT_int(x)                  (((x)&0xff)<<0)
#define NPU_DBG0_m(x)                   (((x)&0xff)<<0)
#define NPU_DBG1_v(x)                   (((x)&0xffffffff)<<0)
#define NPU_DBG2_step(x)                (((x)&0x1)<<0)
#define NPU_REGM_c(x)                   (((x)&0xffffffff)<<0)
#define NPU_DUMMY_0_b(x)                (((x)&0xffffffff)<<0)
#define NPU_DUMMY_1_b(x)                (((x)&0xffffffff)<<0)
#define NPU_DUMMY_2_b(x)                (((x)&0xffffffff)<<0)

void kdp_init_npu(void);
void kdp_exit_npu(void);
void kdp_enable_npu(struct kdp_image_s *image_p, uint32_t key);
void kdp_enable_npu_cont(void);
int kdp_read_data_size(void);
int kdp_get_pixel_size(uint32_t format);
uint32_t kdp_get_channel_size(uint32_t format);
void kdp_enable_npu_int(void);
void kdp_disable_npu_int(void);
void kdp_enable_npu_preproc(uint32_t cmd_addr, int len);
int kdp_wait_for_npu_done(int timeout_count);
int kdp_handle_int(void);

