// Copyright (c) 2024, Auterion AG
// SPDX-License-Identifier: BSD-3-Clause

#include "device.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <libelf.h>
#include <gelf.h>
#include <vector>

using namespace std::literals;

static const Device::IrqTable irq_names_stm32f765 =
{
    {16+-14, "NonMaskableInt"},     // 2 Non Maskable Interrupt
    {16+-13, "HardFault"},          // 3 Cortex-M Hard Fault Interrupt
    {16+-12, "MemoryManagement"},   // 4 Cortex-M Memory Management Interrupt
    {16+-11, "BusFault"},           // 5 Cortex-M Bus Fault Interrupt
    {16+-10, "UsageFault"},         // 6 Cortex-M Usage Fault Interrupt
    {16+ -5, "SVCall"},             // 11 Cortex-M SV Call Interrupt
    {16+ -4, "DebugMonitor"},       // 12 Cortex-M Debug Monitor Interrupt
    {16+ -2, "PendSV"},             // 14 Cortex-M Pend SV Interrupt
    {16+ -1, "SysTick"},            // 15 Cortex-M System Tick Interrupt

    {16+  0, "WWDG"},               // Window WatchDog Interrupt
    {16+  1, "PVD"},                // PVD through EXTI Line detection Interrupt
    {16+  2, "TAMP_STAMP"},         // Tamper and TimeStamp interrupts through the EXTI line
    {16+  3, "RTC_WKUP"},           // RTC Wakeup interrupt through the EXTI line
    {16+  4, "FLASH"},              // FLASH global Interrupt
    {16+  5, "RCC"},                // RCC global Interrupt
    {16+  6, "EXTI0"},              // EXTI Line0 Interrupt
    {16+  7, "EXTI1"},              // EXTI Line1 Interrupt
    {16+  8, "EXTI2"},              // EXTI Line2 Interrupt
    {16+  9, "EXTI3"},              // EXTI Line3 Interrupt
    {16+ 10, "EXTI4"},              // EXTI Line4 Interrupt
    {16+ 11, "DMA1_Stream0"},       // DMA1 Stream 0 global Interrupt
    {16+ 12, "DMA1_Stream1"},       // DMA1 Stream 1 global Interrupt
    {16+ 13, "DMA1_Stream2"},       // DMA1 Stream 2 global Interrupt
    {16+ 14, "DMA1_Stream3"},       // DMA1 Stream 3 global Interrupt
    {16+ 15, "DMA1_Stream4"},       // DMA1 Stream 4 global Interrupt
    {16+ 16, "DMA1_Stream5"},       // DMA1 Stream 5 global Interrupt
    {16+ 17, "DMA1_Stream6"},       // DMA1 Stream 6 global Interrupt
    {16+ 18, "ADC"},                // ADC1, ADC2 and ADC3 global Interrupts
    {16+ 19, "CAN1_TX"},            // CAN1 TX Interrupt
    {16+ 20, "CAN1_RX0"},           // CAN1 RX0 Interrupt
    {16+ 21, "CAN1_RX1"},           // CAN1 RX1 Interrupt
    {16+ 22, "CAN1_SCE"},           // CAN1 SCE Interrupt
    {16+ 23, "EXTI9_5"},            // External Line[9:5] Interrupts
    {16+ 24, "TIM1_BRK_TIM9"},      // TIM1 Break interrupt and TIM9 global interrupt
    {16+ 25, "TIM1_UP_TIM10"},      // TIM1 Update Interrupt and TIM10 global interrupt
    {16+ 26, "TIM1_TRG_COM_TIM11"}, // TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt
    {16+ 27, "TIM1_CC"},            // TIM1 Capture Compare Interrupt
    {16+ 28, "TIM2"},               // TIM2 global Interrupt
    {16+ 29, "TIM3"},               // TIM3 global Interrupt
    {16+ 30, "TIM4"},               // TIM4 global Interrupt
    {16+ 31, "I2C1_EV"},            // I2C1 Event Interrupt
    {16+ 32, "I2C1_ER"},            // I2C1 Error Interrupt
    {16+ 33, "I2C2_EV"},            // I2C2 Event Interrupt
    {16+ 34, "I2C2_ER"},            // I2C2 Error Interrupt
    {16+ 35, "SPI1"},               // SPI1 global Interrupt
    {16+ 36, "SPI2"},               // SPI2 global Interrupt
    {16+ 37, "USART1"},             // USART1 global Interrupt
    {16+ 38, "USART2"},             // USART2 global Interrupt
    {16+ 39, "USART3"},             // USART3 global Interrupt
    {16+ 40, "EXTI15_10"},          // External Line[15:10] Interrupts
    {16+ 41, "RTC_Alarm"},          // RTC Alarm (A and B) through EXTI Line Interrupt
    {16+ 42, "OTG_FS_WKUP"},        // USB OTG FS Wakeup through EXTI line interrupt
    {16+ 43, "TIM8_BRK_TIM12"},     // TIM8 Break Interrupt and TIM12 global interrupt
    {16+ 44, "TIM8_UP_TIM13"},      // TIM8 Update Interrupt and TIM13 global interrupt
    {16+ 45, "TIM8_TRG_COM_TIM14"}, // TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt
    {16+ 46, "TIM8_CC"},            // TIM8 Capture Compare Interrupt
    {16+ 47, "DMA1_Stream7"},       // DMA1 Stream7 Interrupt
    {16+ 48, "FMC"},                // FMC global Interrupt
    {16+ 49, "SDMMC1"},             // SDMMC1 global Interrupt
    {16+ 50, "TIM5"},               // TIM5 global Interrupt
    {16+ 51, "SPI3"},               // SPI3 global Interrupt
    {16+ 52, "UART4"},              // UART4 global Interrupt
    {16+ 53, "UART5"},              // UART5 global Interrupt
    {16+ 54, "TIM6_DAC"},           // TIM6 global and DAC1&2 underrun error  interrupts
    {16+ 55, "TIM7"},               // TIM7 global interrupt
    {16+ 56, "DMA2_Stream0"},       // DMA2 Stream 0 global Interrupt
    {16+ 57, "DMA2_Stream1"},       // DMA2 Stream 1 global Interrupt
    {16+ 58, "DMA2_Stream2"},       // DMA2 Stream 2 global Interrupt
    {16+ 59, "DMA2_Stream3"},       // DMA2 Stream 3 global Interrupt
    {16+ 60, "DMA2_Stream4"},       // DMA2 Stream 4 global Interrupt
    {16+ 61, "ETH"},                // Ethernet global Interrupt
    {16+ 62, "ETH_WKUP"},           // Ethernet Wakeup through EXTI line Interrupt
    {16+ 63, "CAN2_TX"},            // CAN2 TX Interrupt
    {16+ 64, "CAN2_RX0"},           // CAN2 RX0 Interrupt
    {16+ 65, "CAN2_RX1"},           // CAN2 RX1 Interrupt
    {16+ 66, "CAN2_SCE"},           // CAN2 SCE Interrupt
    {16+ 67, "OTG_FS"},             // USB OTG FS global Interrupt
    {16+ 68, "DMA2_Stream5"},       // DMA2 Stream 5 global interrupt
    {16+ 69, "DMA2_Stream6"},       // DMA2 Stream 6 global interrupt
    {16+ 70, "DMA2_Stream7"},       // DMA2 Stream 7 global interrupt
    {16+ 71, "USART6"},             // USART6 global interrupt
    {16+ 72, "I2C3_EV"},            // I2C3 event interrupt
    {16+ 73, "I2C3_ER"},            // I2C3 error interrupt
    {16+ 74, "OTG_HS_EP1_OUT"},     // USB OTG HS End Point 1 Out global interrupt
    {16+ 75, "OTG_HS_EP1_IN"},      // USB OTG HS End Point 1 In global interrupt
    {16+ 76, "OTG_HS_WKUP"},        // USB OTG HS Wakeup through EXTI interrupt
    {16+ 77, "OTG_HS"},             // USB OTG HS global interrupt
    {16+ 78, "DCMI"},               // DCMI global interrupt
    {16+ 80, "RNG"},                // RNG global interrupt
    {16+ 81, "FPU"},                // FPU global interrupt
    {16+ 82, "UART7"},              // UART7 global interrupt
    {16+ 83, "UART8"},              // UART8 global interrupt
    {16+ 84, "SPI4"},               // SPI4 global Interrupt
    {16+ 85, "SPI5"},               // SPI5 global Interrupt
    {16+ 86, "SPI6"},               // SPI6 global Interrupt
    {16+ 87, "SAI1"},               // SAI1 global Interrupt
    {16+ 90, "DMA2D"},              // DMA2D global Interrupt
    {16+ 91, "SAI2"},               // SAI2 global Interrupt
    {16+ 92, "QUADSPI"},            // Quad SPI global interrupt
    {16+ 93, "LPTIM1"},             // LP TIM1 interrupt
    {16+ 94, "CEC"},                // HDMI-CEC global Interrupt
    {16+ 95, "I2C4_EV"},            // I2C4 Event Interrupt
    {16+ 96, "I2C4_ER"},            // I2C4 Error Interrupt
    {16+ 97, "SPDIF_RX"},           // SPDIF-RX global Interrupt
    {16+ 99, "DFSDM1_FLT0"},        // DFSDM1 Filter 0 global Interrupt
    {16+100, "DFSDM1_FLT1"},        // DFSDM1 Filter 1 global Interrupt
    {16+101, "DFSDM1_FLT2"},        // DFSDM1 Filter 2 global Interrupt
    {16+102, "DFSDM1_FLT3"},        // DFSDM1 Filter 3 global Interrupt
    {16+103, "SDMMC2"},             // SDMMC2 global Interrupt
    {16+104, "CAN3_TX"},            // CAN3 TX Interrupt
    {16+105, "CAN3_RX0"},           // CAN3 RX0 Interrupt
    {16+106, "CAN3_RX1"},           // CAN3 RX1 Interrupt
    {16+107, "CAN3_SCE"},           // CAN3 SCE Interrupt
    {16+109, "MDIOS"},              // MDIO Slave global Interrupt
};


