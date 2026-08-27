# PS1 BIOSチェッカー

![gui_picture](/ps1_bios_checker_gui.png)


## ビルド方法

`gcc`でビルド

```shell
gcc ps1_bios_checker.c -o ps1_bios_checker.exe -mwindows -lcomdlg32
```

## チェック対象のBIOS

### 日本版 (NTSC-J)
| 型番 | BIOS名 | BIOS CRC32 |
| --- | --- | --- |
| SCPH-1000 | ps-10j | 0x3B601FC8 |
| SCPH-3000 | ps-11j | 0x3539DEF6 |
| SCPH-3500 | ps-21j | 0xBC190209 |
| SCPH-5000 | ps-22j | 0x24FC7E17 |
| SCPH-5500 | ps-30j | 0xFF3EEB8C |
| SCPH-7000/7500/9000 | ps-40j | 0xEC541CD0 |
| SCPH-9000 | ps-43j | 0x1E68C234 |
| SCPH-100 (PS One) | psone-43j | 0xF2AF798B |

### 北米版 (NTSC-U/C)
| 型番 | BIOS名 | BIOS CRC32 |
| --- | --- | --- |
| SCPH-1001 | ps-20a | 0x55847D8C |
| SCPH-1001 / DTL-H1101 | ps-21a | 0xAFF00F2F |
| SCPH-1001 / DTL-H1201/3001 | ps-22a | 0x37157331 |
| SCPH-5501/5503/7003 | ps-30a | 0x8D8CB7E4 |
| SCPH-7001/7501/9001 | ps-41a | 0x502224B6 |
| SCPH-101 (PS One) | psone-45a | 0x171BDCEC |

### 欧州版 (PAL)
| 型番 | BIOS名 | BIOS CRC32 |
| --- | --- | --- |
| SCPH-1002 | ps-20e | 0x98AFA9DB |
| SCPH-1002 / DTL-H1102 | ps-21e | 0x86C30531 |
| SCPH-1002 / DTL-H1202/3002 | ps-22e | 0x1A25F706 |
| SCPH-5502/5552 | ps-30e | 0xD786F0B9 |
| SCPH-7002/7502/9002 | ps-41e | 0x318178BF |
| SCPH-102 (PS One) | psone-44e | 0xE0DFB769 |
| SCPH-102 (PS One) RevB | psone-45e | 0x5E6B56F5 |

### その他
| 型番 | BIOS名 | BIOS CRC32 |
| --- | --- | --- |
| SCPH-5903 (Video CD) | ps-30j_vcd | 0x32736F17 |
| DTL-H1000H/DTL-H1100 | ps-11j_dbg | 0x4CE8FEE9 |
| DTL-H1200 / DTL-H3000 | ps-22j_dbg | 0x5C464BE0 |