/*!
 *	\file		db.h
 *	\brief		Bluetooth UUID database
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		30/06/2022
 *	\version	1.0
 */

#pragma once

#ifndef __BTEST_DB_H__
#define __BTEST_DB_H__

/* String utilities */
const char *bt_uuid16_to_str(uint16_t uuid);
const char *bt_uuid32_to_str(uint32_t uuid);
const char *bt_uuid128_to_str(const uint8_t uuid[16]);
const char *bt_uuidstr_to_str(const char *uuid);
const char *bt_appear_to_str(uint16_t appearance);

char *strdelimit(char *str, char *del, char c);
int strsuffix(const char *str, const char *suffix);

#endif /* __BTEST_UTILITY_H__ */

/* End of file */