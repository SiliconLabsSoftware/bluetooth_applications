/***************************************************************************//**
 * @file le_voltage_monitor.c
 * @brief Voltage measuring definitions
 * @version 1.0
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided \'as-is\', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/
#include "le_voltage_monitor.h"
#include "sl_bluetooth.h"
#include <stdint.h>
#include <stdbool.h>
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_letimer.h"
#include "em_ldma.h"
#include "em_iadc.h"
#include "em_prs.h"
#include "sl_simple_button_instances.h"

/***************************************************************************//**
 * @brief
 *    IADC Configuration Definitions.
 ******************************************************************************/

// Set CLK_ADC to 10kHz (this corresponds to a sample rate of 1ksps)
#define CLK_SRC_ADC_FREQ          (5000000)  // CLK_SRC_ADC; largest division is by 4
#define CLK_ADC_FREQ              (1000000)  // CLK_ADC; IADC_SCHEDx PRESCALE has 10 valid bits

// When changing GPIO port/pins above, make sure to change xBUSALLOC macro's
// accordingly.
#define IADC_INPUT_BUS            ABUSALLOC
#define IADC_INPUT_BUSALLOC       (GPIO_ABUSALLOC_AEVEN0_ADC0)

// IADC input GPIO port/pin configration
#define IADC_INPUT_POS            (iadcPosInputAvdd)
#define IADC_INPUT_NEG            (iadcNegInputGnd)

/***************************************************************************//**
 * @brief
 *    PRS Configuration Definitions.
 ******************************************************************************/
// Note CH7 is used by the BLE stack.
#define PRS_CHANNEL_LETIMER_IADC  (0)

/***************************************************************************//**
 * @brief
 *    LDMA Configuration Definitions.
 ******************************************************************************/
#define LDMA_CHANNEL              (0)

/***************************************************************************//**
 * @brief
 *    Private general globals.
 ******************************************************************************/
static uint32_t samplingBuffer[NUM_OF_SAMPLES];
static volatile bool startedSampling = false;
static bool prs_feature_enable = true;

/***************************************************************************//**
 * @brief
 *    Private LDMA globals.
 ******************************************************************************/
// Configure LDMA to trigger from IADC peripheral
static LDMA_TransferCfg_t xferCfg =
  LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_IADC0_IADC_SINGLE);

static LDMA_Descriptor_t descriptor =
  LDMA_DESCRIPTOR_LINKREL_P2M_WORD(&(IADC0->SINGLEFIFODATA),
                                   // src
                                   samplingBuffer,
                                   // dest
                                   NUM_OF_SAMPLES,
                                   // number of samples to transfer
                                   1);

/***************************************************************************//**
 * @brief
 *    Private static init functions of different peripherals.
 ******************************************************************************/
static void init_clocks(void);
static void init_letimer(void);
static void init_iadc(void);
static void init_prs(void);
static void init_ldma(void);
static uint32_t convert_to_mv(uint32_t raw);

/***************************************************************************//**
 * @brief
 *    Initialize all the clocks used for the peripherals.
 ******************************************************************************/
static void init_clocks(void)
{
  // Enable GPIO clock
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Select LETimer0 clock to run off LFXO
  // Reference: EFR32xG22 RM, Figure 8.3
  CMU_ClockSelectSet(cmuClock_EM23GRPACLK, cmuSelect_LFXO);

  // Enable LETimer0 clock
  CMU_ClockEnable(cmuClock_LETIMER0, true);

  // Enable PRS clock
  CMU_ClockEnable(cmuClock_PRS, true);

  // Configure IADC clock source for use while in EM2
  // Reference: EFR32xG22 RM, Figure 8.2
  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);  // FSRCO - 20MHz

  // Enable IADC0 clock
  CMU_ClockEnable(cmuClock_IADC0, true);

  // Enable LDMA clock
  CMU_ClockEnable(cmuClock_LDMA, true);
}

/***************************************************************************//**
 * @brief
 *    Initialize LETimer.
 ******************************************************************************/
