/* tnrMods-a.h */
/* new file 18 December 2024 */

#include "pico/bootrom.h"

// #include "pico/bootrom_constants.h" // order of these two - and - compiles without TODO

/*
 * 
 * ref:
 *   https://github.com/raspberrypi/pico-sdk/blob/master/src/rp2_common/pico_bootrom/include/pico/bootrom.h#L441
 * 
 */

/*

static inline int rom_reboot(uint32_t flags, uint32_t delay_ms, uint32_t p0, uint32_t p1) {
    rom_reboot_fn func = (rom_reboot_fn) rom_func_lookup_inline(ROM_FUNC_REBOOT);
    return func(flags, delay_ms, p0, p1);
}

*/

/* may not be needed for c++ */
// extern void reflash_firmware_tnr(void);
extern void gojiraBoot();

/* end */