static const Device::IrqTable irq_names_stm32h753 =
{
    {16+-14, "NonMaskableInt"},     // 2 Non Maskable Interrupt
    {16+-13, "HardFault"},          // 3 Cortex-M Hard Fault Interrupt
    {16+-12, "MemoryManagement"},   // 4 Cortex-M Memory Management Interrupt
    {16+-11, "BusFault"},           // 5 Cortex-M Bus Fault Interrupt
    {16+-10, "UsageFault"},         // 6 Cortex-M Usage Fault Interrupt
    {16+ -5, "SVCall"},             // 11 Cortex-M SV Call Interrupt
    {16+ -4, "DebugMonitor"},       // 12 Cortex-M Debug Monitor Interrupt
    {16+ -2, "PendSV"},             // 14 Cortex-M Pend SV Interrupt
    {16+ -1, "SysTick"},            // 15 Cortex-M System Tick Interrupt

    {16+  0, "WWDG"},               // Window WatchDog Interrupt ( wwdg1_it, wwdg2_it)
    {16+  1, "PVD_AVD"},            // PVD/AVD through EXTI Line detection Interrupt
    {16+  2, "TAMP_STAMP"},         // Tamper and TimeStamp interrupts through the EXTI line
    {16+  3, "RTC_WKUP"},           // RTC Wakeup interrupt through the EXTI line
    {16+  4, "FLASH"},              // FLASH global Interrupt
    {16+  5, "RCC"},                // RCC global Interrupt
    {16+  6, "EXTI0"},              // EXTI Line0 Interrupt
    {16+  7, "EXTI1"},              // EXTI Line1 Interrupt
    {16+  8, "EXTI2"},              // EXTI Line2 Interrupt
    {16+  9, "EXTI3"},              // EXTI Line3 Interrupt
    {16+ 10, "EXTI4"},              // EXTI Line4 Interrupt
    {16+ 11, "DMA1_Stream0"},       // DMA1 Stream 0 global Interrupt
    {16+ 12, "DMA1_Stream1"},       // DMA1 Stream 1 global Interrupt
    {16+ 13, "DMA1_Stream2"},       // DMA1 Stream 2 global Interrupt
    {16+ 14, "DMA1_Stream3"},       // DMA1 Stream 3 global Interrupt
    {16+ 15, "DMA1_Stream4"},       // DMA1 Stream 4 global Interrupt
    {16+ 16, "DMA1_Stream5"},       // DMA1 Stream 5 global Interrupt
    {16+ 17, "DMA1_Stream6"},       // DMA1 Stream 6 global Interrupt
    {16+ 18, "ADC"},                // ADC1 and  ADC2 global Interrupts
    {16+ 19, "FDCAN1_IT0"},         // FDCAN1 Interrupt line 0
    {16+ 20, "FDCAN2_IT0"},         // FDCAN2 Interrupt line 0
    {16+ 21, "FDCAN1_IT1"},         // FDCAN1 Interrupt line 1
    {16+ 22, "FDCAN2_IT1"},         // FDCAN2 Interrupt line 1
    {16+ 23, "EXTI9_5"},            // External Line[9:5] Interrupts
    {16+ 24, "TIM1_BRK"},           // TIM1 Break Interrupt
    {16+ 25, "TIM1_UP"},            // TIM1 Update Interrupt
    {16+ 26, "TIM1_TRG_COM"},       // TIM1 Trigger and Commutation Interrupt
    {16+ 27, "TIM1_CC"},            // TIM1 Capture Compare Interrupt
    {16+ 28, "TIM2"},               // TIM2 global Interrupt
    {16+ 29, "TIM3"},               // TIM3 global Interrupt
    {16+ 30, "TIM4"},               // TIM4 global Interrupt
    {16+ 31, "I2C1_EV"},            // I2C1 Event Interrupt
    {16+ 32, "I2C1_ER"},            // I2C1 Error Interrupt
    {16+ 33, "I2C2_EV"},            // I2C2 Event Interrupt
    {16+ 34, "I2C2_ER"},            // I2C2 Error Interrupt
    {16+ 35, "SPI1"},               // SPI1 global Interrupt
    {16+ 36, "SPI2"},               // SPI2 global Interrupt
    {16+ 37, "USART1"},             // USART1 global Interrupt
    {16+ 38, "USART2"},             // USART2 global Interrupt
    {16+ 39, "USART3"},             // USART3 global Interrupt
    {16+ 40, "EXTI15_10"},          // External Line[15:10] Interrupts
    {16+ 41, "RTC_Alarm"},          // RTC Alarm (A and B) through EXTI Line Interrupt
    {16+ 43, "TIM8_BRK_TIM12"},     // TIM8 Break Interrupt and TIM12 global interrupt
    {16+ 44, "TIM8_UP_TIM13"},      // TIM8 Update Interrupt and TIM13 global interrupt
    {16+ 45, "TIM8_TRG_COM_TIM14"}, // TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt
    {16+ 46, "TIM8_CC"},            // TIM8 Capture Compare Interrupt
    {16+ 47, "DMA1_Stream7"},       // DMA1 Stream7 Interrupt
    {16+ 48, "FMC"},                // FMC global Interrupt
    {16+ 49, "SDMMC1"},             // SDMMC1 global Interrupt
    {16+ 50, "TIM5"},               // TIM5 global Interrupt
    {16+ 51, "SPI3"},               // SPI3 global Interrupt
    {16+ 52, "UART4"},              // UART4 global Interrupt
    {16+ 53, "UART5"},              // UART5 global Interrupt
    {16+ 54, "TIM6_DAC"},           // TIM6 global and DAC1&2 underrun error  interrupts
    {16+ 55, "TIM7"},               // TIM7 global interrupt
    {16+ 56, "DMA2_Stream0"},       //   DMA2 Stream 0 global Interrupt
    {16+ 57, "DMA2_Stream1"},       //   DMA2 Stream 1 global Interrupt
    {16+ 58, "DMA2_Stream2"},       //   DMA2 Stream 2 global Interrupt
    {16+ 59, "DMA2_Stream3"},       //   DMA2 Stream 3 global Interrupt
    {16+ 60, "DMA2_Stream4"},       //   DMA2 Stream 4 global Interrupt
    {16+ 61, "ETH"},                // Ethernet global Interrupt
    {16+ 62, "ETH_WKUP"},           // Ethernet Wakeup through EXTI line Interrupt
    {16+ 63, "FDCAN_CAL"},          // FDCAN Calibration unit Interrupt
    {16+ 68, "DMA2_Stream5"},       // DMA2 Stream 5 global interrupt
    {16+ 69, "DMA2_Stream6"},       // DMA2 Stream 6 global interrupt
    {16+ 70, "DMA2_Stream7"},       // DMA2 Stream 7 global interrupt
    {16+ 71, "USART6"},             // USART6 global interrupt
    {16+ 72, "I2C3_EV"},            // I2C3 event interrupt
    {16+ 73, "I2C3_ER"},            // I2C3 error interrupt
    {16+ 74, "OTG_HS_EP1_OUT"},     // USB OTG HS End Point 1 Out global interrupt
    {16+ 75, "OTG_HS_EP1_IN"},      // USB OTG HS End Point 1 In global interrupt
    {16+ 76, "OTG_HS_WKUP"},        // USB OTG HS Wakeup through EXTI interrupt
    {16+ 77, "OTG_HS"},             // USB OTG HS global interrupt
    {16+ 78, "DCMI"},               // DCMI global interrupt
    {16+ 79, "CRYP"},               // CRYP crypto global interrupt
    {16+ 80, "HASH_RNG"},           // HASH and RNG global interrupt
    {16+ 81, "FPU"},                // FPU global interrupt
    {16+ 82, "UART7"},              // UART7 global interrupt
    {16+ 83, "UART8"},              // UART8 global interrupt
    {16+ 84, "SPI4"},               // SPI4 global Interrupt
    {16+ 85, "SPI5"},               // SPI5 global Interrupt
    {16+ 86, "SPI6"},               // SPI6 global Interrupt
    {16+ 87, "SAI1"},               // SAI1 global Interrupt
    {16+ 88, "LTDC"},               // LTDC global Interrupt
    {16+ 89, "LTDC_ER"},            // LTDC Error global Interrupt
    {16+ 90, "DMA2D"},              // DMA2D global Interrupt
    {16+ 91, "SAI2"},               // SAI2 global Interrupt
    {16+ 92, "QUADSPI"},            // Quad SPI global interrupt
    {16+ 93, "LPTIM1"},             // LP TIM1 interrupt
    {16+ 94, "CEC"},                // HDMI-CEC global Interrupt
    {16+ 95, "I2C4_EV"},            // I2C4 Event Interrupt
    {16+ 96, "I2C4_ER"},            // I2C4 Error Interrupt
    {16+ 97, "SPDIF_RX"},           // SPDIF-RX global Interrupt
    {16+ 98, "OTG_FS_EP1_OUT"},     // USB OTG HS2 global interrupt
    {16+ 99, "OTG_FS_EP1_IN"},      // USB OTG HS2 End Point 1 Out global interrupt
    {16+100, "OTG_FS_WKUP"},        // USB OTG HS2 End Point 1 In global interrupt
    {16+101, "OTG_FS"},             // USB OTG HS2 Wakeup through EXTI interrupt
    {16+102, "DMAMUX1_OVR"},        // Overrun interrupt
    {16+103, "HRTIM1_Master"},      // HRTIM Master Timer global Interrupts
    {16+104, "HRTIM1_TIMA"},        // HRTIM Timer A global Interrupt
    {16+105, "HRTIM1_TIMB"},        // HRTIM Timer B global Interrupt
    {16+106, "HRTIM1_TIMC"},        // HRTIM Timer C global Interrupt
    {16+107, "HRTIM1_TIMD"},        // HRTIM Timer D global Interrupt
    {16+108, "HRTIM1_TIME"},        // HRTIM Timer E global Interrupt
    {16+109, "HRTIM1_FLT"},         // HRTIM Fault global Interrupt
    {16+110, "DFSDM1_FLT0"},        // Filter1 Interrupt
    {16+111, "DFSDM1_FLT1"},        // Filter2 Interrupt
    {16+112, "DFSDM1_FLT2"},        // Filter3 Interrupt
    {16+113, "DFSDM1_FLT3"},        // Filter4 Interrupt
    {16+114, "SAI3"},               // SAI3 global Interrupt
    {16+115, "SWPMI1"},             // Serial Wire Interface 1 global interrupt
    {16+116, "TIM15"},              // TIM15 global Interrupt
    {16+117, "TIM16"},              // TIM16 global Interrupt
    {16+118, "TIM17"},              // TIM17 global Interrupt
    {16+119, "MDIOS_WKUP"},         // MDIOS Wakeup  Interrupt
    {16+120, "MDIOS"},              // MDIOS global Interrupt
    {16+121, "JPEG"},               // JPEG global Interrupt
    {16+122, "MDMA"},               // MDMA global Interrupt
    {16+124, "SDMMC2"},             // SDMMC2 global Interrupt
    {16+125, "HSEM1"},              // HSEM1 global Interrupt
    {16+127, "ADC3"},               // ADC3 global Interrupt
    {16+128, "DMAMUX2_OVR"},        // Overrun interrupt
    {16+129, "BDMA_Channel0"},      // BDMA Channel 0 global Interrupt
    {16+130, "BDMA_Channel1"},      // BDMA Channel 1 global Interrupt
    {16+131, "BDMA_Channel2"},      // BDMA Channel 2 global Interrupt
    {16+132, "BDMA_Channel3"},      // BDMA Channel 3 global Interrupt
    {16+133, "BDMA_Channel4"},      // BDMA Channel 4 global Interrupt
    {16+134, "BDMA_Channel5"},      // BDMA Channel 5 global Interrupt
    {16+135, "BDMA_Channel6"},      // BDMA Channel 6 global Interrupt
    {16+136, "BDMA_Channel7"},      // BDMA Channel 7 global Interrupt
    {16+137, "COMP"},               // COMP global Interrupt
    {16+138, "LPTIM2"},             // LP TIM2 global interrupt
    {16+139, "LPTIM3"},             // LP TIM3 global interrupt
    {16+140, "LPTIM4"},             // LP TIM4 global interrupt
    {16+141, "LPTIM5"},             // LP TIM5 global interrupt
    {16+142, "LPUART1"},            // LP UART1 interrupt
    {16+144, "CRS"},                // Clock Recovery Global Interrupt
    {16+145, "ECC"},                // ECC diagnostic Global Interrupt
    {16+146, "SAI4"},               // SAI4 global interrupt
    {16+149, "WAKEUP_PIN"},         // Interrupt for all 6 wake-up pins
};

