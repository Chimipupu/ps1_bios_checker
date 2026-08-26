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
#define CRC32_TBL_SIZE    256

// NOTE: PS1のBIOSは通常512KB (524288 bytes)
#define BIOS_FILE_SIZE_BYTE    (512 * 1024)

typedef struct {
    uint32_t crc32;             // CRC32 期待値
    const char *p_ps1_name;     // PS1の型番名
    const char *p_bios_name;    // BIOS名
} bios_crc32_t;

// BIOSのCRC32、PS1の型番名、BIOS名のテーブル
const bios_crc32_t g_bios_crc32_tbl[] = {
    { 0x3B601FC8, "SCPH-1000", "ps-10j"    },
    { 0x3539DEF6, "SCPH-3000", "ps-11j"    },
    { 0xBC190209, "SCPH-3500", "ps-21j"    },
    { 0xFF3EEB8C, "SCPH-5500", "ps-30j"    },
    { 0xEC541CD0, "SCPH-7000", "ps-40j"    },
    { 0xF2AF798B, "SCPH-100",  "psone-43j" }
};
const uint8_t BIOS_CRC32_TBL_CNT = sizeof(g_bios_crc32_tbl) / sizeof(g_bios_crc32_tbl[0]);

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
    uint8_t *p_buf;
    long file_size;
    size_t read_size;
    uint32_t calc_crc;
    size_t i;
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
        printf("\33[33m[WARN] File Size Not 512KB (Size: %ld Byte)\n\33[0m", file_size);
    } else {
        printf("[INFO] File Size(= 512KB) OK! Size: %ld Byte\n", file_size);
    }

    p_buf = (uint8_t *)malloc((size_t)file_size);
    if (p_buf == NULL) {
        printf("\33[31m[ERROR] Memory Malloc FAIL\n\33[0m");
        fclose(p_fp);
        return;
    }

    read_size = fread(p_buf, 1, (size_t)file_size, p_fp);
    if (read_size != (size_t)file_size) {
        printf("\33[31m[ERROR] BIOS File Read FAIL\n\33[0m");
        free(p_buf);
        fclose(p_fp);
        return;
    }
    fclose(p_fp);

    // CRC32の計算
    calc_crc = _calc_crc32(p_buf, (size_t)file_size);
    free(p_buf);
    printf("[INFO] Calc CRC32: 0x%08X\n", calc_crc);

    // CRC32テーブルの検索
    for (i = 0; i < BIOS_CRC32_TBL_CNT; i++)
    {
        if (g_bios_crc32_tbl[i].crc32 == calc_crc) {
            printf("\33[32m[INFO] CRC32 Match! CRC32: 0x%08X\n\33[0m", g_bios_crc32_tbl[i].crc32);
            printf("\33[32m[INFO] BIOS Type: %s\n\33[0m", g_bios_crc32_tbl[i].p_bios_name);
            printf("\33[32m[INFO] PS1 Type: %s\n\33[0m", g_bios_crc32_tbl[i].p_ps1_name);
            is_match = true;
            break;
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