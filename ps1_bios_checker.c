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
#include <windows.h>

// ---------------------------------------------------
// [ps1_bios_checker]

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
const uint8_t g_bios_crc32_tbl_cnt = sizeof(g_bios_crc32_tbl) / sizeof(g_bios_crc32_tbl[0]);

static uint8_t s_bios_buf[BIOS_BUF_SIZE_BYTE] = {0};
static uint32_t s_crc32_tbl[CRC32_TBL_SIZE] = {0};
static bool s_is_tbl_init = false;

static void _crc32_tbl_init(void);
static uint32_t _calc_crc32(const uint8_t *p_buf, size_t size);
static void _dump_bios_file(uint8_t *p_bios_buf, char *p_dump_str_buf);
static void _check_bios_file(const char *p_filepath);
static void _save_log_file(void);
// ---------------------------------------------------
// [GUI関連]

// UIハンドラ
static HWND s_hwnd_dump = NULL;
static HWND s_hwnd_btn_open = NULL;
static HWND s_hwnd_btn_check = NULL;
static HWND s_hwnd_btn_save = NULL;
static HWND s_hwnd_result = NULL;
static HWND s_hwnd_log = NULL;

// 描画用リソース
static HBRUSH s_hbr_bg = NULL;
static HBRUSH s_hbr_edit = NULL;
static HBRUSH s_hbr_success = NULL;
static HBRUSH s_hbr_fail = NULL;
static HBRUSH s_hbr_wait = NULL;
static HFONT s_hfont_ui = NULL;
static HFONT s_hfont_dump = NULL;
static HFONT s_hfont_result = NULL;

// 状態フラグ・保持データ
static bool s_is_checked = false;
static bool s_is_success = false;
static bool s_is_filepath_set = false;
static char s_filepath[MAX_PATH] = {0};

// Windows API Wrappers
static bool _win_get_open_file_name(HWND hwnd, char *p_filepath, uint32_t max_len);
static void _win_set_window_text_utf8(HWND hwnd, const char *p_text);
static void _win_append_window_text_utf8(HWND hwnd, const char *p_text);
static void _win_get_module_file_name(char *p_path, uint32_t max_len);
static void _win_get_local_time(SYSTEMTIME *p_st);
static int32_t _win_get_window_text_length(HWND hwnd);
static int32_t _win_get_window_text(HWND hwnd, wchar_t *p_text, int32_t max_count);
static int32_t _win_wide_char_to_multi_byte(const wchar_t *p_wstr, char *p_mbstr, int32_t mbstr_len);
static HBRUSH _win_create_solid_brush(uint8_t r, uint8_t g, uint8_t b);
static HFONT _win_create_font(int height, int weight, const wchar_t *p_name, bool is_mono);
static void _win_set_text_color(HDC hdc, uint8_t r, uint8_t g, uint8_t b);
static void _win_set_bk_color(HDC hdc, uint8_t r, uint8_t g, uint8_t b);
static void _win_set_bk_mode(HDC hdc, int mode);
static void _win_delete_object(HGDIOBJ obj);
static void _win_invalidate_rect(HWND hwnd);

static LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

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

