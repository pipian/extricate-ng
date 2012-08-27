/* freebsd.c - FreeBSD code for extracting data from a CD sector.
 *
 * Copyright (c) 2011, 2012 Ian Jacobi
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <camlib.h>
#include <cam/scsi/scsi_message.h>
#include <cueify/types.h>
#include <cueify/device.h>
#include "device_private.h"

#define READ_CD   0xBE  /** MMC op code for READ CD */

/** Struct representing READ CD command structure */
struct scsi_read_cd {
    uint8_t op_code;      /** MMC op code (0xBE) */
    uint8_t sector_type;  /** Expected sector type in bits 2-4 */
    uint8_t address[4];   /** Starting logical block address */
    uint8_t length[3];    /** Transfer length */
    /**
     * Sync bit in bit 7, Header Code in bits 6-5, User Data bit in bit 4,
     * EDC/ECC bit in bit 3, Erro field in bits 2-1
     */
    uint8_t bitmask;
    uint8_t subchannels;  /** Subchannel selection bits in bits 2-0 */
    uint8_t control;      /** Control data */
};

int read_raw_sector_unportable(cueify_device_private *d, uint32_t lba,
			       uint8_t *buffer) {
    char link_path[1024];
    struct cam_device *camdev;
    union ccb *ccb;
    struct ccb_scsiio *csio;
    struct scsi_read_cd *scsi_cmd;
    int result;

    /* cam_open_device does not resolve symlinks. */
    memset(link_path, 0, sizeof(link_path));
    result = readlink(d->path, link_path, sizeof(link_path) - 1);
    if (result < 0 && errno != EINVAL) {
	return 0;
    } else if (result < 0) {  /* errno == EINVAL */
	camdev = cam_open_device(d->path, O_RDWR);
    } else {
	camdev = cam_open_device(link_path, O_RDWR);
    }

    if (camdev == NULL) {
	return 0;
    }

    ccb = cam_getccb(camdev);
    if (ccb == NULL) {
	cam_close_device(camdev);
	return 0;
    }

    csio = &ccb->csio;
    cam_fill_csio(csio,
		  /* retries */ 4,
		  /* cbfcnp */ NULL,
		  /* flags */ CAM_DIR_IN,
		  /* tag_action */ MSG_SIMPLE_Q_TAG,
		  /* data_ptr */ (unsigned char *)buffer,
		  /* dxfer_len */ 2352,
		  /* sense_len */ SSD_FULL_SIZE,
		  sizeof(struct scsi_read_cd),
		  /* timeout */ 50000);

    scsi_cmd = (struct scsi_read_cd *)&csio->cdb_io.cdb_bytes;
    bzero(scsi_cmd, sizeof(*scsi_cmd));

    scsi_cmd->address[0] = (lba >> 24);
    scsi_cmd->address[1] = (lba >> 16) & 0xFF;
    scsi_cmd->address[2] = (lba >> 8) & 0xFF;
    scsi_cmd->address[3] = lba & 0xFF;

    scsi_cmd->length[0] = 0;
    scsi_cmd->length[1] = 0;
    scsi_cmd->length[2] = 1;

    scsi_cmd->bitmask = 0xF8;  /* Read Sync bit, all headers, User Data, and ECC */

    scsi_cmd->op_code = READ_CD;

    result = cam_send_ccb(camdev, ccb);
    if (result < 0) {
	cam_freeccb(ccb);
	cam_close_device(camdev);
	return 0;
    }

    cam_freeccb(ccb);
    cam_close_device(camdev);

    return 1;
}  /* read_raw_sector_unportable */
