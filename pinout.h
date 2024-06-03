#ifndef PINOUT_H
# define PINOUT_H

// DTE-ETSIST-UPM

// Microprocesadores 21/22

// Pinout for the STM Nucleo-f303k8 and Nucleo-l432kc boards
// Solder bridges SB16 & SB18 must be removed

# ifdef  TARGET_NUCLEO_L432KC
#   define LDL_PIN   (D9)  // left LED
#   define LDM_PIN   (D10) // middle LED
#   define LDR_PIN   (D11) // right LED
#   define SWL_PIN   (D1)  // left switch
#   define SWM_PIN   (D0)  // middle switch
#   define SWR_PIN   (D2)  // right switch
#   define SGG_PIN   (A5)  // G segment
#   define SGF_PIN   (A6)  // F segment
#   define SGE_PIN   (A0)  // E segment
#   define SGD_PIN   (D13) // D segment
#   define SGC_PIN   (D12) // C segment
#   define SGB_PIN   (A3)  // B segment
#   define SGA_PIN   (A4)  // A segment
#   define DSL_PIN   (A2)  // left display common terminal
#   define DSR_PIN   (A1)  // right display common terminal
#   define LIT_PIN   (D3)  // analog input
#   define TRG_PIN   (D5)  // TRIGGER (input for the HC-SR04 sensor)
#   define ECH_PIN   (D4)  // ECHO (output from the HC-SR04 sensor)
#   define SRV_PIN   (D6)  // PWM out for the servo
#   define SDA_PIN   (D4)  // I2C SDA (beware, it is also used as ECHO)
#   define SCL_PIN   (D5)  // I2C SCL (beware, it is also used as TRIGGER)
# endif  // TARGET_NUCLEO_L432KC

#endif  // PINOUT_H