// BIOSファイルをダンプ形式の文字列に変換
static void _dump_bios_file(uint8_t *p_bios_buf, char *p_dump_str_buf)
{
    uint8_t val;
    uint32_t i, j;
    char dump_line[128];
    char *p_dump = p_dump_str_buf;
    char *p_dump_line = &dump_line[0];

    for (i = 0; i < 512; i += 16)
    {
        snprintf(p_dump_line, sizeof(dump_line), "%08X: ", (uint32_t)i);
        strcat(p_dump, p_dump_line);

        for (j = 0; j < 16; j++)
        {
            if ((i + j) < 512) {
                val = p_bios_buf[i + j];
                snprintf(p_dump_line, sizeof(dump_line), "%02X ", val);
                strcat(p_dump, p_dump_line);
            } else {
                strcat(p_dump, "   ");
            }
        }

        strcat(p_dump, " | ");

        for (j = 0; j < 16; j++)
        {
            if ((i + j) < 512) {
                val = p_bios_buf[i + j];
                if (val >= 32 && val <= 126) {
                    snprintf(p_dump_line, sizeof(dump_line), "%c", val);
                } else {
                    snprintf(p_dump_line, sizeof(dump_line), ".");
                }
                strcat(p_dump, p_dump_line);
            }
        }
        strcat(p_dump, "\r\n");
    }
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
    char dump_str_buf[16384];
    char msg_buf[2048];
    char line_buf[256];
    char *p_msg;
    char *p_line;
    is_match = false;
    p_msg = &msg_buf[0];
    p_line = &line_buf[0];
    p_msg[0] = '\0';
    s_is_checked = true;
    s_is_success = false;

    _win_set_window_text_utf8(s_hwnd_result, " CHECKING... ");
    _win_invalidate_rect(s_hwnd_result);
    _win_set_window_text_utf8(s_hwnd_dump, "");

    p_fp = fopen(p_filepath, "rb");

    if (p_fp == NULL) {
        snprintf(p_line, sizeof(line_buf), "[ERROR] File Open Failed:\r\n%s\r\n", p_filepath);
        strcat(p_msg, p_line);
        _win_set_window_text_utf8(s_hwnd_log, p_msg);
        _win_set_window_text_utf8(s_hwnd_result, " FAIL (File Open Error) ");
        return;
    }

    fseek(p_fp, 0, SEEK_END);
    file_size = ftell(p_fp);
    fseek(p_fp, 0, SEEK_SET);

    if (file_size != BIOS_FILE_SIZE_BYTE) {
        fclose(p_fp);
        snprintf(p_line, sizeof(line_buf), "[ERROR] File Size Not 512KB (Size: %ld Byte)\r\n", file_size);
        strcat(p_msg, p_line);
        _win_set_window_text_utf8(s_hwnd_log, p_msg);
        _win_set_window_text_utf8(s_hwnd_result, " FAIL (Invalid Size) ");
        return;
    }

    snprintf(p_line, sizeof(line_buf), "[INFO] File Size(= 512KB) OK! Size: %ld Byte\r\n", file_size);
    strcat(p_msg, p_line);

    fread(&s_bios_buf[0], 1, (size_t)file_size, p_fp);
    fclose(p_fp);

    _dump_bios_file(&s_bios_buf[0], &dump_str_buf[0]);
    _win_set_window_text_utf8(s_hwnd_dump, &dump_str_buf[0]);

    calc_crc = _calc_crc32(&s_bios_buf[0], (size_t)file_size);
    snprintf(p_line, sizeof(line_buf), "[INFO] Calc CRC32: 0x%08X\r\n", calc_crc);
    strcat(p_msg, p_line);

    for (h = 0; h < g_bios_crc32_tbl_cnt; h++)
    {
        p_tbl = (bios_crc32_t *) g_bios_crc32_tbl[h].p_tbl;
        tbl_cnt = g_bios_crc32_tbl[h].tbl_cnt;

        for (i = 0; i < tbl_cnt; i++)
        {
            if (p_tbl[i].crc32 == calc_crc) {
                snprintf(p_line, sizeof(line_buf), "[INFO] CRC32 Match! CRC32: 0x%08X\r\n", p_tbl[i].crc32);
                strcat(p_msg, p_line);
                snprintf(p_line, sizeof(line_buf), "[INFO] BIOS Type: %s\r\n", p_tbl[i].p_bios_name);
                strcat(p_msg, p_line);
                snprintf(p_line, sizeof(line_buf), "[INFO] PS1 Type: %s\r\n", p_tbl[i].p_ps1_name);
                strcat(p_msg, p_line);
                is_match = true;
                break;
            }
        }
        if (is_match) {
            break;
        }
    }

    if (!is_match) {
        snprintf(p_line, sizeof(line_buf), "[INFO] Not Match All PS1 BIOS CRC32\r\n");
        strcat(p_msg, p_line);
        s_is_success = false;
        _win_set_window_text_utf8(s_hwnd_result, " FAIL ");
    } else {
        s_is_success = true;
        _win_set_window_text_utf8(s_hwnd_result, " SUCCESS ");
    }

    _win_set_window_text_utf8(s_hwnd_log, p_msg);
    _win_invalidate_rect(s_hwnd_result);
}

