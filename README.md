# Reliable Transfer over UDP

A file transfer protocol built on UDP, adding the guarantees UDP does not
provide: selective repeat for reliability and Random Early Detection for queue
management.

## Requirements

A C++ compiler with C++14 and pthreads, and `make`.

## Building

```bash
make
```

Produces `build/transfer_server` and `build/transfer_client`.

## Running

```bash
./build/transfer_server      # reads server.txt for its configuration
./build/transfer_client      # reads client.txt
```

The client requests a file by name; the server sends it as numbered packets and
the client reassembles it.

## Selective repeat

UDP delivers datagrams without ordering, acknowledgement or retransmission, so
all three have to be built on top.

Under a stop-and-wait scheme the sender transmits one packet and waits for its
acknowledgement, which wastes the link for a full round trip per packet.
Go-back-N keeps a window in flight but retransmits the whole window after a
loss, including packets that arrived intact.

Selective repeat retransmits only what was actually lost. The receiver buffers
out-of-order packets rather than discarding them and acknowledges each one
individually, so a single loss costs one retransmission instead of a window's
worth. The price is buffering at both ends and per-packet acknowledgement state.

## Congestion control

The window is not fixed. It follows the additive-increase, multiplicative-decrease
rule that keeps a shared link from collapsing:

| phase | condition | window |
| --- | --- | --- |
| slow start | `cwnd` below the threshold of 10 | doubles on every acknowledged window |
| congestion avoidance | `cwnd` at or above the threshold | grows by one packet per window |
| congestion signalled | a drop is reported | halves, floor of one |
| timeout | no acknowledgement before `SR_TIMEOUT` | back to one, and the window is refilled |

Doubling and then adding is the point of the split. A connection that starts at
one packet and adds one per round trip takes a hundred round trips to reach a
hundred packets in flight, so slow start finds the rough capacity quickly; once
near it, additive increase probes gently rather than overshooting by a factor of
two every time.

Backing off by half on a drop and adding one on success is what makes several
flows sharing a link converge on an even split: the flow with the largest window
loses the most in absolute terms when everyone halves, and gains the same as
everyone else when they add.

A timeout is treated as worse news than a reported drop. A drop means packets
are still getting through and one was shed; a timeout means nothing came back at
all, so the window returns to one rather than halving.

## Random Early Detection

A queue that only drops when full punishes every flow at once, and they all back
off and recover together, so the link oscillates between congested and idle.

RED drops earlier and probabilistically. It tracks an exponentially weighted
average queue length and, once that average passes a minimum threshold, drops
arriving packets with a probability that rises as the queue grows. Because the
drops are spread out, flows are signalled to slow down at different moments
rather than all together.

The average is weighted rather than instantaneous on purpose: a queue that
briefly spikes and drains has not congested, and reacting to the spike would
throttle traffic that was never a problem.

## Project structure

```
Server/
    serverMain.cpp        entry point
    Server/               socket setup and the accept loop
    Sender/, Receiver/    the two directions of transfer
    SR/                   selective repeat sender and receiver
    Packet/               packet layout and handling
    File Handler/         reading and writing the transferred file
    RED/red.hpp           the queue management policy
    queue.h               the packet queue RED polices
    Utils/constants.h     shared limits
Client/                   the same structure on the receiving side
Makefile                  server and client targets
```

Both sides carry their own copy of the shared pieces, so each builds
independently.
