/*!
 *	\file		db.h
 *	\brief		Bluetooth UUID database
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		30/06/2022
 *	\version	1.0
 */

#pragma once

#include <stdint.h>

#ifndef __BTEST_DB_H__
#define __BTEST_DB_H__

/* Convert 16-bit characteristic to string */
const char *uuid2str16(const uint16_t uuid);

/* Convert 32-bit characteristic to string */
const char *uuid2str32(const uint32_t uuid);

/* Convert UUID to string */
const char *uuid2str128(const uint8_t uuid[16]);

/* Convert UUID to string */
const char *detect_vendor(const char *uuid);

/* Convert 16-bit characteristic to string */
const char *detect_device(const uint16_t appearance);

#endif /* __BTEST_UTILITY_H__ */

/* End of file */