static const Device::RegisterTable registers_stm32h753 =
{
    {0x40003820, "SPI2.TXDR"},
    {0x40003830, "SPI2.RXDR"},
    {0x40003c20, "SPI3.TXDR"},
    {0x40003c30, "SPI3.RXDR"},
    {0x40004824, "USART3.RDR"},
    {0x40004828, "USART3.TDR"},
    {0x40005024, "UART5.RDR"},
    {0x40005028, "UART5.TDR"},
    {0x40007824, "UART7.RDR"},
    {0x40007828, "UART7.TDR"},
    {0x40011424, "USART1.RDR"},
    {0x40011428, "USART1.TDR"},
    {0x40013020, "SPI1.TXDR"},
    {0x40013030, "SPI1.RXDR"},
};


const char *
Device::supported()
{
    return "stm32f765 (v5x), stm32h753 (v6x), stm32h743 (v6s), nuttx";
}

// ====================================================================================================
// ELF-derived IRQ table (vendor-agnostic)
// ====================================================================================================

/* Wrap a built-in static table without taking ownership of it. */
static std::shared_ptr<const Device::IrqTable>
_borrow(const Device::IrqTable &t)
{
    return std::shared_ptr<const Device::IrqTable>(&t, [](const Device::IrqTable *) {});
}

