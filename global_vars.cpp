/*
 * Globals for VFD Modular Clock
 * (C) 2011-2012 Akafugu Corporation
 * (C) 2012 William B Phelps
 *
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 */
//#define FEATURE_AUTO_DIM  // moved to Makefile

#include "global.h"

#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "global_vars.h"

// Settings saved to eeprom

#define EEPROM_GUARD_BYTE 0x42
uint8_t EEMEM b_guard = EEPROM_GUARD_BYTE; // Guard byte to test if EEPROM is initialized or not

uint8_t EEMEM b_24h_clock = 1;
uint8_t EEMEM b_show_temp = 0;
uint8_t EEMEM b_show_dots = 1;
uint8_t EEMEM b_brightness = 8;
uint8_t EEMEM b_volume = 0;

uint8_t EEMEM b_dateyear = 13;
uint8_t EEMEM b_datemonth = 1;
uint8_t EEMEM b_dateday = 1;
uint8_t EEMEM b_alarmtype = ALARM_NORMAL;
uint8_t EEMEM b_snooze_enabled = false;

#ifdef HAVE_FLW
uint8_t EEMEM b_flw_enabled = 0;
#endif
#ifdef HAVE_GPS
uint8_t EEMEM b_gps_enabled = 0;  // 0, 48, or 96 - default no gps
uint8_t EEMEM b_TZ_hour = 0;
uint8_t EEMEM b_TZ_minute = 0;
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
uint8_t EEMEM b_DST_mode = 0;  // 0: off, 1: on, 2: Auto
uint8_t EEMEM b_DST_offset = 0;
#endif
#ifdef HAVE_AUTO_DATE
uint8_t EEMEM b_date_format = FORMAT_YMD;
uint8_t EEMEM b_Region = 0;  // default European date format Y/M/D
uint8_t EEMEM b_AutoDate = 0;
#endif
#ifdef HAVE_AUTO_DIM
uint8_t EEMEM b_AutoDim = 0;
uint8_t EEMEM b_AutoDimHour = 22;
uint8_t EEMEM b_AutoDimLevel = 2;
uint8_t EEMEM b_AutoBrtHour = 7;
uint8_t EEMEM b_AutoBrtLevel = 8;
#endif
#ifdef HAVE_AUTO_DST
#ifdef DST_NSW
uint8_t EEMEM b_DST_Rule0 = 10;  // DST start month
uint8_t EEMEM b_DST_Rule1 = 1;  // DST start dotw
uint8_t EEMEM b_DST_Rule2 = 1;  // DST start week
uint8_t EEMEM b_DST_Rule3 = 2;  // DST start hour
uint8_t EEMEM b_DST_Rule4 = 4; // DST end month
uint8_t EEMEM b_DST_Rule5 = 1;  // DST end dotw
uint8_t EEMEM b_DST_Rule6 = 1;  // DST end week
uint8_t EEMEM b_DST_Rule7 = 2;  // DST end hour
uint8_t EEMEM b_DST_Rule8 = 1;  // DST offset
#else
uint8_t EEMEM b_DST_Rule0 = 3;  // DST start month
uint8_t EEMEM b_DST_Rule1 = 1;  // DST start dotw
uint8_t EEMEM b_DST_Rule2 = 2;  // DST start week
uint8_t EEMEM b_DST_Rule3 = 2;  // DST start hour
uint8_t EEMEM b_DST_Rule4 = 11; // DST end month
uint8_t EEMEM b_DST_Rule5 = 1;  // DST end dotw
uint8_t EEMEM b_DST_Rule6 = 1;  // DST end week
uint8_t EEMEM b_DST_Rule7 = 2;  // DST end hour
uint8_t EEMEM b_DST_Rule8 = 1;  // DST offset
#endif
#endif

int8_t g_24h_clock = true;
int8_t g_show_temp = true;
int8_t g_show_dots = true;
int8_t g_brightness = 5;
int8_t g_volume = 0;

int8_t g_dateyear;
int8_t g_datemonth;
int8_t g_dateday;
extern int8_t g_autodate;
uint8_t g_alarmtype;
uint8_t g_snooze_enabled;

uint8_t g_has_flw;  // does the unit have an EEPROM with the FLW database?
int8_t g_flw_enabled;

