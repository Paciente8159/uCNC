"""Validate the canonical MCU pin-numbering contract using only the stdlib."""

from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[4]
MCUS = ROOT / "uCNC" / "src" / "hal" / "mcus"
README = MCUS / "README.md"
IO_HAL = ROOT / "uCNC" / "src" / "hal" / "io_hal.h"


def numbered(prefix: str, first: int, count: int) -> dict[int, str]:
    return {first + offset: f"{prefix}{offset}" for offset in range(count)}


EXPECTED = {
    **numbered("STEP", 1, 8),
    **numbered("DIR", 9, 8),
    **numbered("PWM", 25, 16),
    **numbered("SERVO", 41, 6),
    **numbered("DOUT", 47, 50),
    100: "LIMIT_X",
    101: "LIMIT_Y",
    102: "LIMIT_Z",
    103: "LIMIT_X2",
    104: "LIMIT_Y2",
    105: "LIMIT_Z2",
    106: "LIMIT_A",
    107: "LIMIT_B",
    108: "LIMIT_C",
    109: "PROBE",
    110: "ESTOP",
    111: "SAFETY_DOOR",
    112: "FHOLD",
    113: "CS_RES",
    **numbered("ANALOG", 114, 16),
    **numbered("DIN", 130, 50),
    200: "TX",
    201: "RX",
    202: "USB_DM",
    203: "USB_DP",
    204: "SPI_CLK",
    205: "SPI_SDI",
    206: "SPI_SDO",
    207: "SPI_CS",
    208: "I2C_CLK",
    209: "I2C_DATA",
    210: "TX2",
    211: "RX2",
    212: "SPI2_CLK",
    213: "SPI2_SDI",
    214: "SPI2_SDO",
    215: "SPI2_CS",
}
EXPECTED.update({17 + offset: f"STEP{offset}_EN" for offset in range(8)})

ROW = re.compile(
    r"^\|\s*(\d+)\s*\|\s*DIO(\d+)\s*\|\s*([A-Z][A-Z0-9_]*)\s*\|$",
    re.MULTILINE,
)
DEFINE = re.compile(
    r"^\s*#\s*define\s+([A-Z][A-Z0-9_]*)\s+(\d+)\s*(?://.*)?$",
    re.MULTILINE,
)
RESERVED_DIO = re.compile(r"\bDIO(2(?:1[6-9]|[2-4]\d|5[0-4]))\b")
PIN_BY_NAME = {name: pin for pin, name in EXPECTED.items()}
# Communication signals are consumed by peripheral setup rather than the
# generic IO HAL. Chip-select is the exception because callers may toggle it as
# ordinary or extended output IO.
IO_HAL_PINS = set(range(1, 97)) | set(range(100, 180)) | {207, 215}


def main() -> int:
    errors: list[str] = []
    readme_text = README.read_text(encoding="utf-8")
    rows = {
        int(pin): (int(alias), name)
        for pin, alias, name in ROW.findall(readme_text)
    }

    if set(rows) != set(EXPECTED):
        errors.append(
            "README pin IDs differ: "
            f"missing={sorted(set(EXPECTED) - set(rows))}, "
            f"extra={sorted(set(rows) - set(EXPECTED))}"
        )
    for pin, name in EXPECTED.items():
        if rows.get(pin) != (pin, name):
            errors.append(
                f"README DIO{pin}: expected {name}, found {rows.get(pin)}"
            )

    io_text = IO_HAL.read_text(encoding="utf-8")
    for pin in IO_HAL_PINS:
        if not re.search(rf"^#define io{pin}_", io_text, re.MULTILINE):
            errors.append(f"io_hal.h has no dispatch entry for DIO{pin}")

    for path in MCUS.glob("*/mcumap_*.h"):
        text = path.read_text(encoding="utf-8", errors="replace")
        for name, value_text in DEFINE.findall(text):
            if name in PIN_BY_NAME:
                value = int(value_text)
                if value != PIN_BY_NAME[name]:
                    errors.append(
                        f"{path.relative_to(ROOT)}: {name}={value}, "
                        f"expected {PIN_BY_NAME[name]}"
                    )
        match = RESERVED_DIO.search(text)
        if match:
            errors.append(f"{path.relative_to(ROOT)} uses reserved {match.group(0)}")

    if errors:
        print("MCU pin contract validation failed:", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1

    print(
        f"Validated {len(EXPECTED)} canonical pins across README, IO HAL, "
        "and mcumaps."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