/* The Armv7-M / Armv8-M exception vector table (ARMv7-M ARM DDI0403E B1.5.3)
 * is an array of 32-bit words at the vector table base:
 *   [0] = initial SP, [1] = Reset, [2] = NMI, [3] = HardFault, ...
 * i.e. word N (N >= 1) is the handler address for exception number N, which is
 * exactly the numbering ETM/DWT report. So the ELF alone identifies every
 * exception: read the vector words out of the loaded sections, then resolve
 * each address against .symtab to get the handler name.
 *
 * This is strictly better than a vendor device database for our purpose: it
 * reports the handlers the firmware ACTUALLY installed (unused slots collapse
 * to the shared default handler and are reported as such), and it needs no
 * per-part table, so any Cortex-M ELF works. What the ELF cannot tell us is
 * the CPU clock (pass -C) and peripheral register names (DMA annotation only,
 * degrades to an empty string). */
namespace {

struct ElfSection
{
    uint64_t addr;
    std::vector<uint8_t> data;
};

struct FuncSym
{
    uint64_t lo, hi;
    std::string name;
};

/* Vector table base: the entry-point section's address is where the vectors
 * live on every Cortex-M linker script we care about, but rather than assume,
 * find the section that contains e_entry's *pointer* -- i.e. take the lowest
 * loaded section whose first word looks like a stack pointer (points into RAM)
 * and whose second word matches a known function. Falls back to e_entry's
 * containing section start. */
constexpr uint32_t THUMB_BIT = 1u;
constexpr int MAX_EXCEPTIONS = 16 + 480;   // Armv7-M: up to 496 exceptions

const std::string *
_lookup(const std::vector<FuncSym> &syms, uint64_t addr)
{
    const std::string *best = nullptr;
    uint64_t best_lo = 0;
    for (const auto &s : syms)
    {
        if (addr >= s.lo and addr <= s.hi and (not best or s.lo >= best_lo))
        {
            best = &s.name;
            best_lo = s.lo;
        }
    }
    return best;
}

bool
_readWord(const std::vector<ElfSection> &secs, uint64_t addr, uint32_t &out)
{
    for (const auto &s : secs)
    {
        if (addr >= s.addr and addr + 4 <= s.addr + s.data.size())
        {
            const uint8_t *p = s.data.data() + (addr - s.addr);
            out = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                  ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
            return true;
        }
    }
    return false;
}

} // namespace

