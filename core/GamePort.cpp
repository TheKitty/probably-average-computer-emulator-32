#include "GamePort.h"

GamePort::GamePort(System &sys) : sys(sys)
{
    sys.addIODevice(0x3FF, 0x201, 0, this);
}

uint8_t GamePort::read(uint16_t addr)
{
    uint8_t ret = ~buttonState << 4;

    auto elapsed = sys.getCycleCount() - timerStartCycle;

    // figure out which axes are still active
    for(int i = 0; i < 4; i++)
    {
        // ~24-1124 us, 1 us is ~14 cycles
        unsigned axisTime = (24 + axisState[i] * 1100) * 14;

        if(elapsed < axisTime)
            ret |= 1 << i;
    }

    return ret;
}

void GamePort::write(uint16_t addr, uint8_t data)
{
    // start timer
    timerStartCycle = sys.getCycleCount();
}

void GamePort::setButton(int index, bool pressed)
{
    if(index >= 4)
        return;

    int bit = 1 << index;

    if(pressed)
        buttonState |= bit;
    else
        buttonState &= ~bit;
}

void GamePort::setAxis(int index, float value)
{
    if(index >= 4)
        return;

    axisState[index] = value;
}