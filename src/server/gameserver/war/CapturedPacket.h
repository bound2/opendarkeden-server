//////////////////////////////////////////////////////////////////////////////
// Filename    : CapturedPacket.h
// Description : a packet's body, written once and sent again to many players.
//
//               A broadcast to zones another thread owns is posted to each
//               zone's group (WarZoneWork.h, postBroadcast), while the packet
//               the caller built lives on the caller's stack and may own heap
//               objects a copy would share. So its body is written here, on
//               the calling thread, into a stream of its own, and those bytes
//               are what the zone threads receive, each group's command
//               holding its own reference to them. Nothing in a capture
//               changes after it is made, so the groups read it at once.
//
//               A capture is sent like the packet it was taken from: each
//               player's SocketOutputStream::writePacket frames it -- the
//               packet id, the measured size and that stream's own sequence
//               byte -- and the stream's encryption, applied when the stream
//               is flushed, covers its body like any other. A player receives
//               exactly the bytes the original would have produced on his
//               stream.
//
//               A body that depends on the stream it is written to cannot be
//               captured. The packets that encrypt their own fields read the
//               stream's encrypt code and assert that the stream is a
//               SocketEncryptOutputStream; the capture writes into a plain
//               SocketOutputStream, so such a packet fails its assertion here
//               rather than carrying one player's bytes to every player.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CAPTURED_PACKET_H__
#define __CAPTURED_PACKET_H__

#include <memory>
#include <string>

#include "Exception.h"
#include "Packet.h"
#include "SocketOutputStream.h"

namespace de::war {

class CapturedPacket : public Packet {
public:
    // Writes packet's body now. The packet is not referenced afterwards.
    explicit CapturedPacket(const Packet& packet)
        : m_PacketID(packet.getPacketID()), m_Name(packet.getPacketName()), m_Text(packet.toString()) {
        // One byte more than the body: a stream keeps one byte free, and a
        // buffer that never grows keeps the bytes contiguous from the start.
        SocketOutputStream body(NULL, (uint)packet.getPacketSize() + 1);
        packet.write(body);
        m_pBody = std::make_shared<const std::string>(body.getBuffer(), body.length());
    }

    // A capture is only ever sent.
    void read(SocketInputStream&) override {
        throw UnsupportedError("a captured packet is not read");
    }

    void write(SocketOutputStream& oStream) const override {
        oStream.write(m_pBody->data(), (uint)m_pBody->size());
    }

    PacketID_t getPacketID() const override {
        return m_PacketID;
    }

    PacketSize_t getPacketSize() const override {
        return (PacketSize_t)m_pBody->size();
    }

    string getPacketName() const override {
        return m_Name;
    }

    // The original's debug string, for the send log.
    string toString() const override {
        return m_Text;
    }

private:
    PacketID_t m_PacketID;
    std::string m_Name;
    std::string m_Text;
    std::shared_ptr<const std::string> m_pBody;
};

} // namespace de::war

#endif // __CAPTURED_PACKET_H__
