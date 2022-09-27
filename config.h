#pragma once

// Pins (GPIO)
// Status LED
const int P_STATUS_LED = 2;
// Circulation pump
const int P_PUMP = 5;

// Soft UART config
// Rx -- GPIO 12
// Tx -- GPIO 14
const int SOFT_RX = 12;
const int SOFT_TX = 14;


// Addresses
const uint16_t ADDR_ETH_MODULE = 0xFFF4;
const uint16_t ADDR_VALVE = 0x0F8A;


// Fields (read)
// Time & Date
const uint16_t F_TIME_r = 0x1620;
const uint16_t F_WDAY_r = 0x1621;
// Temps
const uint16_t F_CO_CURRENT_r = 0x157D;
const uint16_t F_CO_TARGET_r = 0x157E;
const uint16_t F_CWU_CURRENT_r = 0x166E;
const uint16_t F_CWU_TARGET_r = 0x1616;
const uint16_t F_VALVE_CURRENT_r = 0x1614;
const uint16_t F_VALVE_TARGET_r = 0x167f;
// Temp ranges
const uint16_t F_CO_RANGE_r = 0x169E;
const uint16_t F_CWU_RANGE_r = 0x169F;
const uint16_t F_VALVE_RANGE_r = 0x16C3;
// Pump
const uint16_t F_PUMPS_MODE_r = 0x15CD;
// Control unit
const uint16_t F_CU_STATE_r = 0x157C;


// Fields (write)
// Time & Date
const uint16_t F_TIME_W = 0x298;
const uint16_t F_WDAY_W = 0x299;
// Temps
const uint16_t F_CO_TARGET_W = 0x01F6;
const uint16_t F_CWU_TARGET_W = 0x028E;
const uint16_t F_VALVE_TARGET_W = 0x02f7;
// Pump
const uint16_t F_PUMPS_MODE_W = 0x0245;