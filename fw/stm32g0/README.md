# STM32G0 Firmware

## Debugging

### STM32G0

#### Board's Debug Connector

| # | Ribbon Wire |
|--:|:------------|
| 1 | 🟥 `VCC` |
| 2 | 🟧 `SWCLK` |
| 3 | 🟨 `GND` |
| 4 | 🟩 `SWDIO` |
| 5 | 🟦 `nRESET` |
| 6 | 🟪 `Test / UART Log` |

#### ST-LINK's 20-Pin Connector

| # | Ribbon Wire | Debug Header | # | # | Debug Header | Ribbon Wire | # |
|--:|:------------|-------------:|--:|:--|:-------------|------------:|:--|
| 1 | 🟥 `VCC` | `VTref` | 1 | 2 | `NC` | | |
| | | `nTRST` | 3 | 4 | `GND` | `GND` 🟨 | 3 |
| | | `TDI` | 5 | 6 | `GND` | | |
| 4 | 🟩 `SWDIO` | `SWDIO` | 7 | 8 | `GND` | | |
| 2 | 🟧 `SWCLK` | `SWCLK` | 9 | 10 | `GND` | | |
| | | `RTCK` | 11 | 12 | `GND` | | |
| | | `TDO` | 13 | 14 | `GND` | | |
| 5 | 🟦 `nRESET` | `nRESET` | 15 | 16 | `GND` | | |
| | | `NC` | 17 | 18 | `GND` | | |
| | | `NC` | 19 | 20 | `GND` | | |
