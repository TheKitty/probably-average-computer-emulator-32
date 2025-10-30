#include <cstdio>

#include "DiskIO.h"

uint32_t FileATAIO::getNumSectors(int unit)
{
    if(unit >= maxDrives)
        return false;

    return numSectors[unit];
}

bool FileATAIO::isATAPI(int unit)
{
    if(unit >= maxDrives)
        return false;

    return isCD[unit];
}

bool FileATAIO::read(int unit, uint8_t *buf, uint32_t lba)
{
    if(unit >= maxDrives)
        return false;

    f_lseek(&file[unit], lba * 512);

    UINT read = 0;
    auto res = f_read(&file[unit], buf, 512, &read);

    return res == FR_OK && read == 512;
}

bool FileATAIO::write(int unit, const uint8_t *buf, uint32_t lba)
{
    if(unit >= maxDrives)
        return false;

    f_lseek(&file[unit], lba * 512);

    UINT written = 0;
    auto res = f_write(&file[unit], buf, 512, &written);

    return res == FR_OK && written == 512;
}

void FileATAIO::openDisk(int unit, const char *path)
{
    if(unit >= maxDrives)
        return;

    auto res = f_open(&file[unit], path, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);

    // TODO: check ext (also handle sector size in read)
    isCD[unit] = false;

    if(res != FR_OK)
        return;

    // get size
    int sectorSize = isCD[unit] ? 2048 : 512;

    numSectors[unit] = f_size(&file[unit]) / sectorSize;

    printf("Loaded ATA disk %i: %s (size: %lu)\n", unit, path, numSectors[unit] * sectorSize);

}