static void _save_log_file(void)
{
    char exe_path[MAX_PATH];
    char log_path[MAX_PATH];
    char append_msg[MAX_PATH + 64];
    char *p_last_slash;
    SYSTEMTIME st;
    FILE *p_fp;
    int32_t dump_len;
    int32_t log_len;
    wchar_t *p_wdump;
    wchar_t *p_wlog;
    int32_t utf8_dump_len;
    int32_t utf8_log_len;
    char *p_utf8_dump;
    char *p_utf8_log;

    if (!s_is_checked) {
        _win_append_window_text_utf8(s_hwnd_log, "[WARNING] 先にチェックを実行してください。\r\n");
        return;
    }

    _win_get_module_file_name(&exe_path[0], MAX_PATH);
    p_last_slash = strrchr(&exe_path[0], '\\');
    if (p_last_slash != NULL) {
        *(p_last_slash + 1) = '\0';
    }

    _win_get_local_time(&st);
    snprintf(&log_path[0], MAX_PATH, "%sps1_bios_checker_result_%04d%02d%02d%02d%02d.log",
            &exe_path[0], st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

    dump_len = _win_get_window_text_length(s_hwnd_dump);
    log_len = _win_get_window_text_length(s_hwnd_log);

    p_wdump = (wchar_t *)malloc((size_t)(dump_len + 1) * sizeof(wchar_t));
    p_wlog = (wchar_t *)malloc((size_t)(log_len + 1) * sizeof(wchar_t));

    if (p_wdump != NULL && p_wlog != NULL) {
        _win_get_window_text(s_hwnd_dump, p_wdump, dump_len + 1);
        _win_get_window_text(s_hwnd_log, p_wlog, log_len + 1);

        utf8_dump_len = _win_wide_char_to_multi_byte(p_wdump, NULL, 0);
        p_utf8_dump = (char *)malloc((size_t)utf8_dump_len);

        utf8_log_len = _win_wide_char_to_multi_byte(p_wlog, NULL, 0);
        p_utf8_log = (char *)malloc((size_t)utf8_log_len);

        if (p_utf8_dump != NULL && p_utf8_log != NULL) {
            _win_wide_char_to_multi_byte(p_wdump, p_utf8_dump, utf8_dump_len);
            _win_wide_char_to_multi_byte(p_wlog, p_utf8_log, utf8_log_len);

            p_fp = fopen(&log_path[0], "wb");
            if (p_fp != NULL) {
                fprintf(p_fp, "--- Memory Dump ---\r\n");
                fwrite(p_utf8_dump, 1, (size_t)(utf8_dump_len - 1), p_fp);
                fprintf(p_fp, "\r\n--- Log ---\r\n");
                fwrite(p_utf8_log, 1, (size_t)(utf8_log_len - 1), p_fp);
                fclose(p_fp);
                snprintf(&append_msg[0], sizeof(append_msg), "\r\n[INFO] ログファイルを保存しました:\r\n%s\r\n", &log_path[0]);
                _win_append_window_text_utf8(s_hwnd_log, &append_msg[0]);
            }
        }
        if (p_utf8_dump != NULL) free(p_utf8_dump);
        if (p_utf8_log != NULL) free(p_utf8_log);
    }
    if (p_wdump != NULL) free(p_wdump);
    if (p_wlog != NULL) free(p_wlog);
}

static LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
    char filepath[MAX_PATH];
    char *p_filepath;
    bool is_opened;
    LRESULT res;
    HDC h_dc;
    HWND h_wnd;

    p_filepath = &filepath[0];
    res = 0;

    switch (u_msg) {
        case WM_COMMAND:
            if (LOWORD(w_param) == 2) {
                is_opened = _win_get_open_file_name(hwnd, p_filepath, MAX_PATH);
                if (is_opened) {
                    strncpy(s_filepath, p_filepath, sizeof(s_filepath));
                    s_filepath[sizeof(s_filepath) - 1] = '\0'; // 必ず末尾をヌル終端
                    s_is_filepath_set = true;
                    _win_set_window_text_utf8(s_hwnd_log, "[INFO] BIOSファイルが選択されました。\r\n「チェック実行」ボタンを押して解析を開始してください。\r\n");
                    s_is_checked = false;
                    _win_set_window_text_utf8(s_hwnd_result, "");
                    _win_invalidate_rect(s_hwnd_result);
                }
            } else if (LOWORD(w_param) == 5) {
                if (s_is_filepath_set) {
                    _check_bios_file(s_filepath);
                } else {
                    _win_set_window_text_utf8(s_hwnd_log, "[WARNING] 先に「BIOSファイルを開く」からファイルを選択してください。\r\n");
                }
            } else if (LOWORD(w_param) == 6) {
                _save_log_file();
            }
            break;

        case WM_CTLCOLORSTATIC:
            h_dc = (HDC)w_param;
            h_wnd = (HWND)l_param;

            if (h_wnd == s_hwnd_result) {
                _win_set_bk_mode(h_dc, OPAQUE);
                _win_set_text_color(h_dc, 255, 255, 255);

                if (s_is_checked) {
                    if (s_is_success) {
                        _win_set_bk_color(h_dc, 76, 175, 80);
                        res = (LRESULT)s_hbr_success;
                    } else {
                        _win_set_bk_color(h_dc, 224, 108, 117);
                        res = (LRESULT)s_hbr_fail;
                    }
                } else {
                    _win_set_bk_color(h_dc, 0, 0, 0);
                    res = (LRESULT)s_hbr_wait;
                }
                return res;
            } else if (h_wnd == s_hwnd_log || h_wnd == s_hwnd_dump) {
                _win_set_bk_mode(h_dc, OPAQUE);
                _win_set_bk_color(h_dc, 33, 37, 43);
                _win_set_text_color(h_dc, 171, 178, 191);
                res = (LRESULT)s_hbr_edit;
                return res;
            }

            _win_set_bk_mode(h_dc, TRANSPARENT);
            _win_set_text_color(h_dc, 171, 178, 191);
            res = (LRESULT)s_hbr_bg;
            return res;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            res = DefWindowProcW(hwnd, u_msg, w_param, l_param);
            break;
    }

    return res;
}

