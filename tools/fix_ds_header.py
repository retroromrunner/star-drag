#!/usr/bin/env python3
"""Strip DSi enhancement from a devkitPro/calico-built .nds so it is a
pure Nintendo DS ROM.

Why: calico's ARM7 carries .twl (DSi) sections, so ndstool 2.3.1 writes a
DSi-enhanced header (unit code 0x02, DSi9/DSi7 fields, and the DSi magic
0x8D898581/0x8C888480 at 0x180/0x184). Strict emulators and DSi-mode boots
then take the DSi path; the DSi9 stub is only 12 bytes, so ARM9 never runs
and the screens stay white (DeSmuME 0.9.11 logs "ROM DSi Enhanced" /
"TWL-####"; melonDS in DSi mode with no DSi BIOS does the same). A pure-DS
header makes every loader treat the ROM as DS-only.

What it does:
  * unit code (0x12) -> 0x00
  * DSi magic at 0x180-0x187 -> zero (this is what DeSmuME checks:
    NDSSystem.cpp: _isDSiEnhanced = (readROM(0x180)==0x8D898581 &&
    readROM(0x184)==0x8C888480))
  * DSi header fields 0x1B0-0x1FF -> zero
  * header CRC16 (0x15E, over 0x000-0x15D) recomputed
The DSi9/DSi7 binary blobs remain in the file but are unreferenced once
the header fields are gone; no DS-mode loader reads them.
Usage: fix_ds_header.py in.nds [out.nds]   (in place if out omitted)
"""
import struct
import sys

DSI_MAGIC = (0x8D898581, 0x8C888480)


def crc16(data: bytes) -> int:
    # DS header CRC: CRC-16/MODBUS (poly 0xA001 reflected, init 0xFFFF),
    # over header bytes 0x000-0x15D.
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc


def main() -> int:
    src, dst = sys.argv[1], (sys.argv[2] if len(sys.argv) > 2 else sys.argv[1])
    d = bytearray(open(src, 'rb').read())
    if len(d) < 0x200:
        print('file too small to be a .nds')
        return 1
    was_dsi = (d[0x12] == 0x02 or
               struct.unpack('<I', d[0x180:0x184])[0] == DSI_MAGIC[0])
    d[0x12] = 0x00                       # unit code: DS only
    d[0x180:0x188] = b'\x00' * 8          # DSi magic
    d[0x1B0:0x200] = b'\x00' * 0x50       # DSi header fields
    # v3: game code MUST stay '####' (ndstool default) and maker 0x0000.
    # melonDS (NDS_Header.h) classifies a ROM as homebrew ONLY if
    # ARM9ROMOffset < 0x4000 OR game code == '####'. v2 stamped SDRG/01,
    # so melonDS treated it as a RETAIL cart: the RetroArch melonDS core
    # (melonds-ds >= 1.1.8) then refuses to load it without native BIOS
    # ("The loaded rom is encrypted and needs native NDS BIOS files").
    # With '####' ParseROM() builds a CartHomebrew, which needs no BIOS
    # and no secure-area decryption. The stock libnds hello-world ships
    # exactly this way (####, ARM9 offset 0x4000). Do NOT stamp a real
    # game code here again.
    d[0x0C:0x10] = b'####'
    d[0x10:0x12] = b'\x00\x00'
    d[0x15E:0x160] = struct.pack('<H', crc16(bytes(d[0x000:0x15E])))
    # v2: pad the file to the declared device capacity (byte 0x14).
    # v1 was 310272 bytes for a 512 KB capacity and melonDS logged
    # "bad ROM size ... rounded to 524288"; stricter loaders (DeSmuME
    # stream mode, RetroArch cores) may not pad for us.
    cap = d[0x14]
    want = 128 * 1024 << cap if cap < 10 else len(d)
    if len(d) < want:
        d.extend(b'\xff' * (want - len(d)))
    open(dst, 'wb').write(d)
    print(f'{dst}: was_dsi={was_dsi} -> pure DS header, '
          f'unit=0x{d[0x12]:02x}, magic=0x{struct.unpack("<I", d[0x180:0x184])[0]:08x}, '
          f'crc=0x{struct.unpack("<H", d[0x15E:0x160])[0]:04x}, '
          f'gamecode={bytes(d[0x0C:0x10]).decode()}, size={len(d)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
