/* tnrMods-a.cpp */
/* new file 18 December 2024 */

#include <Arduino.h>
// #include "tnrMods-a.h"

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

// #include "pico/bootrom.h"
// #include "pico/bootrom_constants.h" // order of these two - and - compiles without TODO

void reflash_firmware_tnr(void) {

   /* BUG - REBOOT_TYPE_NORMAL only in comments not pico-sdk proper code */

   rom_reboot(BOOT_TYPE_NORMAL, 500, 0, 0);

   if (0) {
        ; // reset_usb_boot(0, 0);
   }
   if (-1) {
        ;
   }
}

void gojiraBoot() {
    Serial.println(
        " gojiraBoot() called here.. "
    );
    delay(5000);
    Serial.println(
        " .. would be rebooting here w reflash testing.."
    );
    reflash_firmware_tnr();
}

/* end */