Device
Device::fromElf(const std::string &elfPath, std::string *err)
{
    Device d;
    const auto fail = [&](const std::string &m) -> Device
    {
        if (err) *err = m;
        return Device{};
    };

    if (elf_version(EV_CURRENT) == EV_NONE)
        return fail("libelf version mismatch");

    int fd = open(elfPath.c_str(), O_RDONLY);
    if (fd < 0)
        return fail("cannot open " + elfPath);

    Elf *e = elf_begin(fd, ELF_C_READ, NULL);
    if (not e)
    {
        close(fd);
        return fail("not an ELF: " + elfPath);
    }

    GElf_Ehdr ehdr;
    if (gelf_getehdr(e, &ehdr) != &ehdr)
    {
        elf_end(e);
        close(fd);
        return fail("no ELF header");
    }

    /* Collect loadable section contents (for reading the vector words) and
     * STT_FUNC symbols (for naming the handlers). */
    std::vector<ElfSection> secs;
    std::vector<FuncSym> syms;
    Elf_Scn *scn = NULL;
    while ((scn = elf_nextscn(e, scn)) != NULL)
    {
        GElf_Shdr shdr;
        if (gelf_getshdr(scn, &shdr) != &shdr) continue;

        if (shdr.sh_type == SHT_PROGBITS and (shdr.sh_flags & SHF_ALLOC) and shdr.sh_size)
        {
            Elf_Data *data = elf_getdata(scn, NULL);
            if (data and data->d_buf)
            {
                ElfSection s;
                s.addr = shdr.sh_addr;
                const uint8_t *b = (const uint8_t *)data->d_buf;
                s.data.assign(b, b + data->d_size);
                secs.push_back(std::move(s));
            }
        }
        else if (shdr.sh_type == SHT_SYMTAB)
        {
            Elf_Data *data = elf_getdata(scn, NULL);
            int nsym = shdr.sh_entsize ? (int)(shdr.sh_size / shdr.sh_entsize) : 0;
            for (int i = 0; i < nsym; i++)
            {
                GElf_Sym sym;
                if (gelf_getsym(data, i, &sym) != &sym) continue;
                if (GELF_ST_TYPE(sym.st_info) != STT_FUNC) continue;
                const char *nm = elf_strptr(e, shdr.sh_link, sym.st_name);
                if (not nm or not nm[0]) continue;
                uint64_t lo = sym.st_value & ~(uint64_t)THUMB_BIT;
                /* zero-size symbols (common for asm handlers) still name their
                 * exact entry address */
                uint64_t hi = lo + (sym.st_size ? sym.st_size - 1 : 0);
                syms.push_back({lo, hi, std::string(nm)});
            }
        }
    }

    if (syms.empty())
    {
        elf_end(e);
        close(fd);
        return fail("ELF has no .symtab STT_FUNC symbols (stripped?)");
    }

    /* Locate the vector table: it is the section holding the word that points
     * at the reset handler, i.e. vectors[1] == e_entry. Scan section starts. */
    uint64_t vecbase = 0;
    const uint64_t entry = ehdr.e_entry & ~(uint64_t)THUMB_BIT;
    for (const auto &s : secs)
    {
        uint32_t w1;
        if (_readWord(secs, s.addr + 4, w1) and (w1 & ~THUMB_BIT) == entry)
        {
            vecbase = s.addr;
            break;
        }
    }

    if (not vecbase)
    {
        elf_end(e);
        close(fd);
        return fail("no vector table found (no section whose word[1] == e_entry "
                    "0x" + std::to_string(entry) + ")");
    }

    /* Walk the vectors. The table length is not recorded anywhere, so stop at
     * the first word that is not a plausible handler pointer (must resolve to a
     * function symbol). Slot 0 is the initial SP, so start at 1. */
    auto table = std::make_shared<IrqTable>();
    int16_t maxirq = 0;
    int resolved = 0;
    for (int n = 1; n < MAX_EXCEPTIONS; n++)
    {
        uint32_t w;
        if (not _readWord(secs, vecbase + 4ull * n, w)) break;
        if (not w or w == 0xFFFFFFFFu) continue;   // reserved/empty slot
        const std::string *nm = _lookup(syms, w & ~THUMB_BIT);
        if (not nm) break;                         // past the end of the table
        (*table)[(int16_t)n] = *nm;
        maxirq = (int16_t)n;
        resolved++;
    }

    elf_end(e);
    close(fd);

    if (resolved < 8)
        return fail("vector table at 0x" + std::to_string(vecbase) +
                    " resolved only " + std::to_string(resolved) + " handlers");

    d._irq_table = table;
    d._max_irq = maxirq;
    d._clock = 0;                 // not derivable from an ELF; pass -C
    d._register_table = nullptr;  // peripheral names are vendor data
    d._id = DeviceId::GENERIC_ELF;
    d._origin = elfPath + " vector table (" + std::to_string(resolved) +
                " handlers, max exception " + std::to_string(maxirq) + ")";
    return d;
}

