#ifndef OFETHERENCAP__HH
#define OFETHERENCAP__HH 1

#include <click/config.h>
#include <click/element.hh>
#include <click/args.hh>
#include <click/etheraddress.hh>
#include <clicknet/ether.h>

#include "../Utilities/OFFunctions.hh"

#include <cstring>

CLICK_DECLS

/*
=c

OFEtherEncap()

=s

Encapsule IP Packet in EthernetII Packet.

=d

---

*/
class OFEtherEncap : public Element
{

	EtherAddress *etherAddrSrc;
	EtherAddress *etherAddrDst;

public:
	OFEtherEncap();
	~OFEtherEncap();

	const char *class_name() const { return "OFEtherEncap"; }
	const char *port_count() const { return "1/1"; }
	const char *processing() const { return AGNOSTIC; }

	virtual void cleanup(CleanupStage) CLICK_COLD;

	Packet *simple_action(Packet *p);
	int configure(Vector<String> &conf, ErrorHandler *errh);
};

CLICK_ENDDECLS

#endif
