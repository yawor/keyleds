/* Keyleds -- Gaming keyboard tool
 * Copyright (C) 2017 Julien Hartmann, juli1.hartmann@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KEYLEDS_H
#define KEYLEDS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOGITECH_VENDOR_ID  ((uint16_t)0x046d)
#define KEYLEDS_TARGET_DEFAULT ((uint8_t)0xff)

typedef struct keyleds_device Keyleds;

/****************************************************************************/
/* Device setup */

#define KEYLEDS_APP_ID_MIN  ((uint8_t)0x0)
#define KEYLEDS_APP_ID_MAX  ((uint8_t)0xf)

Keyleds * keyleds_open(const char * path, uint8_t target_id, uint8_t app_id);
void keyleds_close(Keyleds * device);
void keyleds_set_timeout(Keyleds * device, unsigned us);
int keyleds_device_fd(Keyleds * device);
bool keyleds_flush_fd(Keyleds * device);

/****************************************************************************/
/* Basic device communication */

typedef enum {
    KEYLEDS_DEVICE_HANDLER_DEVICE = (1<<0),
    KEYLEDS_DEVICE_HANDLER_GAMING = (1<<1),
    KEYLEDS_DEVICE_HANDLER_PREFERENCE = (1<<2),
    KEYLEDS_DEVICE_HANDLER_FEATURE = (1<<7)
} keyleds_device_handler_t;

bool keyleds_get_protocol(Keyleds * device,
                          unsigned * version, keyleds_device_handler_t * handler);
bool keyleds_ping(Keyleds * device); /* re-sync with device after error */
unsigned keyleds_get_feature_count(Keyleds * dev);
uint16_t keyleds_get_feature_id(Keyleds * dev, uint8_t feature_idx);
uint8_t keyleds_get_feature_index(Keyleds * dev, uint16_t feature_id);

/****************************************************************************/
/* Device information */

struct keyleds_device_version {
    uint8_t     serial[4];
    uint16_t    transport;
    uint8_t     model[6];

    unsigned    length;
    struct {
                uint8_t     type;
                char        prefix[4];
                unsigned    version_major;
                unsigned    version_minor;
                unsigned    build;
                bool        is_active;
                uint16_t    product_id;
                uint8_t     misc[5];
    }           protocols[];
};

typedef enum {
    KEYLEDS_DEVICE_TYPE_KEYBOARD = 0,
    KEYLEDS_DEVICE_TYPE_REMOTE = 1,
    KEYLEDS_DEVICE_TYPE_NUMPAD = 2,
    KEYLEDS_DEVICE_TYPE_MOUSE = 3,
    KEYLEDS_DEVICE_TYPE_TOUCHPAD = 4,
    KEYLEDS_DEVICE_TYPE_TRACKBALL = 5,
    KEYLEDS_DEVICE_TYPE_PRESENTER = 6,
    KEYLEDS_DEVICE_TYPE_RECEIVER = 7
} keyleds_device_type_t;

bool keyleds_get_device_name(Keyleds * device,
                             /*@out@*/ char ** out);    /* caller must free() on success */
void keyleds_free_device_name(/*@only@*/ /*@out@*/ char *);
bool keyleds_get_device_type(Keyleds * device,
                             /*@out@*/ keyleds_device_type_t * out);
bool keyleds_get_device_version(Keyleds * device,
                                /*@out@*/ struct keyleds_device_version ** out);
void keyleds_free_device_version(/*@only@*/ /*@out@*/ struct keyleds_device_version *);

/****************************************************************************/
/* Gamemode feature */

bool keyleds_gamemode_max(Keyleds * device, /*@out@*/ unsigned * nb);
bool keyleds_gamemode_set(Keyleds * device,
                          const uint8_t * ids, unsigned ids_nb);    /* add some keys */
bool keyleds_gamemode_clear(Keyleds * device,
                            const uint8_t * ids, unsigned ids_nb);  /* remove some keys */
bool keyleds_gamemode_reset(Keyleds * device);   /* remove all keys */

/****************************************************************************/
/* GKeys feature */

typedef enum {
    KEYLEDS_GKEYS_GKEY,
    KEYLEDS_GKEYS_MKEY,
    KEYLEDS_GKEYS_MRKEY,
} keyleds_gkeys_type_t;

typedef void (*keyleds_gkeys_cb)(Keyleds * device,
                                 keyleds_gkeys_type_t, uint16_t mask, void *);

bool keyleds_gkeys_count(Keyleds * device, unsigned * nb);
bool keyleds_gkeys_enable(Keyleds * device, bool enabled);
void keyleds_gkeys_set_cb(Keyleds * device, keyleds_gkeys_cb, void * userdata);
bool keyleds_mkeys_set(Keyleds * device, uint8_t mask);
bool keyleds_mrkeys_set(Keyleds * device, uint8_t mask);

/****************************************************************************/
/* Keyboard layout feature */

typedef enum {
    KEYLEDS_KEYBOARD_LAYOUT_FRA = 5,
    KEYLEDS_KEYBOARD_LAYOUT_INVALID = -1
} keyleds_keyboard_layout_t;

keyleds_keyboard_layout_t keyleds_keyboard_layout(Keyleds * device);

