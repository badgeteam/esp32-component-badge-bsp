#pragma once

#include "sdmmc_cmd.h"

// Replace ESP-IDF's FatFs disk layer for this card with one that RETRIES
// (badge_bsp_storage_retry.c). IDF's does not, and on this hardware one
// disturbed transaction is otherwise permanent: FatFs marks the file object
// dead, leaks the cluster it had just allocated, and never repairs either.
//
// Called by bsp_storage_mount_advanced() straight after a successful mount,
// so it is not part of the public storage API - no caller has to know it
// exists, and no caller can forget it.
void bsp_storage_install_retrying_diskio(sdmmc_card_t* card);

// Undo it: drop the card and release the bounce buffer.
// Called by bsp_storage_unmount() once the card is really gone, so the 8 KiB
// is only held while a card is mounted.
void bsp_storage_release_retrying_diskio(void);
