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
