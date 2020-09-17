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
#ifndef KEYLEDS_DEVICE_H
#define KEYLEDS_DEVICE_H

#include <stdint.h>
#include <sys/types.h>

struct keyleds_device_reports {
    uint8_t     id;
    uint8_t     size;
};
#define DEVICE_REPORT_INVALID   (0xff)

struct keyleds_device_feature {
    uint16_t    id;                             /* feature identifier from features.h */
    uint8_t     target_id;                      /* target device for feature */
    uint8_t     index;                          /* position of feature */
    bool        reserved:1;
    bool        hidden:1;
    bool        obsolete:1;
};

struct keyleds_device {
    int         fd;                             /* device file descriptor */
    uint8_t     target_id;                      /* Device's target identifier, for devices behind a unifying receiver.
                                                 * for the receiver itself, or for directly attached devices, use
                                                 * KEYLEDS_TARGET_DEFAULT. */
    uint8_t     app_id;                         /* our application identifier */
    uint8_t     ping_seq;                       /* using for resyncing after errors */
    unsigned    timeout;                        /* read timeout in microseconds */

    struct keyleds_device_reports * reports;    /* list of device-supported hid reports */
    unsigned    max_report_size;                /* maximum number of bytes in a report */

    struct keyleds_device_feature * features;   /* feature index cache */

    keyleds_gkeys_cb gkeys_cb;                  /* callback to invoke on gkey presses */
    void *      userdata;                       /* for library user */
};


/****************************************************************************/
/* Core functions */

bool keyleds_send(Keyleds * device, uint8_t feature_idx,
                  uint8_t function, size_t length, const uint8_t * data);
bool keyleds_receive(Keyleds * device, uint8_t feature_idx,
                     uint8_t * message, size_t * size);
ssize_t keyleds_call(Keyleds * device, /*@null@*/ /*@out@*/ uint8_t * result, size_t result_len,
                     uint16_t feature_id, uint8_t function,
                     size_t length, const uint8_t * data);

void keyleds_gkeys_filter(Keyleds * device, uint8_t buffer[], ssize_t buflen);

/****************************************************************************/
/* Helpers */

static inline const uint8_t * keyleds_response_data(Keyleds * device, const uint8_t * message)
{
    (void)device;
    return message + 4;
}

/****************************************************************************/


#endif
