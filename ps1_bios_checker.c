/**
 * @file ps1_bios_checker.c
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief PS1 BIOSのチェッカー
 * @version 0.1
 * @date 2026-08-27
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ---------------------------------------------------
#define PS1_BIOS_CHECKER_VERSION    0.1f
#define CRC32_TBL_SIZE              256
#define BIOS_BUF_SIZE_BYTE          (512 * 1024)

// NOTE: PS1のBIOSは通常512KB (524288 bytes)
#define BIOS_FILE_SIZE_BYTE         BIOS_BUF_SIZE_BYTE

// BIOSのCRC32、PS1の型番名、BIOS名のテーブル構造体
typedef struct {
    uint32_t crc32;             // CRC32 期待値
    const char *p_ps1_name;     // PS1の型番名
    const char *p_bios_name;    // BIOS名
} bios_crc32_t;

// 日本版 (NTSC-J)
const bios_crc32_t g_bios_crc32_tbl_jpn[] = {
    { 0x3B601FC8, "SCPH-1000",                  "ps-10j"    }, // BIOS: v1.0J
    { 0x3539DEF6, "SCPH-3000",                  "ps-11j"    }, // BIOS: v1.1J
    { 0xBC190209, "SCPH-3500",                  "ps-21j"    }, // BIOS: v2.1J
    { 0x24FC7E17, "SCPH-5000",                  "ps-22j"    }, // BIOS: v2.2J
    { 0xFF3EEB8C, "SCPH-5500",                  "ps-30j"    }, // BIOS: v3.0J
    { 0xEC541CD0, "SCPH-7000/7500/9000",        "ps-40j"    }, // BIOS: v4.0J
    { 0x1E68C234, "SCPH-9000",                  "ps-43j"    }, // BIOS: v4.3J
    { 0xF2AF798B, "SCPH-100 (PS One)",          "psone-43j" }, // BIOS: v4.3J (PS One)
};
const uint8_t BIOS_CRC32_JPN_TBL_CNT = sizeof(g_bios_crc32_tbl_jpn) / sizeof(g_bios_crc32_tbl_jpn[0]);

// 北米版 (NTSC-U/C)
const bios_crc32_t g_bios_crc32_tbl_ntsc[] = {
    { 0x55847D8C, "SCPH-1001",                  "ps-20a"    }, // BIOS: v2.0A
    { 0xAFF00F2F, "SCPH-1001 / DTL-H1101",      "ps-21a"    }, // BIOS: v2.1A
    { 0x37157331, "SCPH-1001 / DTL-H1201/3001", "ps-22a"    }, // BIOS: v2.2A
    { 0x8D8CB7E4, "SCPH-5501/5503/7003",        "ps-30a"    }, // BIOS: v3.0A
    { 0x502224B6, "SCPH-7001/7501/9001",        "ps-41a"    }, // BIOS: v4.1A
    { 0x171BDCEC, "SCPH-101 (PS One)",          "psone-45a" }, // BIOS: v4.5A (PS One)
};
const uint8_t BIOS_CRC32_NTSC_TBL_CNT = sizeof(g_bios_crc32_tbl_ntsc) / sizeof(g_bios_crc32_tbl_ntsc[0]);

// 欧州版 (PAL)
const bios_crc32_t g_bios_crc32_tbl_pal[] = {
    { 0x98AFA9DB, "SCPH-1002",                  "ps-20e"    }, // BIOS: v2.0E
    { 0x86C30531, "SCPH-1002 / DTL-H1102",      "ps-21e"    }, // BIOS: v2.1E
    { 0x1A25F706, "SCPH-1002 / DTL-H1202/3002", "ps-22e"    }, // BIOS: v2.2E
    { 0xD786F0B9, "SCPH-5502/5552",             "ps-30e"    }, // BIOS: v3.0E
    { 0x318178BF, "SCPH-7002/7502/9002",        "ps-41e"    }, // BIOS: v4.1E
    { 0xE0DFB769, "SCPH-102 (PS One)",          "psone-44e" }, // BIOS: v4.4E (PS One)
    { 0x5E6B56F5, "SCPH-102 (PS One) RevB",     "psone-45e" }, // BIOS: v4.5E (PS One)
};
const uint8_t BIOS_CRC32_PAL_TBL_CNT = sizeof(g_bios_crc32_tbl_pal) / sizeof(g_bios_crc32_tbl_pal[0]);

// その他機種
const bios_crc32_t g_bios_crc32_tbl_ext[] = {
    { 0x32736F17, "SCPH-5903 (Video CD)",       "ps-30j_vcd"}, // BIOS: v3.0J (VCD搭載機)
    { 0x4CE8FEE9, "DTL-H1000H/DTL-H1100",       "ps-11j_dbg"}, // BIOS: v1.1J (デバッギングステーション)
    { 0x5C464BE0, "DTL-H1200 / DTL-H3000",      "ps-22j_dbg"}  // BIOS: v2.2J (ネットやろうぜ！ PS1)
};
const uint8_t BIOS_CRC32_EXT_TBL_CNT = sizeof(g_bios_crc32_tbl_ext) / sizeof(g_bios_crc32_tbl_ext[0]);

typedef struct {
    const bios_crc32_t *p_tbl;
    uint8_t tbl_cnt;
} bios_crc32_tbl_t;

const bios_crc32_tbl_t g_bios_crc32_tbl[] = {
    { &g_bios_crc32_tbl_jpn[0], BIOS_CRC32_JPN_TBL_CNT },
    { &g_bios_crc32_tbl_ntsc[0], BIOS_CRC32_NTSC_TBL_CNT },
    { &g_bios_crc32_tbl_pal[0], BIOS_CRC32_PAL_TBL_CNT },
    { &g_bios_crc32_tbl_ext[0], BIOS_CRC32_EXT_TBL_CNT }
};
const uint8_t BIOS_CRC32_TBL_CNT = sizeof(g_bios_crc32_tbl) / sizeof(g_bios_crc32_tbl[0]);

static uint8_t s_bios_buf[BIOS_BUF_SIZE_BYTE] = {0};
static uint32_t s_crc32_tbl[CRC32_TBL_SIZE] = {0};
static bool s_is_tbl_init = false;

static void _crc32_tbl_init(void);
static uint32_t _calc_crc32(const uint8_t *p_buf, size_t size);
static void _check_bios_file(const char *p_filepath);
// ---------------------------------------------------
// [Static]

// CRC32テーブルの生成
static void _crc32_tbl_init(void)
{
    uint32_t crc;
    uint32_t i;
    uint32_t j;

    memset(&s_crc32_tbl[0], 0, sizeof(s_crc32_tbl));

    for (i = 0; i < CRC32_TBL_SIZE; i++)
    {
        crc = i;
        for (j = 0; j < 8; j++)
        {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc = (crc >> 1);
            }
        }
        s_crc32_tbl[i] = crc;
    }

    s_is_tbl_init = true;
}

// CRC32の計算関数
static uint32_t _calc_crc32(const uint8_t *p_buf, size_t size)
{
    uint32_t crc;
    size_t i;
    uint8_t idx;

    if (!s_is_tbl_init) {
        _crc32_tbl_init();
    }

    crc = 0xFFFFFFFF;

    for (i = 0; i < size; i++)
    {
        idx = (uint8_t)((crc ^ p_buf[i]) & 0xFF);
        crc = s_crc32_tbl[idx] ^ (crc >> 8);
    }

    return ~crc;
}

// BIOSファイルを読み込んでチェックする関数
static void _check_bios_file(const char *p_filepath)
{
    FILE *p_fp;
    long file_size;
    bios_crc32_t *p_tbl;
    uint8_t tbl_cnt;
    uint32_t calc_crc;
    size_t h, i;
    bool is_match;

    is_match = false;
    p_fp = fopen(p_filepath, "rb");

    if (p_fp == NULL) {
        printf("[INFO] Fail. File Open: %s\n", p_filepath);
        return;
    }

    // ファイルサイズの取得
    fseek(p_fp, 0, SEEK_END);
    file_size = ftell(p_fp);
    fseek(p_fp, 0, SEEK_SET);

    // ファイルサイズチェック
    if (file_size != BIOS_FILE_SIZE_BYTE) {
        fclose(p_fp);
        printf("\33[31m[ERROR] File Size Not 512KB (Size: %ld Byte)\n\33[0m", file_size);
        return;
    } else {
        printf("[INFO] File Size(= 512KB) OK! Size: %ld Byte\n", file_size);
    }

    fread(&s_bios_buf[0], 1, (size_t)file_size, p_fp);
    fclose(p_fp);

    // CRC32の計算
    calc_crc = _calc_crc32(&s_bios_buf[0], (size_t)file_size);
    printf("[INFO] Calc CRC32: 0x%08X\n", calc_crc);

    // CRC32テーブルの検索
    for (h = 0; h < BIOS_CRC32_TBL_CNT; h++)
    {
        p_tbl = (bios_crc32_t *) g_bios_crc32_tbl[h].p_tbl;
        tbl_cnt = g_bios_crc32_tbl[h].tbl_cnt;

        for (i = 0; i < tbl_cnt; i++)
        {
            if (p_tbl[i].crc32 == calc_crc) {
                printf("\33[32m[INFO] CRC32 Match! CRC32: 0x%08X\n\33[0m", p_tbl[i].crc32);
                printf("\33[32m[INFO] BIOS Type: %s\n\33[0m", p_tbl[i].p_bios_name);
                printf("\33[32m[INFO] PS1 Type: %s\n\33[0m", p_tbl[i].p_ps1_name);
                is_match = true;
                break;
            }
        }
    }

    if (!is_match) {
        printf("\33[32m[INFO] Not Match All PS1 BIOS CRC32\n\33[0m");
    }
}
// ---------------------------------------------------
// [メイン]
int main(int argc, char **p_argv)
{
    if (argc < 2) {
        printf("\33[31m[ERROR] Please, PS1 BIOS File Path\n\33[0m");
        return 1;
    }

    printf("------------------------------------------------------\n");
    printf("\33[34mPS1 BIOS Checker v%.01f\n\33[0m", PS1_BIOS_CHECKER_VERSION);
    printf("\33[34mDevelop by Chimipupu (https://github.com/Chimipupu/ps1_bios_checker)\n\33[0m");
    printf("------------------------------------------------------\n");

    _check_bios_file(p_argv[1]);

    printf("------------------------------------------------------\n");

    return 0;
}
// ---------------------------------------------------