/****************************************************************************/
/* Reportrate feature */

bool keyleds_get_reportrates(Keyleds * device, /*@out@*/ unsigned ** out);
void keyleds_free_reportrates(unsigned *);
bool keyleds_get_reportrate(Keyleds * device, /*@out@*/ unsigned * rate);
bool keyleds_set_reportrate(Keyleds * device, unsigned rate);

/****************************************************************************/
/* Leds features */

typedef enum {
    KEYLEDS_BLOCK_KEYS = (1<<0),
    KEYLEDS_BLOCK_MULTIMEDIA = (1<<1),
    KEYLEDS_BLOCK_GKEYS = (1<<2),
    KEYLEDS_BLOCK_LOGO = (1<<4),
    KEYLEDS_BLOCK_MODES = (1<<6),
    KEYLEDS_BLOCK_INVALID = -1
} keyleds_block_id_t;

struct keyleds_keyblocks_info {
    unsigned    length;
    struct {
        keyleds_block_id_t block_id;
        uint16_t    nb_keys;    /* number of keys in block */
        uint8_t     red;        /* maximum allowable value for red channel */
        uint8_t     green;      /* maximum allowable value for green channel */
        uint8_t     blue;       /* maximum allowable value for blue channel */
    }           blocks[];
};

struct keyleds_key_color {
    uint8_t     id;             /* as reported by keyboard */
    uint8_t     red;
    uint8_t     green;
    uint8_t     blue;
};
#define KEYLEDS_KEY_ID_INVALID  (0)

bool keyleds_get_block_info(Keyleds * device,
                            /*@out@*/ struct keyleds_keyblocks_info ** out);
void keyleds_free_block_info(/*@only@*/ /*@out@*/ struct keyleds_keyblocks_info * info);
bool keyleds_get_leds(Keyleds * device, keyleds_block_id_t block_id,
                      struct keyleds_key_color * keys, uint16_t offset, unsigned keys_nb);
bool keyleds_set_leds(Keyleds * device, keyleds_block_id_t block_id,
                      const struct keyleds_key_color * keys, unsigned keys_nb);
bool keyleds_set_led_block(Keyleds * device, keyleds_block_id_t block_id,
                           uint8_t red, uint8_t green, uint8_t blue);
bool keyleds_commit_leds(Keyleds * device);

/****************************************************************************/
/* Leds v2 features */

bool keyleds_set_leds2(Keyleds * device, const struct keyleds_key_color * keys, unsigned keys_nb);
bool keyleds_commit_leds2(Keyleds * device);

/****************************************************************************/
/* Error and logging */

typedef enum {
    KEYLEDS_NO_ERROR    = 0,
    KEYLEDS_ERROR_ERRNO,         /* system error, look it up in errno */
    KEYLEDS_ERROR_DEVICE,        /* error return by device */
    KEYLEDS_ERROR_IO_LENGTH,
    KEYLEDS_ERROR_HIDREPORT,
    KEYLEDS_ERROR_HIDNOPP,
    KEYLEDS_ERROR_HIDVERSION,
    KEYLEDS_ERROR_FEATURE_NOT_FOUND,
    KEYLEDS_ERROR_TIMEDOUT,
    KEYLEDS_ERROR_RESPONSE,
    KEYLEDS_ERROR_INVAL
} keyleds_error_t;

/*@observer@*/ const char * keyleds_get_error_str(void);
keyleds_error_t             keyleds_get_errno(void);

extern /*@null@*/ FILE * g_keyleds_debug_stream;
extern int g_keyleds_debug_level;
extern int g_keyleds_debug_hid;     /* set to KEYLEDS_LOG_DEBUG to see parser output */
#define KEYLEDS_LOG_ERROR       (1)
#define KEYLEDS_LOG_WARNING     (2)
#define KEYLEDS_LOG_INFO        (3)
#define KEYLEDS_LOG_DEBUG       (4)

/****************************************************************************/
/* String definitions and utility functions */

struct keyleds_indexed_string {unsigned id; /*@null@*/ /*@observer@*/ const char * str;};
#define KEYLEDS_STRING_INVALID  ((unsigned)-1)

extern const struct keyleds_indexed_string keyleds_feature_names[];
extern const struct keyleds_indexed_string keyleds_protocol_types[];
extern const struct keyleds_indexed_string keyleds_device_types[];
extern const struct keyleds_indexed_string keyleds_block_id_names[];
extern const struct keyleds_indexed_string keyleds_keycode_names[];

/*@observer@*/ const char * keyleds_lookup_string(const struct keyleds_indexed_string *,
                                                  unsigned id);
unsigned keyleds_string_id(const struct keyleds_indexed_string *,
                           const char * str);

/* keycode is code event (eg KEY_F1)
 * scancode is physical key id as defined by USB standards
 * The keycode/scancode translation only applies to keys from block 0x01 (keys block) and 0x02 (media) */
unsigned keyleds_translate_scancode(keyleds_block_id_t block, uint8_t scancode);
bool keyleds_translate_keycode(unsigned keycode, keyleds_block_id_t * block, uint8_t * scancode);

#ifdef __cplusplus
}
#endif

#endif