// ---------------------------------------------------
// [GUI関連 Static関数]

static bool _win_get_open_file_name(HWND hwnd, char *p_filepath, uint32_t max_len)
{
    OPENFILENAMEA ofn;
    bool is_success;

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = p_filepath;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = max_len;
    ofn.lpstrFilter = "BIOS Files\0*.bin;*.rom\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    is_success = (GetOpenFileNameA(&ofn) != 0);
    return is_success;
}

static void _win_set_window_text_utf8(HWND hwnd, const char *p_text)
{
    int32_t w_len;
    wchar_t *p_wtext;

    w_len = (int32_t)MultiByteToWideChar(CP_UTF8, 0, p_text, -1, NULL, 0);
    if (w_len > 0) {
        p_wtext = (wchar_t *)malloc((size_t)w_len * sizeof(wchar_t));
        if (p_wtext != NULL) {
            MultiByteToWideChar(CP_UTF8, 0, p_text, -1, p_wtext, w_len);
            SetWindowTextW(hwnd, p_wtext);
            free(p_wtext);
        }
    }
}

static void _win_append_window_text_utf8(HWND hwnd, const char *p_text)
{
    int32_t len;
    int32_t w_len;
    wchar_t *p_wtext;

    len = _win_get_window_text_length(hwnd);
    SendMessageW(hwnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);

    w_len = (int32_t)MultiByteToWideChar(CP_UTF8, 0, p_text, -1, NULL, 0);
    if (w_len > 0) {
        p_wtext = (wchar_t *)malloc((size_t)w_len * sizeof(wchar_t));
        if (p_wtext != NULL) {
            MultiByteToWideChar(CP_UTF8, 0, p_text, -1, p_wtext, w_len);
            SendMessageW(hwnd, EM_REPLACESEL, 0, (LPARAM)p_wtext);
            free(p_wtext);
        }
    }
}

static void _win_get_module_file_name(char *p_path, uint32_t max_len)
{
    GetModuleFileNameA(NULL, p_path, (DWORD)max_len);
}

static void _win_get_local_time(SYSTEMTIME *p_st)
{
    GetLocalTime(p_st);
}

static int32_t _win_get_window_text_length(HWND hwnd)
{
    return (int32_t)GetWindowTextLengthW(hwnd);
}

static int32_t _win_get_window_text(HWND hwnd, wchar_t *p_text, int32_t max_count)
{
    return (int32_t)GetWindowTextW(hwnd, p_text, max_count);
}

static int32_t _win_wide_char_to_multi_byte(const wchar_t *p_wstr, char *p_mbstr, int32_t mbstr_len)
{
    return (int32_t)WideCharToMultiByte(CP_UTF8, 0, p_wstr, -1, p_mbstr, mbstr_len, NULL, NULL);
}

static HBRUSH _win_create_solid_brush(uint8_t r, uint8_t g, uint8_t b)
{
    return CreateSolidBrush(RGB(r, g, b));
}

static HFONT _win_create_font(int height, int weight, const wchar_t *p_name, bool is_mono)
{
    DWORD pitch;
    pitch = is_mono ? (FIXED_PITCH | FF_MODERN) : (DEFAULT_PITCH | FF_DONTCARE);
    return CreateFontW( height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, pitch, p_name );
}

static void _win_set_text_color(HDC hdc, uint8_t r, uint8_t g, uint8_t b)
{
    SetTextColor(hdc, RGB(r, g, b));
}

static void _win_set_bk_color(HDC hdc, uint8_t r, uint8_t g, uint8_t b)
{
    SetBkColor(hdc, RGB(r, g, b));
}

static void _win_set_bk_mode(HDC hdc, int mode)
{
    SetBkMode(hdc, mode);
}

static void _win_delete_object(HGDIOBJ obj)
{
    if (obj != NULL) {
        DeleteObject(obj);
    }
}