static void init_letimer(void)
{
  LETIMER_Reset(LETIMER0);

  // Declare init struct
  LETIMER_Init_TypeDef init = LETIMER_INIT_DEFAULT;

  // Initialize letimer to run in free running mode
  // Reference: EFR32xG22 RM, Section 18.3.2
  init.repMode = letimerRepeatFree;

  // Pulse output for PRS
  if (prs_feature_enable == true) {
    init.ufoa0 = letimerUFOAPulse;
  }

  // Set frequency
  init.topValue = CMU_ClockFreqGet(cmuClock_LETIMER0) / SAMPLING_FREQ_HZ;

  // Disable letimer
  init.enable = false;
  init.debugRun = true;

  // Initialize free-running letimer
  LETIMER_Init(LETIMER0, &init);

  if (prs_feature_enable == false) {
    // Clear any previous interrupt flags
    LETIMER_IntClear(LETIMER0, _LETIMER_IF_MASK);

    // Enable underflow interrupts
    LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

    // Enable LETIMER interrupts
    NVIC_ClearPendingIRQ(LETIMER0_IRQn);
    NVIC_EnableIRQ(LETIMER0_IRQn);
  }
}

/***************************************************************************//**
 * @brief
 *    Initialize IADC.
 ******************************************************************************/
static void init_iadc(void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

  // Reset IADC to reset configuration in case it has been modified
  IADC_reset(IADC0);

  // Reference: EFR32xG22 RM, Section 24.3.3.1
  init.warmup = iadcWarmupNormal;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  // Configuration 0 is used by both scan and single conversions by default
  // Use unbuffered AVDD as reference
  initAllConfigs.configs[0].reference = iadcCfgReferenceVddx;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency for desired sample rate
  // Default oversampling (OSR) is 2x, and Conversion Time = ((4 * OSR) + 2) / fCLK_ADC
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                     CLK_ADC_FREQ,
                                                                     0,
                                                                     iadcCfgModeNormal,
                                                                     init.
                                                                     srcClkPrescale);

  if (prs_feature_enable == true) {
    // === PRS Connection Config =======
    // On every trigger, start conversion
    initSingle.triggerAction = iadcTriggerActionOnce;

    // Set conversions to trigger from letimer/PRS
    initSingle.triggerSelect = iadcTriggerSelPrs0PosEdge;
  }

  // === LDMA Connection Config ======
  // Wake up the DMA when FIFO is filled
  initSingle.fifoDmaWakeup = true;

  // Set how many elements in FIFO will generate DMA request
  initSingle.dataValidLevel = iadcFifoCfgDvl1;  // Use _IADC_SCANFIFOCFG_DVL_VALID1 for GSDK 3.0

  // === Pin Input Config ============
  // Configure Input sources for single ended conversion
  initSingleInput.posInput = IADC_INPUT_POS;
  initSingleInput.negInput = IADC_INPUT_NEG;

  // Allocate the analog bus for IADC0 input
  GPIO->IADC_INPUT_BUS |= IADC_INPUT_BUSALLOC;

  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Single
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);
}

/***************************************************************************//**
 * @brief
 *    Initialize PRS.
 ******************************************************************************/
static void init_prs(void)
{
  if (prs_feature_enable == true) {
    // Producer
    PRS_SourceAsyncSignalSet(PRS_CHANNEL_LETIMER_IADC,
                             PRS_ASYNC_CH_CTRL_SOURCESEL_LETIMER0,
                             PRS_ASYNC_CH_CTRL_SIGSEL_LETIMER0CH0);

    // Consumer
    PRS_ConnectConsumer(PRS_CHANNEL_LETIMER_IADC,
                        prsTypeAsync,
                        prsConsumerIADC0_SINGLETRIGGER);
  }
}

/***************************************************************************//**
 * @brief
 *    Initialize LDMA.
 ******************************************************************************/
static void init_ldma(void)
{
  LDMA_DeInit();

  // Declare init struct
  LDMA_Init_t init = LDMA_INIT_DEFAULT;

  // Initialize LDMA
  LDMA_Init(&init);

  // Number of DMA transfers
  descriptor.xfer.xferCnt = NUM_OF_SAMPLES;

  // Trigger interrupt when samplingBuffer is filled
  descriptor.xfer.doneIfs = true;

  // Enable LDMA Interrupt
  NVIC_ClearPendingIRQ(LDMA_IRQn);
  NVIC_EnableIRQ(LDMA_IRQn);
}

