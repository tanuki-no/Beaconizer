/*!
 *	\file		hci.h
 *	\brief		Bluetooth Host Controller Interface (HCI) functions
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		25/06/2022
 *	\version	1.0
 */

#include <stdbool.h>
#include <stdint.h>

#pragma once

#ifndef __BTEST_HCI_H__
#define __BTEST_HCI_H__

typedef void (*bt_hci_destroy_fn_t)(
    void *user_data);

struct bt_hci;

struct bt_hci *bt_hci_new(
    int fd);
struct bt_hci *bt_hci_new_user_channel(
    uint16_t index);
struct bt_hci *bt_hci_new_raw_device(
    uint16_t index);

struct bt_hci *bt_hci_ref(
    struct bt_hci *hci);
void bt_hci_unref(
    struct bt_hci *hci);

bool bt_hci_set_close_on_unref(
    struct bt_hci *hci,
    bool do_close);

typedef void (*bt_hci_callback_fn_t)(
    const void *data,
    uint8_t size,
    void *user_data);

unsigned int bt_hci_send(
    struct bt_hci   *hci,
    uint16_t        opcode,
    const void      *data,
    uint8_t         size,
    bt_hci_callback_fn_t callback,
    void            *user_data,
    bt_hci_destroy_fn_t destroy);

bool bt_hci_cancel(
    struct bt_hci *hci,
    unsigned int id);

bool bt_hci_flush(
    struct bt_hci *hci);

unsigned int bt_hci_register(
    struct bt_hci *hci,
    uint8_t event,
    bt_hci_callback_fn_t callback,
    void *user_data,
    bt_hci_destroy_fn_t destroy);

bool bt_hci_unregister(
    struct bt_hci *hci,
    unsigned int id);

#endif /* __BTEST_HCI_H__ */

/* End of file */