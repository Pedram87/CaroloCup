/*
 * OpenDaVINCI - STM32F4 Discovery Board Software/Hardware Interface.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#include "DiscoveryBoard.h"

///////////////////////////////////////////////////////////////////////////////
// Data structures and configuation.
///////////////////////////////////////////////////////////////////////////////

#define INFRARED_ADC_GRP_NUM_CHANNELS   3
#define INFRARED_ADC_GRP_BUF_DEPTH      32

// Buffer for the data samples.
static adcsample_t infraredSamples[INFRARED_ADC_GRP_NUM_CHANNELS * INFRARED_ADC_GRP_BUF_DEPTH];
static float dIR0 = 0;
static float dIR1 = 0;
static float dIR2 = 0;

static const ADCConversionGroup infraredSensors = {
    FALSE, // No circular buffer mode.
    INFRARED_ADC_GRP_NUM_CHANNELS, // Number of the analog channels.
    NULL, // No callback function.
    NULL, // No error callback function.
    0, // CR1.
    ADC_CR2_SWSTART, // CR2.
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3) | ADC_SMPR1_SMP_AN14(ADC_SAMPLE_3) | ADC_SMPR1_SMP_AN15(ADC_SAMPLE_3), // Sample times for channels 10 - 18, here sample time for channel SENSOR.
    0, // Sample times for channels 0 - 9.
    ADC_SQR1_NUM_CH(INFRARED_ADC_GRP_NUM_CHANNELS), // ADC SQR1 register initialization data for conversion group sequence 13 - 16 + sequence length.
    0, // ADC SQR2 register initialization data for conversion group sequence 7 - 12.
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN14) | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN15) // ADC SQR3 register initialization data for conversion group sequence 1 - 6.
};

///////////////////////////////////////////////////////////////////////////////
// Interface methods.
///////////////////////////////////////////////////////////////////////////////

void getInfraredData(int data[3]) {
    data[0] = (int)dIR0;
    data[1] = (int)dIR1;
    data[2] = (int)dIR2;
}

void commandPrintInfraredDistances(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    chprintf(chp, "Infrared distances: d0 = %f, d1 = %f,  d2 = %f", dIR0, dIR1, dIR2);
    chprintf(chp, "\r\n");
}

///////////////////////////////////////////////////////////////////////////////
// Sensor reading methods.
///////////////////////////////////////////////////////////////////////////////

void readInfrared(void) {
    adcConvert(&ADCD1, &infraredSensors, infraredSamples, INFRARED_ADC_GRP_BUF_DEPTH);

    int i = 0;
    int d0 = 0;
    int d1 = 0;
    int d2 = 0;
    for(; i < INFRARED_ADC_GRP_NUM_CHANNELS * INFRARED_ADC_GRP_BUF_DEPTH; i+=3) {
        d0 += (int) infraredSamples[i];
        d1 += (int) infraredSamples[i+1];
        d2 += (int) infraredSamples[i+2];
    }

    // Average the last INFRARED_ADC_GRP_BUF_DEPTH sampled values.
    d0 /= INFRARED_ADC_GRP_BUF_DEPTH;
	dIR0 = 12.174 * powf((d0) * (3.0 / 4096), -1.051);

    d1 /= INFRARED_ADC_GRP_BUF_DEPTH;
	dIR1 = 12.174 * powf((d1) * (3.0 / 4096), -1.051);

    d2 /= INFRARED_ADC_GRP_BUF_DEPTH;
	dIR2 = 12.174 * powf((d2) * (3.0 / 4096), -1.051);
}

static WORKING_AREA(workingAreaThread_Infrared, 512);
static msg_t Thread_Infrared(void *arg) {
    (void)arg;

    chRegSetThreadName("Infrared");

    while (TRUE) {
        readInfrared();
        chThdSleepMilliseconds(60);
    }

    return (msg_t)0;
}

///////////////////////////////////////////////////////////////////////////////
// Initialization method.
///////////////////////////////////////////////////////////////////////////////

void initializeInfrared(void) {
    // Initializes the ADC driver 1 in order to access the analog values from the onboard temperature sensor.
	adcStart(&ADCD1, NULL);

	// Infrared distance sensor 0 is connected to pin PC1 (IN11).	
	palSetGroupMode(GPIOC, PAL_PORT_BIT(1), 0, PAL_MODE_INPUT_ANALOG);
	
	// Infrared distance sensor 1 is connected to pin PC4 (IN14).	
	palSetGroupMode(GPIOC, PAL_PORT_BIT(4), 0, PAL_MODE_INPUT_ANALOG);

	// Infrared distance sensor 2 is connected to pin PC5 (IN15).	
	palSetGroupMode(GPIOC, PAL_PORT_BIT(5), 0, PAL_MODE_INPUT_ANALOG);

    // Start infrared reading thread.
    chThdCreateStatic(workingAreaThread_Infrared, sizeof(workingAreaThread_Infrared),
                      NORMALPRIO + 10, Thread_Infrared, NULL);
}

