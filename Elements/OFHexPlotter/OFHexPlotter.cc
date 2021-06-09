//our includes
#include "OFHexPlotter.hh"

CLICK_DECLS

OFHexPlotter::OFHexPlotter(){};
OFHexPlotter::~OFHexPlotter(){};

int OFHexPlotter::configure(Vector<String> &conf, ErrorHandler *errh)
{

	if (Args(conf, this, errh)
			.read_mp("Name", this->Name)
			.read("NR", Plotter_nr)
			.complete() < 0)
	{
		return -EXIT_FAILURE;
	}
	click_chatter("OFHexPlotter(%d) configured as %s", Plotter_nr, this->Name.c_str());
	return EXIT_SUCCESS;
}

Packet *OFHexPlotter::simple_action(Packet *p)
{
	OF::Functions::printHexAndChar((uint8_t *)p->data(), p->length(), std::string(this->Name.c_str()) + " Plotter Nummer " + std::to_string(this->Plotter_nr) + ": ");

	return p;
};
CLICK_ENDDECLS

EXPORT_ELEMENT(OFHexPlotter)
