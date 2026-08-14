#pragma once
#include_next <mcuconf.h>

/* GP0/GP1 sao pinos do periferico I2C0 do RP2040.
 * O padrao do QMK para RP2040 usa I2C1 (pinos GP2/GP3), que aqui
 * colidem com o Encoder 1. Por isso precisamos trocar o periferico.
 */
#undef RP_I2C_USE_I2C0
#define RP_I2C_USE_I2C0 TRUE

#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 FALSE
