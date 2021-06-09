#ifndef CLICK_OFVLCQUEUE__HH
#define CLICK_OFVLCQUEUE__HH 1

#include "../Utilities/OFPaket.hh"


#include <click/config.h>
#include <click/element.hh>
#include <click/args.hh>

#include <iostream>
#include <string>
#include <queue>

CLICK_DECLS

/*
=c

OFVLCQueue(DB,AD)

=s

TODO:
Initializes and maintains initilization of a connected VLC-module.
Sends the initializatione message with TICK period.
DB gives debug level and number of printed debug messages
AD is the local address.
The packet creates a CamKin-Parameter-Update-Message and does not require the VLC-Wrapper.
However, the CanKinChecksumAdder is still needed.

=d

---

*/

class OFVLCQueue : public Element
{
private:
    bool _vlcReady;
    std::queue<OF::Packet *> _queue;
    int _DBlevel;
    uint32_t _queueMaxSize;

    enum inPort {
        ToQueue = 0x00,
        ACKFromVLC = 0x01,
    };

public:
    OFVLCQueue();
    ~OFVLCQueue();

    const char *class_name() const { return "OFVLCQueue"; }
    const char *port_count() const { return "2/1"; }
    const char *processing() const { return "hh/a"; }
    const char *flow_code() { return "x/x"; }
    virtual void cleanup(CleanupStage) CLICK_COLD;

    int initialize(ErrorHandler *errh);
    int configure(Vector<String> &conf, ErrorHandler *errh);
    void push(int inPort, Packet *p);

    Packet *pull();
};

CLICK_ENDDECLS

#endif