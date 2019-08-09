/** @file
 * Declaration of a structure to insert faults to different levels of caches. 
 * It allows to insert stuck at 0 and stuck at 1 faults for permanent, intermittent 
 * and transient faults that were declared in an input file. In order to see an 
 * example input file, check input.txt in the top level directory of gem5 source code.
 */

#ifndef __MEM_CACHE_FAULT_INJECTOR_HH__
#define __MEM_CACHE_FAULT_INJECTOR_HH__

#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>

#include "params/FaultInjector.hh"
#include "sim/sim_object.hh"
#include "mem/packet.hh"
#include "mem/cache/base.hh"
#include "mem/cache/tags/base_set_assoc.hh"
#include "base/logging.hh"

class BaseTags;

struct CacheFault {
    int type; // Type of fault : Transient (0), Intermittent (1), Permanent (2)
    int blockAddr; // Address of the block that will be corrupted.
    int byteOffset; // Byte offset of the address from the beginning of block address.
    int bitOffset; // Determines which bit of the byte will be corrupted.
    int tickStart; // At which cycle the fault will be inserted.
    int tickEnd; // Only for intermittent faults.
    int stuckAt; // Stuck at 0 (0), Stuck at 1 (1)
    std::string cacheToBeInserted;
    uint8_t alteredByte; // Only for intermittent faults. Holds the bit value that was corrupted. Useful for restoring.
    int recovered;
    int inserted;
};

class FaultInjector : public SimObject
{
    private:
        /** Absolute path of input file that contains faults. */
        std::string inputPath;

        /** Vector that contains faults.  */
        std::vector<CacheFault> faults;

    public:
        FaultInjector(FaultInjectorParams *p);

        /** Reads all faults from the input file and populates faults vector. */
        void init(std::string owner);
        
        /** Helper functions to detect the type of fault. 
         * 
         * @param fault The fault to test whether it is transient, intermittent or permanent,
        */
        bool isTransient(CacheFault fault){ return fault.type == 0; }
        bool isIntermittent(CacheFault fault){ return fault.type == 1; }
        bool isPermanent(CacheFault fault){ return fault.type == 2; }
        
        /** Function to check whether packet's address range contains a faulty addresss.
         * 
         * @param fault The fault that provides block address and byte offset of the faulty address
         * @param pkt The packet that attempts to change the content of the block
         * @param blkSize Size of one block in the cache.
         */
        bool isFaultyAddress(CacheFault fault, PacketPtr pkt, unsigned blkSize) {
            int targetAddress = fault.blockAddr + fault.byteOffset;

            return targetAddress >= pkt->getAddr() && targetAddress <= pkt->getAddr() + pkt->getSize() - 1; 
        }

        /** Checks whether a fault is active. Useful for intermittent faults. This function
         * is meaningless for permanent and transient faults.
         * 
         * @param fault The fault to test whether it is active.
          */
        bool isFaultActive(CacheFault fault) { return fault.tickStart <= curTick() && fault.tickEnd >= curTick(); }
        bool isFaultDead(CacheFault fault) { return fault.tickEnd < curTick(); }

        /** Function to get faults. */
        std::vector<CacheFault>& getFaults() { return faults; }

        /** Flips a bit of the data according to stuck at policy. If it is stuck at 1 fault, it flips
         * specified bit to 1. If it is stuck at 0 fault, it flips specified bit to 0.
         * 
         * @param fault The fault that defines the stuck at policy, byte offset and bit offset.
         * @param pkt Pointer to the packet whose data will be corrupted.
         * @param blkSize Size of one block in the cache.
         */
        void flipBit(CacheFault fault, PacketPtr pkt, unsigned blkSize);

        /** Injects all active faults at specified tick. 
         * 
         * @param data Data that will be corrupted.
         * @param blockAddr Physical address of packet's block address. This will be compared with faults' physical addresses.
         * @param isRead Indicates whether we inject faults on a read. This is useful because, for example, there is no point of inserting
         * a transient fault on a write (I am not sure but this logic should probably be changed).  
        */
        void injectFaults(PacketPtr pkt, unsigned blkSize, bool isRead, std::string cacheType);
};

#endif // __MEM_CACHE_FAULT_INJECTOR_HH__