/***************************************************************************//**
 * @brief
 *    Convert raw ADC data to millivolts.
 ******************************************************************************/
static uint32_t convert_to_mv(uint32_t raw)
{
  return raw * 3300 / 0xFFF;  // 12 bit adc
}

/***************************************************************************//**
 * @brief
 *    Initialize the low energy peripherals to measure the supply voltage.
 *
 * @details
 *    The LETIMER, PRS, IADC, and LDMA peripherals are initialized. The
 *    LETIMER's underflow event will be connected to the IADC start conversion
 *    trigger through PRS. The LDMA will transfer the data to a buffer when the
 *    IADC conversion is complete.
 *
 * @note
 *    The LDMA will not begin transferring the data after initialization. The
 *    le_voltage_monitor_start_next() function must be called to start
 *    measuring data.
 ******************************************************************************/
void le_voltage_monitor_init(void)
{
  init_clocks();
  init_letimer();
  init_prs();
  init_iadc();
  init_ldma();
}

/***************************************************************************//**
 * @brief
 *    Gets the average millivoltage of the samples taken between complete LDMA
 *    transfers.
 *
 * @return
 *    Average voltage in millivolts
 ******************************************************************************/
uint16_t le_voltage_monitor_get_average_mv(void)
{
  uint32_t avg = 0;
  for (int32_t i = 0; i < NUM_OF_SAMPLES; i++) {
    avg += convert_to_mv(samplingBuffer[i]);
  }
  return avg / NUM_OF_SAMPLES;
}

/***************************************************************************//**
 * @brief
 *    Starts the peripherals to begin sampling until internal buffer is filled.
 ******************************************************************************/
void le_voltage_monitor_start_next(void)
{
  if (!startedSampling) {
    IADC_command(IADC0, iadcCmdStartSingle);

    // Start timer
    LETIMER_Enable(LETIMER0, true);

    // Start LDMA
    LDMA_StartTransfer(LDMA_CHANNEL, &xferCfg, &descriptor);

    // Set flag to indicate sampling is occuring.
    startedSampling = true;
  }
}

/***************************************************************************//**
 * @brief
 *    Stops the sampling.
 ******************************************************************************/
void le_voltage_monitor_stop(void)
{
  // Stop timer
  LETIMER_Enable(LETIMER0, false);

  // Stop IADC
  IADC_command(IADC0, iadcCmdStopSingle);

  // Reset flag
  startedSampling = false;

  // Stop LDMA
  LDMA_StopTransfer(LDMA_CHANNEL);
}

/***************************************************************************//**
 * @brief
 *    LDMA Interrupt Handler
 ******************************************************************************/
void LDMA_IRQHandler(void)
{
  // Clear interrupts
  LDMA_IntClear(LDMA_IntGet());

  // Stop timer
  LETIMER_Enable(LETIMER0, false);

  // Stop ADC
  IADC_command(IADC0, iadcCmdStopSingle);

  // Signal ble stack that LDMA has finished
  sl_bt_external_signal(LE_MONITOR_SIGNAL);

  // Set flag to indicate sampling finished
  startedSampling = false;
}

/**************************************************************************//**
 * @brief  LETIMER interrupt handler
 *****************************************************************************/
void LETIMER0_IRQHandler(void)
{
  uint32_t flags = LETIMER_IntGet(LETIMER0);

  // Clear LETIMER interrupt flags
  LETIMER_IntClear(LETIMER0, flags);

  if (prs_feature_enable == false) {
    // Trigger an IADC single conversion
    IADC_command(IADC0, iadcCmdStartSingle);
  }
}

void sl_button_on_change(const sl_button_t *handle)
{
  if (handle == sl_simple_button_array[0]) {
    if (handle->get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
      sl_bt_external_signal(LE_PRS_FEATURE_CONFIG_SIGNAL);
    }
  }
}

void le_voltage_monitor_conifg_prs_feature(void)
{
  le_voltage_monitor_stop();
  prs_feature_enable = !prs_feature_enable;
  le_voltage_monitor_init();
  le_voltage_monitor_start_next();
}
