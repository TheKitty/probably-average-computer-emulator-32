#include <cstdio>

#include "pico/multicore.h"

#include "Floppy.h"

#include "DiskIO.h"

bool FileFloppyIO::isPresent(int unit)
{
    if(unit >= maxDrives)
        return false;

    return sectorsPerTrack[unit] != 0;
}

uint32_t FileFloppyIO::getLBA(int unit, uint8_t cylinder, uint8_t head, uint8_t sector)
{
    int heads = doubleSided[unit] ? 2 : 1;
    return ((cylinder * heads + head) * sectorsPerTrack[unit]) + sector - 1;
}

bool FileFloppyIO::read(FloppyController *controller, int unit, uint8_t *buf, uint32_t lba)
{
    if(unit >= maxDrives || !sectorsPerTrack[unit])
        return false;

    if(curAccessController)
    {
        printf("Floppy IO already in progress! (%c %u -> R %u)\n", curAccessWrite ? 'W' : 'R', curAccessLBA, lba);
        return false;
    }

    curAccessController = controller;
    curAccessDevice = unit;
    curAccessBuf = buf;
    curAccessLBA = lba;
    curAccessWrite = false;
    multicore_fifo_push_blocking(1);

    return true;
}

bool FileFloppyIO::write(FloppyController *controller, int unit, const uint8_t *buf, uint32_t lba)
{
    if(unit >= maxDrives || !sectorsPerTrack[unit])
        return false;

    if(curAccessController)
    {
        printf("Floppy IO already in progress! (%c %u -> W %u)\n", curAccessWrite ? 'W' : 'R', curAccessLBA, lba);
        return false;
    }

    curAccessController = controller;
    curAccessDevice = unit;
    curAccessBuf = const_cast<uint8_t *>(buf);
    curAccessLBA = lba;
    curAccessWrite = true;
    multicore_fifo_push_blocking(1);

    return true;
}

void FileFloppyIO::openDisk(int unit, const char *path)
{
    if(unit >= maxDrives)
        return;

    auto res = f_open(&file[unit], path, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);

    if(res != FR_OK)
    {
        sectorsPerTrack[unit] = 0;
        return;
    }

    guessFloppyImageGeometry(f_size(&file[unit]), doubleSided[unit], sectorsPerTrack[unit]);

    printf("Loaded floppy disk %i: %s (%i heads %i sectors/track)\n", unit, path, doubleSided[unit] ? 2 : 1, sectorsPerTrack[unit]);
}

void FileFloppyIO::doCore0IO()
{
    f_lseek(&file[curAccessDevice], curAccessLBA * 512);

    UINT accessed;
    FRESULT res;
    if(curAccessWrite)
        res = f_write(&file[curAccessDevice], curAccessBuf, 512, &accessed);
    else
        res = f_read(&file[curAccessDevice], curAccessBuf, 512, &accessed);

    curAccessSuccess = res == FR_OK && accessed == 512;

    multicore_fifo_push_blocking(1);
}

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

bool FileATAIO::read(ATAController *controller, int unit, uint8_t *buf, uint32_t lba)
{
    if(unit >= maxDrives)
        return false;

    if(lba >= numSectors[unit])
        return false;

    if(curAccessController)
    {
        printf("ATA IO already in progress! (%c %u -> R %u)\n", curAccessWrite ? 'W' : 'R', curAccessLBA, lba);
        return false;
    }

    curAccessController = controller;
    curAccessDevice = unit;
    curAccessBuf = buf;
    curAccessLBA = lba;
    curAccessWrite = false;
    multicore_fifo_push_blocking(2);

    return true;
}

bool FileATAIO::write(ATAController *controller, int unit, const uint8_t *buf, uint32_t lba)
{
    if(unit >= maxDrives)
        return false;

    if(lba >= numSectors[unit])
        return false;

    if(curAccessController)
    {
        printf("ATA IO already in progress! (%c %u -> W %u)\n", curAccessWrite ? 'W' : 'R', curAccessLBA, lba);
        return false;
    }

    curAccessController = controller;
    curAccessDevice = unit;
    curAccessBuf = const_cast<uint8_t *>(buf);
    curAccessLBA = lba;
    curAccessWrite = true;
    multicore_fifo_push_blocking(2);

    return true;
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

void FileATAIO::doCore0IO()
{
    f_lseek(&file[curAccessDevice], curAccessLBA * 512);

    UINT accessed;
    FRESULT res;
    if(curAccessWrite)
        res = f_write(&file[curAccessDevice], curAccessBuf, 512, &accessed);
    else
        res = f_read(&file[curAccessDevice], curAccessBuf, 512, &accessed);

    curAccessSuccess = res == FR_OK && accessed == 512;

    multicore_fifo_push_blocking(2);
}