static void _win_invalidate_rect(HWND hwnd)
{
    InvalidateRect(hwnd, NULL, TRUE);
}

// ---------------------------------------------------
// [メイン関数]

// GUIメイン for Windows
int WINAPI WinMain(HINSTANCE h_inst, HINSTANCE h_prev_inst, LPSTR p_cmd_line, int n_cmd_show)
{
    WNDCLASSEXW wc;
    MSG msg;
    HWND hwnd_main;

    (void)h_prev_inst;
    (void)p_cmd_line;

    s_hbr_bg      = _win_create_solid_brush(0, 0, 0);
    s_hbr_edit    = _win_create_solid_brush(33, 37, 43);
    s_hbr_success = _win_create_solid_brush(76, 175, 80);
    s_hbr_fail    = _win_create_solid_brush(224, 108, 117);
    s_hbr_wait    = _win_create_solid_brush(0, 0, 0);
    s_hfont_ui     = _win_create_font(16, FW_NORMAL, L"Segoe UI", false);
    s_hfont_dump   = _win_create_font(14, FW_NORMAL, L"Consolas", true);
    s_hfont_result = _win_create_font(24, FW_BOLD, L"Segoe UI", false); 

    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = _wnd_proc;
    wc.hInstance = h_inst;
    wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = s_hbr_bg;
    wc.lpszClassName = L"PS1BiosCheckerClass";

    RegisterClassExW(&wc);

    hwnd_main = CreateWindowExW(0, L"PS1BiosCheckerClass", L"PS1 BIOS Checker v0.1",
                                WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, 720, 640,
                                NULL, NULL, h_inst, NULL);

    s_hwnd_dump = CreateWindowExW(  WS_EX_CLIENTEDGE, L"EDIT", L"",
                                    WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                    10, 10, 680, 330,
                                    hwnd_main, (HMENU)1, h_inst, NULL  );

    // 3等分したボタン配置
    s_hwnd_btn_open = CreateWindowExW(  0, L"BUTTON", L"",
                                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                        10, 350, 220, 40,
                                        hwnd_main, (HMENU)2, h_inst, NULL);
    _win_set_window_text_utf8(s_hwnd_btn_open, "BIOSファイルを開く");

    s_hwnd_btn_check = CreateWindowExW( 0, L"BUTTON", L"",
                                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                        240, 350, 220, 40,
                                        hwnd_main, (HMENU)5, h_inst, NULL);
    _win_set_window_text_utf8(s_hwnd_btn_check, "チェック実行");

    s_hwnd_btn_save = CreateWindowExW(  0, L"BUTTON", L"",
                                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                        470, 350, 220, 40,
                                        hwnd_main, (HMENU)6, h_inst, NULL);
    _win_set_window_text_utf8(s_hwnd_btn_save, "ログ保存");

    s_hwnd_result = CreateWindowExW(0, L"STATIC", L"",
                                    WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE,
                                    10, 400, 680, 40,
                                    hwnd_main, (HMENU)3, h_inst, NULL);

    s_hwnd_log = CreateWindowExW(   WS_EX_CLIENTEDGE, L"EDIT", L"",
                                    WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                    10, 450, 680, 130,
                                    hwnd_main, (HMENU)4, h_inst, NULL);
    _win_set_window_text_utf8(s_hwnd_log, "[INFO] アプリケーションが起動しました。\r\n");

    SendMessageW(s_hwnd_btn_open, WM_SETFONT, (WPARAM)s_hfont_ui, TRUE);
    SendMessageW(s_hwnd_btn_check, WM_SETFONT, (WPARAM)s_hfont_ui, TRUE);
    SendMessageW(s_hwnd_btn_save, WM_SETFONT, (WPARAM)s_hfont_ui, TRUE);
    SendMessageW(s_hwnd_result, WM_SETFONT, (WPARAM)s_hfont_result, TRUE);
    SendMessageW(s_hwnd_log, WM_SETFONT, (WPARAM)s_hfont_ui, TRUE);
    SendMessageW(s_hwnd_dump, WM_SETFONT, (WPARAM)s_hfont_dump, TRUE);

    ShowWindow(hwnd_main, n_cmd_show);
    UpdateWindow(hwnd_main);

    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    _win_delete_object(s_hfont_ui);
    _win_delete_object(s_hfont_dump);
    _win_delete_object(s_hfont_result);
    _win_delete_object(s_hbr_bg);
    _win_delete_object(s_hbr_edit);
    _win_delete_object(s_hbr_success);
    _win_delete_object(s_hbr_fail);
    _win_delete_object(s_hbr_wait);

    return (int)msg.wParam;
}