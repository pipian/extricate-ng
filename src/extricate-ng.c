/* extricate-ng.c - A utility to extract ISOs from data CDs.
 *
 * Copyright (c) 2012 Ian Jacobi
 * A spiritual descendent of Extricate 0.2.0
 * Copyright (c) 2005 Thomas Schumm <phong@phong.org>
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cueify/types.h>
#include <cueify/constants.h>
#include <cueify/error.h>
#include <cueify/device.h>
#include <cueify/full_toc.h>
#include <cueify/track_data.h>
#include "device_private.h"

/**
 * Convert an MSF time address to an LBA absolute address.
 *
 * @param msf the MSF address to convert
 * @return the LBA address
 */
static inline uint32_t msf_to_lba(cueify_msf_t msf) {
    uint32_t lba = 0;

    lba += msf.frm;
    lba += msf.sec * 75;
    lba += msf.min * 60 * 75;
    lba -= 150;  /* Lead-in */

    return lba;
}  /* msf_to_lba */



/**
 * Read a sector of data from a data track on an optical disc.
 *
 * @param d the cueify_device object representing the device from
 *          which to read the sector
 * @param lba the LBA offset of the sector to read
 * @param buffer a 2352-byte buffer into which the sector should be read
 * @return the number of data bytes in the sector
 */
size_t read_sector(cueify_device *d, uint32_t lba, uint8_t *buffer) {
    /*
     * Cheat and peek under the libcueify API to grab the underlying
     * handle of the device.
     */
    cueify_device_private *device = (cueify_device_private *)d;

    /*
     * Read the raw sector unportably (since we don't expose this API
     * in libcueify).
     */
    if (read_raw_sector_unportable(device, lba, buffer) == 0) {
	return 0;
    }

    /* Extract only the data, as given by the mode bits in the data header. */
    uint8_t data_mode = buffer[0xF];
    switch (data_mode) {
    case 0x01:
	/* Mode 1 */
	/* 16 byte header, followed by 2048 data bytes. */
	memmove(buffer, buffer + 16, 2048);
	return 2048;
    case 0x02:
	/* Mode 2 */
	/* 16 byte header, followed by (up to) 2336 data bytes. */
	memmove(buffer, buffer + 16, 2336);
	return 2336;
    default:
	/* Unrecognized (CDDA?) */
	return 0;
    }
}


/**
 * Extract all data tracks as separate ISO/BIN files named
 * "prefixXX.iso" or "prefixXX.bin" where XX is the number of the data
 * track being ripped (e.g. first data track is 01, second is 02,
 * etc.)  ISO files are created when the track is Mode 1 or when the
 * track is Mode 2 and all frames in the track are Form 1.  Otherwise,
 * a BIN file containing frames of size 2336 is created.
 *
 * @param device the name of the device to extricate data tracks from
 * @param prefix the prefix of the files to create
 * @return 0 if successful, otherwise 1
 */
