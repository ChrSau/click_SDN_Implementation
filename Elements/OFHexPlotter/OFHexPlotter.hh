#ifndef OFHEXPLOTTER__HH
#define OFHEXPLOTTER__HH 1


#include <click/config.h>
#include <click/element.hh>
#include <click/args.hh>

#include "../Utilities/OFFunctions.hh"

CLICK_DECLS

/*
=c

OFHexPlotter(Name, NR)

=s

Plots incoming messages in a "human-readable" hexadecimal
format. Also plots an initial line containing the over-all
length of the message.

=d

---

*/
class OFHexPlotter : public Element {

	int Plotter_nr;
	String Name;

	public:
		OFHexPlotter();
		~OFHexPlotter();

		const char *class_name() const { return "OFHexPlotter"; }
		const char *port_count() const { return "1/1"; }
		const char *processing() const { return AGNOSTIC; }

		Packet *simple_action(Packet *p);
		int configure(Vector<String> &conf, ErrorHandler *errh);
};

CLICK_ENDDECLS

#endif
