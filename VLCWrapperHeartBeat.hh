#ifndef CLICK_VLCWrapperHeartBeat__HH
#define CLICK_VLCWrapperHeartBeat__HH

#include <click/element.hh>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/udp.h>
#include "vlc_proto.hh"

CLICK_DECLS

/*
=c

VLCWrapperHeartBeat(TICK)

=s

Sends a broadcast for every tick to the connected Interface (assuming WiFi)

=d

Test to send a broadcast packet to the wifi

*/
class VLCWrapperHeartBeat : public Element {

	unsigned int _tick_nr;
	int _DBlevel;
	int _PORT;

	int headroom;

	public:
		VLCWrapperHeartBeat();
		~VLCWrapperHeartBeat();

		const char *class_name() const { return "VLCWrapperHeartBeat"; }
		const char *port_count() const { return "1/1"; }
		const char *processing() const { return PUSH; }

		Packet *simple_action(Packet *p);
		int initialize(ErrorHandler *);
		int configure(Vector<String> &conf, ErrorHandler *errh);
};

CLICK_ENDDECLS

#endif
