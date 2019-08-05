#ifndef __LEARNING_GEM5_MYMEMOBJ_MYMEMOBJ_HH__
#define __LEARNING_GEM5_MYMEMOBJ_MYMEMOBJ_HH__

#include "mem/mem_object.hh"
#include "params/MyMemobj.hh"

class MyMemobj : public MemObject
{
    private:
        class CPUSidePort : public SlavePort
        {
            private:
                MyMemobj *owner;
                bool needRetry; // True if port needs to send a retry req
                PacketPtr blockedPacket; // If the packet that this port sent was blocked, it stores it here.

            public:
                CPUSidePort(const std::string& name, MyMemobj *owner) :
                    SlavePort(name, owner), owner(owner), needRetry(false), blockedPacket(nullptr)
                {}

                void sendPacket(PacketPtr pkt); // Send a packet across this port. Called by the owner and all the flow control is handled in this function
                AddrRangeList getAddrRanges() const override; // List of the non-overlapping address ranges the owner is responsible for. All slave ports
                                                              // must override this function and return a populated list with at least one item.
                void trySendRetry(); // Send a retry to the peer port only if it is needed. This is called from the MyMemobj whenever it is unblocked.

            protected:
                Tick recvAtomic(PacketPtr pkt) override { panic("recvAtomic is not implemented"); } // not necessary but had to override.
                void recvFunctional(PacketPtr pkt) override; // Receive functional packet from the master. Performs a debug access updating/reading the data in place.
                bool recvTimingReq(PacketPtr pkt) override; // Receive a timing request from master port
                void recvRespRetry() override; // Master calls this if sendTimingResp was unsuccessful because master was busy (Master calls this with sendRespRetry)
        };

        class MemSidePort : public MasterPort
        {
            private:
                MyMemobj *owner;
                PacketPtr blockedPacket; // If the packet that this port sent was blocked, it stores it here.

            public:
                MemSidePort(const std::string& name, MyMemobj *owner) :
                    MasterPort(name, owner), owner(owner), blockedPacket(nullptr)
                {}

                void sendPacket(PacketPtr pkt); // Send a packet across this port. Called by the owner and all the flow control is handled in this function.

            protected:
                bool recvTimingResp(PacketPtr pkt) override; // Receive timing response from the slave.
                void recvReqRetry() override; // Slave calls this if sendTimingReq was unsuccessfull because slave was busy (Slave calls this with sendReqRetry)
                void recvRangeChange() override; // Called to receive an address change from the peer slave port. The default implementation ignores the change.
        };

        bool handleRequest(PacketPtr pkt); // Handle the request from the CPU Side. True if we can handle the request this cycle, false if the requestor needs to retry later.
        bool handleResponse(PacketPtr pkt); // Handle the request from the Mem Side. True if we can handle the response this cycle, false if the responder needs to retry later.
        void handleFunctional(PacketPtr pkt); // Handle a packet functionally. Update the data on a write and get the data on a read.
        AddrRangeList getAddrRanges() const; // Return the address ranges this memory object is responsible for.
        void sendRangeChange(); // Tell the CPU Side to ask for our memory ranges.

        CPUSidePort instPort;
        CPUSidePort dataPort;
        MemSidePort memPort;
        bool blocked; // True if this is currently blocked waiting for a response.

    public:
        MyMemobj(MyMemobjParams *params);

        Port &getPort(const std::string& if_name, PortID idx=InvalidPortID) override; // Get a port with a given name and index.
};
#endif // __LEARNING_GEM5_MYMEMOBJ_MYMEMOBJ_HH__
