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
#include "keyleds/features.h"
#include "keyleds/logging.h"

enum leds_feature_function {    /* Function table for KEYLEDS_FEATURE_GAMEMODE */
    F_GET_MAX = 0,
    F_BLOCK_KEYS = 1,
    F_UNBLOCK_KEYS = 2,
    F_CLEAR = 3
};

#define KEYS_PER_COMMAND    (16)


/** Get the maximum number of blocked keys.
 * @param device Open device as returned by keyleds_open().
 * @param [out] nb Maximum number of keys that can be blocked when game mode is enabled.
 * @return `true` on success, `false` on error.
 */
KEYLEDS_EXPORT bool keyleds_gamemode_max(Keyleds * device, unsigned * nb)
{
    uint8_t data[1];

    assert(device != NULL);
    assert(nb != NULL);

    if (keyleds_call(device, data, sizeof(data),
                     KEYLEDS_FEATURE_GAMEMODE, F_GET_MAX,
                     0, NULL) < 0) {
        return false;
    }
    *nb = (unsigned)data[0];
    return true;
}


/** Send a group of keys to the device.
 * @param device Open device as returned by keyleds_open().
 * @param ids Array of key identifiers to affect.
 * @param ids_nb Number of keys in `ids`.
 * @param set `true` to add keys to the list, `false` to remove them.
 * @return `true` on success, `false` on error.
 */
static bool gamemode_send(Keyleds * device, const uint8_t * ids, unsigned ids_nb, bool set)
{
    assert(device != NULL);
    assert(ids != NULL);

    for (unsigned offset = 0; offset < ids_nb; offset += KEYS_PER_COMMAND) {
        unsigned batch_size = KEYS_PER_COMMAND;
        if (batch_size > ids_nb - offset) { batch_size = ids_nb - offset; }
        if (keyleds_call(device, NULL, 0,
                         KEYLEDS_FEATURE_GAMEMODE, set ? F_BLOCK_KEYS : F_UNBLOCK_KEYS,
                         batch_size, ids + offset) < 0) {
            return false;
        }
    }
    return true;
}


/** Add a group of keys to blocked list.
 * @param device Open device as returned by keyleds_open().
 * @param ids Array of key identifiers to add.
 * @param ids_nb Number of keys in `ids`.
 * @return `true` on success, `false` on error.
 */
KEYLEDS_EXPORT bool keyleds_gamemode_set(Keyleds * device, const uint8_t * ids, unsigned ids_nb)
{
    return gamemode_send(device, ids, ids_nb, true);
}


/** Remove a group of keys from blocked list.
 * @param device Open device as returned by keyleds_open().
 * @param ids Array of key identifiers to remove.
 * @param ids_nb Number of keys in `ids`.
 * @return `true` on success, `false` on error.
 */
KEYLEDS_EXPORT bool keyleds_gamemode_clear(Keyleds * device, const uint8_t * ids, unsigned ids_nb)
{
    return gamemode_send(device, ids, ids_nb, false);
}


/** Clear the blocked keys list.
 * @param device Open device as returned by keyleds_open().
 * @return `true` on success, `false` on error.
 */
KEYLEDS_EXPORT bool keyleds_gamemode_reset(Keyleds * device)
{
    assert(device != NULL);

    if (keyleds_call(device, NULL, 0,
                     KEYLEDS_FEATURE_GAMEMODE, F_CLEAR, 0, NULL) < 0) {
        return false;
    }
    return true;
}
