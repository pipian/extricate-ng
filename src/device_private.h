/* device_private.h - Private optical disc device API
 *
 * Copyright (c) 2011, 2012 Ian Jacobi <pipian@pipian.com>
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

#ifndef _EXTRICATE_DEVICE_PRIVATE_H
#define _EXTRICATE_DEVICE_PRIVATE_H

/** OS-specific device handle types. */
#ifdef _WIN32
#define device_handle HANDLE  /* HANDLE to an OpenVolume */
#elif defined(__unix__) || defined(__APPLE__)
#define device_handle int     /* file descriptor */
#endif

/** Internal version of the cueify_device structure. */
typedef struct {
    device_handle handle;  /** OS-specific device handle */
    char *path;  /** OS-specific identifier used to open the handle. */
} cueify_device_private;


/**
 * Read a raw 2352-byte sector of data from a data track on an optical disc.
 *
 * @param d the cueify_device_private representing the device from
 *          which to read the sector
 * @param lba the LBA offset of the sector to read
 * @param buffer a 2352-byte buffer into which the sector should be read
 * @return 1 if successful, otherwise 0
 */
int read_raw_sector_unportable(cueify_device_private *d, uint32_t lba,
			       uint8_t *buffer);

#endif  /* _CUEIFY_DEVICE_PRIVATE_H */
