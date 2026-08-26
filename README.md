# PS1 BIOSチェッカー v0.1

## 使い方

### ビルド方法

`gcc`でビルド

```shell
gcc -o ps1_bios_checker ps1_bios_checker.c 
```

### 実行方法

例) PS1のBIOS名が `PS1_BIOS.bin`の場合

```shell
./ps1_bios_checker PS1_BIOS.bin
```
### 実行例

```shell
------------------------------------------------------
PS1 BIOS Checker v0.1
Develop by chimipupu (https://github.com/Chimipupu/ps1_bios_checker)
------------------------------------------------------
[INFO] File Size(= 512KB) OK! Size: 524288 Byte
[INFO] Calc CRC32: 0xEC541CD0
[INFO] CRC32 Match! CRC32: 0xEC541CD0
[INFO] BIOS Type: ps-40j
[INFO] PS1 Type: SCPH-7000
------------------------------------------------------
```

## チェック対象のBIOS

| 型番 | BIOS名 | BIOS CRC32 |
| --- | --- | --- |
| SCPH-1000 | ps-10j | 0x3B601FC8 |
| SCPH-3000 | ps-11j | 0x3539DEF6 |
| SCPH-3500 | ps-21j | 0xBC190209 |
| SCPH-5500 | ps-30j | 0xFF3EEB8C |
| SCPH-7000 | ps-40j | 0xEC541CD0 |
| SCPH-100 (PS one) | psone-43j | 0xF2AF798B |
