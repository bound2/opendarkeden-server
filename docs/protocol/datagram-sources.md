---
type: Rule
title: "Datagram sources"
description: "A datagram is read only from a sender its packet's link admits: a client's port check from anywhere on the gameserver, the loginserver's packets only from the configured loginserver, a gameserver's only from the gameservers the catalogue lists, a link the server does not receive from no one; a refused datagram is counted and dropped unread. An address check, not authentication."
tags: [protocol, security]
enforced_by: [datagram_admission_tests, datagram_gate_audit]
generated: { by: claude-code/claude-fable-5-1, at: 2026-11-02T03:30:00Z }
---

# Datagram sources

| Rule | Owner | What fails when it is broken |
|------|-------|------------------------------|
| A datagram is read only from a sender its packet's link admits: on the gameserver's socket `CGPortCheck` from anywhere, the `LG` packets only from the `LoginServerIP` its configuration names and the `GG` packets only from the gameservers the catalogue lists; on the loginserver's socket the `GL` packets and `GMServerInfo` only from those gameservers; `DatagramPeerIPs` adds to the server links; a link the server does not receive and every other id from no one. The gate peeks the id, so a refused datagram's body is never parsed and an admitted one reads as before | `datagram_admission_tests` (`tests/datagram_admission_test.cpp`) over `de::DatagramGate`, `admitsDatagram`, `datagramLinkOf` and the two peer builders in `src/server/DatagramAdmission.h` | an id `Datagram::isDatagram` admits with no link, a link admitted on a socket that does not receive it or from a sender outside its peers, a configured address that is no dotted quad accepted, a refused datagram not counted and reported, or an admitted one whose read position moved |
| Both datagram loops consult the gate before `Datagram::read` | `datagram_gate_audit` (`tests/tools/check_datagram_gate.sh`), since neither loop has a test of its own: each opens a database connection before it reads | a loop file whose read is not the first statement inside the gate's branch |

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
`GG*`, `None` for every other id), and admits the datagram only when its socket receives that
link and the sender's address is among the link's peers. A gameserver's socket receives the
client, loginserver and gameserver links; a loginserver's the gameserver link alone, so a `GG`
packet or a port check, which it has no handler for, is never parsed there. The peers are the
`LoginServerIP` of the gameserver's configuration and every address in
`de::serverContext().serverInfos()`, asked for once, on the first datagram of a server link, and
kept: the configuration and the catalogue are loaded before the workers start and never
reloaded. A refused datagram is counted (`refusedDatagrams()` on either manager), reported once
per 30 seconds with its id and sender on standard error or output and, on the gameserver, in
`LOGINSERVERMANAGER.log` (`de::RepeatedErrorReport`), and deleted unread; a datagram the gate
cannot judge, because the peers or the report threw, is refused. The datagram fuzz target has
no sender, so it reads every input as one from an admitted peer ([fuzzing](fuzzing.md)).

**`DatagramPeerIPs`.** Addresses are compared as the dotted quads `Datagram::getHost` answers,
so a catalogue row named by hostname matches no sender, and so does a server whose datagrams
arrive from an address neither the configuration nor the catalogue names: a NAT or a proxy
between the servers, or a published port, which Docker hairpins. Such a deployment lists those
addresses, comma-separated, under `DatagramPeerIPs` in the server's configuration; they join
the server links that socket receives, never the client's, and an entry that is no IPv4 dotted
quad stops the server at startup. The compose deployment is one: with a routable
`GameServerInfo.IP` (the address remote clients need, `docker/initdb-docker.sql`), the
loginserver's datagrams to the gameserver go to that address and its published port, and Docker
hands them back into the container from the compose network's gateway, which
`docker/docker-compose.yml` pins and `docker/conf/gameserver.conf` names; the gameserver's
datagrams go to the loginserver's `127.0.0.1`, which `docker/conf/loginserver.conf` names. That
reading of Docker's port publishing is from its documentation, not from a run here; a refused
handoff shows as the refusal line in the compose log, and the remedy is the key.

**An address check, not authentication.** UDP carries no proof of the source address the
socket reports, and the packets that do damage need no reply: a sender on the network path who
writes a listed peer's address into the source field has `GGCommand` run at `Permission::God`,
`LGKickCharacter` set the kick flags, `LGIncomingConnectionOK` and `LGIncomingConnectionError`
mark or retire the named account, `GGGuildChat` and `GGServerChat` deliver text under any name,
`GLKickVerify` let a waiting login through, and `GMServerInfo` set a group's displayed
population. The compose deployment's peers are loopback and the pinned gateway, which a sender
outside the host cannot carry (the kernel refuses a loopback source arriving on another
interface, and Docker hands the container the real external source), so there the gate holds
against the outside; with LAN peers, as in the default `conf/gameserver.conf`, a sender on the
segment or the path is not stopped. Every process on a listed host is a peer: an address is not
a privilege boundary. An address listed for the gameserver's loginserver link is also what
`LGIncomingConnectionOKHandler` hands reconnecting clients as the login address. What closes the
residual is the owner's call ([next work plan](../reviews/NEXT.md), *Blocked or with the owner*):
binding the loginserver's socket, which no client uses, to a peer-facing or loopback address,
ingress filtering, or a shared secret on the server links, a wire change shipped with the
client. The [fix log](../reviews/FIXES.md) records it as *A forged source address passes the
datagram gate*.

**Left open.** The port check is admitted from anywhere by design; its handler refuses a name
character creation would refuse or that no statement may carry, and the repository escapes the
one it records (`gameserver/PortCheck.h`, task 16 of the next work plan, done), so what any
sender can still do there is name a recorded character and overwrite the address and port its
`UserIPInfo` row holds, which nothing reads back today, or add a row per distinct name, ten
characters each (the fix log's *Any sender can set the address and port recorded for any
character*; task 16 (a) under *Blocked or with the owner*). The sockets still bind
every address, and whether the compose file keeps `9997/udp` published is the owner's call: a
native client sends its port check there, and with a routable catalogue address the handoff
hairpins through it. The per-datagram lines a listed peer can still grow, `ggCommand.txt`
(`GGCommandHandler`) and `GuildMissing.log` (`GGGuildChatHandler`), are one line each and
unrotated, as the gameserver loop's console lines on a protocol failure are.
