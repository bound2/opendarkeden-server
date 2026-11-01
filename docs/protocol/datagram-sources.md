---
type: Rule
title: "Datagram sources"
description: "A datagram is read only from a sender its packet's link admits: a client's port check from anywhere, the loginserver's packets only from the configured loginserver, a gameserver's only from the gameservers the catalogue lists; a refused datagram is counted and dropped unread."
tags: [protocol, security]
enforced_by: [datagram_admission_tests]
---

# Datagram sources

| Rule | Owner | What fails when it is broken |
|------|-------|------------------------------|
| A datagram is read only from a sender its packet's link admits: `CGPortCheck` from anywhere, the `LG` packets only from the `LoginServerIP` the gameserver's configuration names, the `GL`, `GM` and `GG` packets only from the gameservers the catalogue lists, and no other id from anyone; the gate peeks the id, so a refused datagram's body is never parsed | `datagram_admission_tests` (`tests/datagram_admission_test.cpp`) over `de::DatagramGate`, `admitsDatagram`, `datagramLinkOf` and the two peer builders in `src/server/DatagramAdmission.h` | an id `Datagram::isDatagram` admits with no link, or a link admitted from a sender outside its peers, or a refused datagram that is not counted and reported |

**Why.** Both datagram sockets bind every address and authenticate nothing, and the compose
file publishes the gameserver's (`9997/udp`). What `Datagram::isDatagram` admits is more than
the login handoff: `GGCommand` runs a relayed message through the God-level relay commands
(`shutdown`, `set`, `kick` among them, `gm/RelayCommandRegistration.cpp`), and
`LGIncomingConnectionOKHandler` takes the datagram's source as the login address it hands a
reconnecting client. Before this rule anyone who could reach the port could do both.

**The gate.** `LoginServerManager::run` (the gameserver) and `GameServerManager::run` (the
loginserver) run every received datagram through a `de::DatagramGate` before `Datagram::read`.
The gate reads the packet id from the datagram's first two bytes, asks `datagramLinkOf` which
link the id rides (`Client` for the port check, `LoginToGame` for `LG*`, `GameToLogin` for
`GL*` and `GMServerInfo`, which `ConnectionInfoManager` sends the loginserver, `GameToGame` for
`GG*`, `None` for every other id), and admits the datagram only when the sender's address is
among that link's peers. The peers are asked for on every datagram, so a reloaded catalogue is
seen at once: a gameserver's are the `LoginServerIP` of its configuration and every address in
`de::serverContext().serverInfos()`; a loginserver's are the catalogue's addresses alone. A
refused datagram is counted (`refusedDatagrams()` on either manager) and reported once per 30
seconds with the id and the sender (`LOGINSERVERMANAGER.log` on the gameserver, the diagnostics
stream on the loginserver, through `de::RepeatedErrorReport`), and deleted unread. The datagram
fuzz target has no sender, so it reads every input as one from an admitted peer
([fuzzing](fuzzing.md)).

**`DatagramPeerIPs`.** Addresses are compared as the dotted quads `Datagram::getHost` answers,
so a peer named by hostname is refused, and so is a server whose datagrams arrive from an
address neither the configuration nor the catalogue names (a multi-homed host, a NAT between
the servers). Such a deployment lists those addresses, comma-separated, under
`DatagramPeerIPs` in the server's configuration; they are admitted for every server link,
never for a client's. The compose deployment needs none: its catalogue overlay names the
gameserver as `127.0.0.1`, the address its datagrams to the loginserver's `127.0.0.1` carry.

**What it does not decide.** The socket still binds every address, and whether the compose
file keeps `9997/udp` published (a native client sends its port check there on reconnect) is
the owner's call ([next work plan](../reviews/NEXT.md), task 11).
