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
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "keyleds.h"
#include "keyleds/device.h"
#include "keyleds/error.h"
#include "keyleds/features.h"
#include "keyleds/logging.h"

enum leds_feature_function {    /* Function table for KEYLEDS_FEATURE_LEDS2 */
    F_SET_LEDS = 1,
    F_SET_LEDS_COLOR_SEQUENCE = 2,
    F_SET_LEDS_RANGE = 5,
    F_SET_LEDS_SINGLE_COLOR = 6,
    F_COMMIT = 7
};


/** Set the color of a set of LEDs.
 * Updates an internal buffer on the device. Actual lights are not updated until
 * keyleds_commit_leds2() is called.
 * @param device Open device as returned by keyleds_open().
 * @param keys Color table with `keys_nb` entries.
 * @param keys_nb Number of keys to send.
 * @return `true` on success, `false` on error.
 */
KEYLEDS_EXPORT bool keyleds_set_leds2(Keyleds * device, const struct keyleds_key_color * keys, unsigned keys_nb)
{
    assert(device != NULL);
    assert(keys != NULL);
    assert(keys_nb <= UINT16_MAX);

    unsigned per_call = (device->max_report_size - 3) / 4;  /* 4 bytes per key, mins headers */
    unsigned offset, idx;

    uint8_t data[per_call * 4];

    /* Send keys in chunks */
    for (offset = 0; offset < keys_nb; offset += per_call) {
        unsigned batch_length = offset + per_call > keys_nb ? keys_nb - offset : per_call;
        for (idx = 0; idx < batch_length; idx += 1) {
            data[idx * 4 + 0] = keys[offset + idx].id;
            data[idx * 4 + 1] = keys[offset + idx].red;
            data[idx * 4 + 2] = keys[offset + idx].green;
            data[idx * 4 + 3] = keys[offset + idx].blue;
        }

        if (keyleds_call(device, NULL, 0,
                         KEYLEDS_FEATURE_LEDS2, F_SET_LEDS,
                         batch_length * 4, data) < 0) {
            return false;
        }
    }
    return true;
}


/** Apply LED state changes.
 * Other functions only update an internal buffer on the device. The actual colors
 * of the lights are only shown after a commit.
 * @param device Open device as returned by keyleds_open().
 * @return `true` on success, `false` on error.
 * @note This device function takes relatively longer than most others. Testing on a G410
 *       showed 10 to 15 milliseconds runtimes.
 */
KEYLEDS_EXPORT bool keyleds_commit_leds2(Keyleds * device)
{
    assert(device != NULL);
    return keyleds_call(device, NULL, 0, KEYLEDS_FEATURE_LEDS2, F_COMMIT,
                        0, NULL) >= 0;
}
