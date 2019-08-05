#ifndef __LEARNING_GEM5_MYCACHE_MYCACHE_HH__
#define __LEARNING_GEM5_MYCACHE_MYCACHE_HH__

#include <unordered_map>

#include "mem/mem_object.hh"
#include "params/MyCache.hh"

class MyCache : public MemObject
{
    private:
        class CPUSidePort : public SlavePort
        {
            private:
                int id; // Since this is a vector port, we need id.
                MyCache *owner;
                bool needRetry; // True if port needs to send a retry req
                PacketPtr blockedPacket; // If the packet that this port sent was blocked, it stores it here.

            public:
                CPUSidePort(const std::string& name, int id, MyCache *owner) :
                    SlavePort(name, owner), id(id), owner(owner), needRetry(false), blockedPacket(nullptr)
                {}

                void sendPacket(PacketPtr pkt); // Send a packet across this port. Called by the owner and all the flow control is handled in this function
                AddrRangeList getAddrRanges() const override; // List of the non-overlapping address ranges the owner is responsible for. All slave ports
                                                              // must override this function and return a populated list with at least one item.
                void trySendRetry(); // Send a retry to the peer port only if it is needed. This is called from the MyCache whenever it is unblocked.
                bool recvTimingReq(PacketPtr pkt) override; // Receive a timing request from master port

            protected:
                Tick recvAtomic(PacketPtr pkt) override { panic("recvAtomic is not implemented"); } // not necessary but had to override.
                void recvFunctional(PacketPtr pkt) override; // Receive functional packet from the master. Performs a debug access updating/reading the data in place.
                void recvRespRetry() override; // Master calls this if sendTimingResp was unsuccessful because master was busy (Master calls this with sendRespRetry)
        };

        class MemSidePort : public MasterPort
        {
            private:
                MyCache *owner;
                PacketPtr blockedPacket; // If the packet that this port sent was blocked, it stores it here.

            public:
                MemSidePort(const std::string& name, MyCache *owner) :
                    MasterPort(name, owner), owner(owner), blockedPacket(nullptr)
                {}

                void sendPacket(PacketPtr pkt); // Send a packet across this port. Called by the owner and all the flow control is handled in this function.

            protected:
                bool recvTimingResp(PacketPtr pkt) override; // Receive timing response from the slave.
                void recvReqRetry() override; // Slave calls this if sendTimingReq was unsuccessfull because slave was busy (Slave calls this with sendReqRetry)
                void recvRangeChange() override; // Called to receive an address change from the peer slave port. The default implementation ignores the change.
        };

        void processEvent();
        bool handleRequest(PacketPtr pkt, int port_id); // Handle the request from the CPU Side. True if we can handle the request this cycle, false if the requestor needs to retry later.
        bool handleResponse(PacketPtr pkt); // Handle the request from the Mem Side. True if we can handle the response this cycle, false if the responder needs to retry later.
        void sendResponse(PacketPtr pkt); // This function assumes the pkt is already a response packet and forwards it to the correct port. This function also unblocks this object and cleans up the whole request.
        void handleFunctional(PacketPtr pkt); // Handle a packet functionally. Update the data on a write and get the data on a read.
        void accessTiming(PacketPtr pkt); // Access the cache for a timing access. This is called after the cache latency has already elapsed.
        bool accessFunctional(PacketPtr pkt); // This is where we update / read from the cache. This function is executed on both timing and functional accesses.
        void insert(PacketPtr pkt); // Insert a block into the cache. If there is no room left in the cache, then this function evicts a random entry.
        AddrRangeList getAddrRanges() const; // Return the address ranges this memory object is responsible for.
        void sendRangeChange(); // Tell the CPU Side to ask for our memory ranges.
        //assumes little endian
        void printBits(size_t const size, void const * const ptr);


        EventFunctionWrapper event;
        const Cycles latency; // Latency to check the cache. Number of cycles for both hit and miss
        const unsigned blockSize; // Block size for the cache.
        const unsigned capacity; // Number of blocks in the cache.

        bool goldenRun;
        std::string faultType; // Type of fault that is introduced.
        int faultTick;        // Tick number to insert fault
        int faultAddr;        // Address of fault
        int faultBit;         // Bit to change

        std::vector<CPUSidePort> cpuPorts;
        MemSidePort memPort;
        bool blocked; // True if this is currently blocked waiting for a response.
        PacketPtr originalPacket; // Packet that we are currently handling. Used for upgrading to larger cache line sizes.
        int waitingPortId; // The port to send the response when we recieve it back
        Tick missTime; // For tracking miss latency.
        std::unordered_map<Addr, uint8_t*> cacheStore; // Cache storage.Maps block addresses to data.

        // STATS
        Stats::Scalar hits;
        Stats::Scalar misses;
        Stats::Histogram missLatency;
        Stats::Formula hitRatio;

    public:
        MyCache(MyCacheParams *params);

        void startup();

        Port &getPort(const std::string& if_name, PortID idx=InvalidPortID) override; // Get a port with a given name and index.

        void regStats() override; // Initializes stats.
};
#endif // __LEARNING_GEM5_MYCACHE_MYCACHE_HH__