// ====================================================================================================
// Built-in device tables (fallback when the ELF has no usable vector table)
// ====================================================================================================

/* Device selection is an EXPLICIT choice (--device / ORBETTO_DEVICE).
 * It used to be inferred by substring-matching the ELF *filename*, which meant
 * any ELF not named after a known target silently failed the
 * assert(device.valid()) in main() -- the build name and the target choice were
 * needlessly coupled. Match on the device name only. */
Device::Device(std::string_view name)
{
    // name.contains() is only available in C++23
    if (name.find("stm32f765") != std::string_view::npos or
        name.find("v5x") != std::string_view::npos)
    {
        _irq_table = _borrow(irq_names_stm32f765);
        // FIXME
        // _register_table = &registers_stm32f765;
        _max_irq = 16+109;
        _clock = 216'000'000;
        _id = DeviceId::SYKNODE_V5X;
        _origin = "built-in table stm32f765";
    }
    else if (name.find("stm32h753") != std::string_view::npos or
             name.find("v6x") != std::string_view::npos)
    {
        _irq_table = _borrow(irq_names_stm32h753);
        _register_table = &registers_stm32h753;
        _max_irq = 16+149;
        _clock = 480'000'000;
        _id = DeviceId::SYKNODE_V6X;
        _origin = "built-in table stm32h753";
    }
    else if (name.find("stm32h743") != std::string_view::npos or
             name.find("v6s") != std::string_view::npos)
    {
        _irq_table = _borrow(irq_names_stm32h753);
        _register_table = &registers_stm32h753;
        _max_irq = 16+149;
        _clock = 480'000'000;
        _id = DeviceId::SKYNODE_V6S;
        _origin = "built-in table stm32h743";
    }
    // this was used in a pure NuttX build as ETM test
    else if (name.find("nuttx") != std::string_view::npos)
    {
        _irq_table = _borrow(irq_names_stm32f765);
        _max_irq = 16+109;
        _clock = 48'000'000;
        _id = DeviceId::SYKNODE_V5X;
        _origin = "built-in table nuttx";
    }
}

/* Peripheral register annotation is the only genuinely vendor-specific piece
 * left. Attach it to an ELF-derived device on request so DMA slices keep their
 * source/destination names. */
void
Device::attachRegisterTable(std::string_view name)
{
    if (name.find("stm32h7") != std::string_view::npos or
        name.find("v6s") != std::string_view::npos or
        name.find("v6x") != std::string_view::npos)
    {
        _register_table = &registers_stm32h753;
    }
}
