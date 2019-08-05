#include "learning_gem5/my_memobj/my_memobj.hh"
#include "debug/MyMemobj.hh"

MyMemobj::MyMemobj(MyMemobjParams *params) :
    MemObject(params), instPort(params->name + ".inst_port", this), dataPort(params->name + ".data_port", this), memPort(params->name + ".mem_side_port", this),
    blocked(false)
{}

Port &
MyMemobj::getPort(const std::string &if_name, PortID idx)
{
    panic_if(idx != InvalidPortID, "This object does not support vector ports");

    // Name from python object
    if (if_name == "mem_side_port" ) {
        return memPort;
    }else if (if_name == "inst_port" ) {
        return instPort;
    }else if (if_name == "data_port" ) {
        return dataPort;
    }else {
        return MemObject::getPort(if_name, idx); // pass it to super class
    }
}

void MyMemobj::CPUSidePort::sendPacket(PacketPtr pkt) 
{
        panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

        if(!sendTimingResp(pkt)) { // If we cannot send the packet accross the port, store it for later
            blockedPacket = pkt;
        }
}

AddrRangeList
MyMemobj::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

void MyMemobj::CPUSidePort::trySendRetry()
{
    if (needRetry && blockedPacket == nullptr) { // Only send a retry if the port is now completely free
        needRetry = false;
        DPRINTF(MyMemobj, "Sending retry req for %d\n", id);
        sendRetryReq();
    }
}

void MyMemobj::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    return owner->handleFunctional(pkt);
}

bool MyMemobj::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    if(!owner->handleRequest(pkt)) {
        needRetry = true;
        return false;
    }else {
        return true;
    }
}

void MyMemobj::CPUSidePort::recvRespRetry()
{
    assert(blockedPacket != nullptr); // We should have a blocked packet if this function is called.

    PacketPtr pkt = blockedPacket; // Grab the blocked packet.
    blockedPacket = nullptr;

    sendPacket(pkt); // Try to resend it. It's possible that it fails again.
}

void MyMemobj::MemSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    if (!sendTimingReq(pkt)) { // If we can't send the packet across the port, store it for later.
        blockedPacket = pkt;
    }
}

bool MyMemobj::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
}

void MyMemobj::MemSidePort::recvReqRetry()
{
    assert(blockedPacket != nullptr); // We should have a blocked packet if this function is called.

    PacketPtr pkt = blockedPacket; // Grab the blocked packet.
    blockedPacket = nullptr;

    sendPacket(pkt); // Try to resend it. It's possible that it fails again.
}

void
MyMemobj::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

bool MyMemobj::handleRequest(PacketPtr pkt) 
{
    if (blocked) {
        return false; // stall
    }

    DPRINTF(MyMemobj, "Got request for addr %#x\n", pkt->getAddr());

    blocked = true; // Blocked, waiting for response to this packet

    memPort.sendPacket(pkt);

    return true;
}

bool MyMemobj::handleResponse(PacketPtr pkt) 
{
    assert(blocked);
    DPRINTF(MyMemobj, "Got response for addr %#x\n", pkt->getAddr());

    blocked = false; // The packet is done 

    if(pkt->req->isInstFetch()) {
        instPort.sendPacket(pkt);
    }else {
        dataPort.sendPacket(pkt);
    }

    instPort.trySendRetry(); // if it needs to send a retry, it should do it
    dataPort.trySendRetry(); // if it needs to send a retry, it should do it

    return true;
}

void MyMemobj::handleFunctional(PacketPtr pkt)
{
    // Just pass this on to the memory side to handle for now.
    memPort.sendFunctional(pkt);
}

AddrRangeList
MyMemobj::getAddrRanges() const
{
    DPRINTF(MyMemobj, "Sending new ranges\n");
    // Just use the same ranges as whatever is on the memory side.
    return memPort.getAddrRanges();
}

void MyMemobj::sendRangeChange()
{
    instPort.sendRangeChange();
    dataPort.sendRangeChange();
}

MyMemobj*
MyMemobjParams::create()
{
    return new MyMemobj(this);
}