#ifdef HAVE_GPS 
int8_t g_gps_enabled;
int8_t g_TZ_hour;
int8_t g_TZ_minute;
// debugging counters 
int8_t g_gps_cks_errors;  // gps checksum error counter
int8_t g_gps_parse_errors;  // gps parse error counter
int8_t g_gps_time_errors;  // gps time error counter
#endif
int8_t g_gps_updating;  // for signalling GPS update on some displays
#if defined HAVE_GPS || defined HAVE_AUTO_DST
int8_t g_DST_mode;  // DST off, on, auto?
int8_t g_DST_offset;  // DST offset in Hours
int8_t g_DST_updated;  // DST update flag = allow update only once per day
#endif
#ifdef HAVE_AUTO_DST  // DST rules
int8_t g_DST_Rules[9];
#endif
#ifdef HAVE_AUTO_DATE
date_format_t g_date_format;
int8_t g_AutoDate;
#endif
#ifdef HAVE_AUTO_DIM
int8_t g_AutoDim;
int8_t g_AutoDimHour;
int8_t g_AutoDimLevel;
int8_t g_AutoBrtHour;
int8_t g_AutoBrtLevel;
#endif
uint8_t g_has_dots; // can current shield show dot (decimal points)

// workaround: Arduino avr-gcc toolchain is missing eeprom_update_byte
#define eeprom_update_byte eeprom_write_byte

void clean_eeprom()
{
    eeprom_update_byte(&b_24h_clock, 1);    
    eeprom_update_byte(&b_show_temp, 0);    
    eeprom_update_byte(&b_show_dots, 1);    
    eeprom_update_byte(&b_brightness, 8);    
    eeprom_update_byte(&b_volume, 0);    

    eeprom_update_byte(&b_dateyear, 13);    
    eeprom_update_byte(&b_datemonth, 1);
    eeprom_update_byte(&b_dateday, 1);
    eeprom_update_byte(&b_alarmtype, ALARM_NORMAL);
    eeprom_update_byte(&b_snooze_enabled, false);

#ifdef HAVE_FLW
    eeprom_update_byte(&b_flw_enabled, 0);
#endif
#ifdef HAVE_GPS
    eeprom_update_byte(&b_gps_enabled, 0);    
    eeprom_update_byte(&b_TZ_hour, 0);    
    eeprom_update_byte(&b_TZ_minute, 0);    
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
    eeprom_update_byte(&b_DST_mode, 0);    
    eeprom_update_byte(&b_DST_offset, 0);
#endif
#ifdef HAVE_AUTO_DATE
    eeprom_update_byte(&b_date_format, 0);    
    eeprom_update_byte(&b_Region, 0);    
    eeprom_update_byte(&b_AutoDate, 0);
#endif
#ifdef HAVE_AUTO_DIM
    eeprom_update_byte(&b_AutoDim, 0);    
    eeprom_update_byte(&b_AutoDimHour, 0);    
    eeprom_update_byte(&b_AutoDimLevel, 0);
    eeprom_update_byte(&b_AutoBrtHour, 0);    
    eeprom_update_byte(&b_AutoBrtLevel, 0);
#endif
#ifdef HAVE_AUTO_DST
#ifdef DST_NSW
    eeprom_update_byte(&b_DST_Rule0, 10);
    eeprom_update_byte(&b_DST_Rule1, 1);
    eeprom_update_byte(&b_DST_Rule2, 1);
    eeprom_update_byte(&b_DST_Rule3, 2);
    eeprom_update_byte(&b_DST_Rule4, 3);
    eeprom_update_byte(&b_DST_Rule5, 1);
    eeprom_update_byte(&b_DST_Rule6, 1);
    eeprom_update_byte(&b_DST_Rule7, 2);
    eeprom_update_byte(&b_DST_Rule8, 1);
#else
    eeprom_update_byte(&b_DST_Rule0, 3);
    eeprom_update_byte(&b_DST_Rule1, 1);
    eeprom_update_byte(&b_DST_Rule2, 2);
    eeprom_update_byte(&b_DST_Rule3, 2);
    eeprom_update_byte(&b_DST_Rule4, 11);
    eeprom_update_byte(&b_DST_Rule5, 1);
    eeprom_update_byte(&b_DST_Rule6, 1);
    eeprom_update_byte(&b_DST_Rule7, 2);
    eeprom_update_byte(&b_DST_Rule8, 1);
#endif
#endif

    eeprom_update_byte(&b_guard, EEPROM_GUARD_BYTE);    
}

