#pragma once
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>

class CPUTrace
{
public:
    void addEntry(uint32_t virtAddr, uint32_t physAddr, uint8_t opcode, bool op32, uint32_t regs[], uint32_t flags)
    {
#ifdef CPU_TRACE_SIZE
        TraceEntry te{virtAddr, physAddr, opcode, op32, {}, flags};
        memcpy(te.regs, regs, sizeof(te.regs));

        entries[entryIndex++] = te;
        if(entryIndex == std::size(entries))
            entryIndex = 0;
#endif
    }

    bool isEnabled() const
    {
#ifdef CPU_TRACE_SIZE
        return true;
#else
        return false;
#endif
    }

    void dump()
    {
#ifdef CPU_TRACE_SIZE
        auto pos = entryIndex;

        for(size_t i = 0; i < std::size(entries); i++)
        {
            auto &entry = entries[pos++];
            // wrap
            if(pos == std::size(entries))
                pos = 0;

            printf("%08X", entry.addr);
        
            // print physical addr if different
            if(entry.addr != entry.physAddr)
                printf("(%08X)", entry.physAddr);

            auto cr0 = entry.regs[16];

            printf(" (%i%c): %02X EAX %08X ECX %08X EDX %08X EBX %08X ESP %08X EBP %08X ESI %08X EDI %08X ES %04X CS %04X SS %04X DS %04X EFLAGS %08X\n",
                entry.code32 ? 32 : 16, (cr0 & 1) ? 'P' : 'R', entry.opcode0,
                entry.regs[0], entry.regs[1], entry.regs[2], entry.regs[3], entry.regs[4], entry.regs[5], entry.regs[6], entry.regs[7],
                entry.regs[9], entry.regs[10], entry.regs[11], entry.regs[12], entry.flags
            );
        }
#endif
    }

private:
    struct TraceEntry
    {
        uint32_t addr = ~0;
        uint32_t physAddr = ~0;
        uint8_t opcode0 = ~0;
        bool code32;
        uint32_t regs[20];
        uint32_t flags;
    };

#ifdef CPU_TRACE_SIZE
    TraceEntry entries[CPU_TRACE_SIZE];
    size_t entryIndex = 0;
#endif
};