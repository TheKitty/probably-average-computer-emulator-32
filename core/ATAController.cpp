#include <cstdio>

#include "ATAController.h"

enum ATAStatus
{
    Status_ERR  = 1 << 0, // error
    Status_DRQ  = 1 << 3, // data request
    Status_DF   = 1 << 5, // device fault
    Status_DRDY = 1 << 6, // device ready
    Status_BSY  = 1 << 7, // busy
};

ATAController::ATAController(System &sys)
{
    // 1F0-1F7 (primary, 170-177 for secondary)
    sys.addIODevice(0x3F8, 0x1F0, 0, this);
    // 3F6-3F7 (primary, 376-377 for secondary)
    sys.addIODevice(0x3FE, 0x3F6, 0, this);
}

uint8_t ATAController::read(uint16_t addr)
{
    switch(addr & ~(1 << 7))
    {
        /*
        case 0x170: // data
        case 0x171: // error

        case 0x376: // alt status
        case 0x377: // device address
        */
        case 0x172: // sector count
            return sectorCount;
        case 0x173: // lba low/sector
            return lbaLowSector;
        case 0x174: // lba mid/cylinder low
            return lbaMidCylinderLow;
        case 0x175: // lba high/cylinder high
            return lbaHighCylinderHigh;
        case 0x176: // device/head
            return deviceHead;
        case 0x177: // status
            // clears irq
            return status;
        default:
            printf("ATA R %04X\n", addr);
            return 0xFF;
    }
}

void ATAController::write(uint16_t addr, uint8_t data)
{
    switch(addr & ~(1 << 7))
    {
        /*
        case 0x170: // data
        
        case 0x376: // device control
        */
        case 0x171: // features
            features = data;
            break;
        case 0x172: // sector count
            sectorCount = data;
            break;
        case 0x173: // lba low/sector
            lbaLowSector = data;
            break;
        case 0x174: // lba mid/cylinder low
            lbaMidCylinderLow = data;
            break;
        case 0x175: // lba high/cylinder high
            lbaHighCylinderHigh = data;
            break;
        case 0x176: // device/head
            deviceHead = data;
            status |= Status_DRDY; // for non-ATAPI
            break;
        case 0x177: // command
        {
            int dev = (deviceHead >> 4) & 1;
            switch(data)
            {
                default:
                    printf("ATA command %02X (dev %i)\n", data, dev);
                    status |= Status_ERR;
            }
            break;
        }
        default:
            printf("ATA W %04X = %02X\n", addr, data);
    }
}