void globals_init(void)
{
    // check if EEPROM is initialized
    uint8_t guard = eeprom_read_byte(&b_guard);
    
    if (guard != EEPROM_GUARD_BYTE) {
        Serial.println("Initializing EEPROM for first time use");
        clean_eeprom();
    }
    
	// read eeprom
	g_24h_clock  = eeprom_read_byte(&b_24h_clock);
	g_show_temp  = eeprom_read_byte(&b_show_temp);
	g_show_dots  = eeprom_read_byte(&b_show_dots);
	g_brightness = eeprom_read_byte(&b_brightness);
	g_volume     = eeprom_read_byte(&b_volume);
	g_alarmtype  = eeprom_read_byte(&b_alarmtype);
        g_snooze_enabled = eeprom_read_byte(&b_snooze_enabled);

#ifdef HAVE_FLW
	g_flw_enabled = eeprom_read_byte(&b_flw_enabled);
    Serial.print("g_flw_enabled = ");
    Serial.println(g_flw_enabled);
#else
        g_flw_enabled = 0;
#endif
#ifdef HAVE_GPS
	g_gps_enabled = eeprom_read_byte(&b_gps_enabled);
    Serial.print("g_gps_enabled = ");
    Serial.println(g_gps_enabled);
	if (g_gps_enabled != 0 && g_gps_enabled != 48 && g_gps_enabled != 96) g_gps_enabled = 0;
	g_TZ_hour = eeprom_read_byte(&b_TZ_hour) - 12;
	if ((g_TZ_hour<-12) || (g_TZ_hour>12))  g_TZ_hour = 0;  // add range check
	g_TZ_minute = eeprom_read_byte(&b_TZ_minute);
	if (g_TZ_minute % 15 != 0) g_TZ_minute = 0;
#endif
#if defined HAVE_GPS || defined HAVE_AUTO_DST
	g_DST_mode = eeprom_read_byte(&b_DST_mode);
#ifdef HAVE_AUTO_DST
	if (g_DST_mode < 0 || g_DST_mode > 2) g_DST_mode = 0;
#else
	if (g_DST_mode < 0 || g_DST_mode > 1) g_DST_mode = 0;
#endif

	g_DST_offset = eeprom_read_byte(&b_DST_offset);
	g_DST_offset = 1; // use fixed offset for now
	g_DST_updated = false;  // allow automatic DST update

        Serial.print("g_DST_mode = ");
        Serial.println(g_DST_mode);
        Serial.print("g_DST_offset = ");
        Serial.println(g_DST_offset);

#endif
#ifdef HAVE_AUTO_DATE
	g_date_format = (date_format_t)eeprom_read_byte(&b_date_format);
	g_AutoDate = eeprom_read_byte(&b_AutoDate);
#endif
#ifdef HAVE_AUTO_DIM
	g_AutoDim = eeprom_read_byte(&b_AutoDim);
	g_AutoDimHour = eeprom_read_byte(&b_AutoDimHour);
	g_AutoDimLevel = eeprom_read_byte(&b_AutoDimLevel);
	g_AutoBrtHour = eeprom_read_byte(&b_AutoBrtHour);
	g_AutoBrtLevel = eeprom_read_byte(&b_AutoBrtLevel);
#endif
	g_dateyear = 12;
	g_datemonth = 1;
	g_dateday = 1;
#ifdef HAVE_AUTO_DST
//	g_DST_Rules[0] = eeprom_read_byte(&b_DST_Rule0);  // DST start month
//	g_DST_Rules[1] = eeprom_read_byte(&b_DST_Rule1);  // DST start dotw
//	g_DST_Rules[2] = eeprom_read_byte(&b_DST_Rule2);  // DST start week
//	g_DST_Rules[3] = eeprom_read_byte(&b_DST_Rule3);  // DST start hour
//	g_DST_Rules[4] = eeprom_read_byte(&b_DST_Rule4); // DST end month
//	g_DST_Rules[5] = eeprom_read_byte(&b_DST_Rule5);  // DST end dotw
//	g_DST_Rules[6] = eeprom_read_byte(&b_DST_Rule6);  // DST end week
//	g_DST_Rules[7] = eeprom_read_byte(&b_DST_Rule7);  // DST end hour
//	g_DST_Rules[8] = eeprom_read_byte(&b_DST_Rule8);  // DST offset
//        g_DST_Rules[] = {3,1,2,2,11,1,1,2,1};  // TEMP 26mar13/wbp
// temp - set rules for USA until menu is sorted out and rules are in EE again
        g_DST_Rules[0] = 3;  // TEMP 26mar13/wbp
        g_DST_Rules[1] = 1;  // TEMP 26mar13/wbp
        g_DST_Rules[2] = 2;  // TEMP 26mar13/wbp
        g_DST_Rules[3] = 2;  // TEMP 26mar13/wbp
        g_DST_Rules[4] = 11;  // TEMP 26mar13/wbp
        g_DST_Rules[5] = 1;  // TEMP 26mar13/wbp
        g_DST_Rules[6] = 1;  // TEMP 26mar13/wbp
        g_DST_Rules[7] = 2;  // TEMP 26mar13/wbp
        g_DST_Rules[8] = 1;  // TEMP 26mar13/wbp
#endif
 }