int extricate(const char *device, const char *prefix) {
    cueify_device *d = cueify_device_new();
    cueify_full_toc *t;
    int data_track = 1;

    if (d == NULL) {
	fprintf(stderr, "Failed to open device\n");
	return 1;
    }
    if (cueify_device_open(d, device) != CUEIFY_OK) {
	fprintf(stderr, "Failed to open device\n");
	cueify_device_free(d);
	return 1;
    }

    /* Find any data tracks. */
    t = cueify_full_toc_new();
    if (t == NULL) {
	fprintf(stderr, "Failed to read device TOC\n");
	cueify_device_free(d);
	return 1;
    }
    if (cueify_device_read_full_toc(d, t) != CUEIFY_OK) {
	fprintf(stderr, "Failed to read device TOC\n");
	cueify_full_toc_free(t);
	cueify_device_free(d);
	return 1;
    }

    for (uint8_t i = cueify_full_toc_get_first_track(t);
	 i <= cueify_full_toc_get_last_track(t);
	 i++) {
	/* If it's a data track, then... */
	if (cueify_full_toc_get_track_control_flags(t, i) &
	    CUEIFY_TOC_TRACK_IS_DATA) {
	    /* Open the output file. */
	    char *filename = calloc(1, strlen(prefix) + 7);
	    FILE *iso_fp = NULL, *bin_fp = NULL;
	    if (filename == NULL) {
		fprintf(stderr, "Unable to write track %02d\n", i);
		continue;
	    }

	    sprintf(filename, "%s%02d.iso", prefix, data_track);
	    iso_fp = fopen(filename, "wb");
	    if (iso_fp == NULL) {
		fprintf(stderr, "Unable to write track %02d\n", i);
		free(filename);
		continue;
	    }

	    int data_mode = cueify_device_read_data_mode(d, i);
	    if (data_mode == CUEIFY_DATA_MODE_MODE_2) {
		sprintf(filename, "%s%02d.bin", prefix, data_track);
		bin_fp = fopen(filename, "wb");
		if (bin_fp == NULL) {
		    fprintf(stderr, "Unable to write track %02d\n", i);
		    fclose(iso_fp);
		    free(filename);
		    continue;
		}
	    }

	    uint32_t offset =
		msf_to_lba(cueify_full_toc_get_track_address(t, i));
	    uint8_t buffer[2352];
	    uint16_t size;
	    for (uint32_t j = 0;
		 // No 150 sector adjustment, since this is duration
		 j < msf_to_lba(cueify_full_toc_get_track_length(t, i)) + 150;
		 j++) {
		/* Read each frame from the track. */
		size = read_sector(d, offset + j, buffer);

		uint8_t submode = buffer[0x2];
		switch (data_mode) {
		case CUEIFY_DATA_MODE_MODE_1:
		    /* Mode 1 */
		    /* Write the frame out to the ISO file. */
		    if (fwrite(buffer, 1, size, iso_fp) != size) {
			fprintf(stderr, "Unable to write track %02d\n", i);
			goto end_sector_loop;
		    }
		    break;
		case CUEIFY_DATA_MODE_MODE_2:
		    /* Mode 2 */
		    switch (submode >> 5 & 0x1) {
		    case 0x00:
			/* Form 1 */
			/* 16 + 8 byte header + subheader, followed by
			 * 2048 data bytes */

			/* Could be an ISO hiding in Mode 2 clothing. */
			if (iso_fp != NULL &&
			    fwrite(buffer + 8, 1, 2048, iso_fp) != 2048) {
			    fprintf(stderr, "Unable to write track %02d\n", i);
			    goto end_sector_loop;
			}

			/* Also write the BIN file. */
			if (fwrite(buffer, 1, size, bin_fp) != size) {
			    fprintf(stderr, "Unable to write track %02d\n", i);
			    goto end_sector_loop;
			}
			break;
		    case 0x01:
			/* Form 2 */
		    default:
			/* 16 + 8 byte header + subheader, followed by
			 * 2324 data bytes (and 4 extra bytes at the end) */

			/* Close the ISO file and delete it, since we
			 * found a non-Form 1 frame. */
			if (iso_fp != NULL) {
			    fclose(iso_fp);
			    sprintf(filename, "%s%02d.iso",
				    prefix, data_track);
			    unlink(filename);
			    iso_fp = NULL;
			}

			/* Also write the BIN file. */
			if (fwrite(buffer, 1, size, bin_fp) != size) {
			    fprintf(stderr, "Unable to write track %02d\n", i);
			    goto end_sector_loop;
			}
			break;
		    }
		    break;
		default:
		    break;
		}
	    }
	end_sector_loop:
	    if (iso_fp != NULL) {
		fclose(iso_fp);
	    }
	    if (bin_fp != NULL) {
		fclose(bin_fp);
		/* The ISO file is sufficient, since we found no
		 * non-Form 1 frames. */
		if (iso_fp != NULL) {
		    sprintf(filename, "%s%02d.bin", prefix, data_track);
		    unlink(filename);
		}
	    }
	    free(filename);
	    data_track++;
	}
    }

    cueify_full_toc_free(t);
    cueify_device_free(d);
    return 0;
}  /* extricate */

int main(int argc, char *argv[]) {
    char *device, *prefix;

    if (argc > 3) {
	printf("Usage: extricate-ng [DEVICE [PREFIX]]\n");
	return 0;
    }

    if (argc > 1) {
	device = argv[1];
    } else {
	device = NULL;
    }

    if (argc == 3) {
	prefix = argv[2];
    } else {
	prefix = "disc";
    }

    return extricate(device, prefix);
}  /* main */
