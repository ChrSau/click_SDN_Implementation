//our includes
#include "OFEtherEncap.hh"

CLICK_DECLS

//constructor
OFEtherEncap::OFEtherEncap()
{
	this->etherAddrDst = nullptr;
	this->etherAddrSrc = nullptr;
};

//basic destructor
OFEtherEncap::~OFEtherEncap(){};

int OFEtherEncap::configure(Vector<String> &conf, ErrorHandler *errh)
{
	String dst;
	String src;

	if (Args(conf, this, errh)
			.read_mp("SRC", src)
			.read_mp("DST", dst)
			.complete() < 0)
	{
		return -EXIT_FAILURE;
	}

	this->etherAddrSrc = new EtherAddress();
	if (!cp_ethernet_address(src, etherAddrSrc, 0))
	{
		std::cerr << "ERROR: Could't parse SRC Ethernet Address" << std::endl;
		return -EXIT_FAILURE;
	}

	this->etherAddrDst = new EtherAddress();
	if (!cp_ethernet_address(dst, etherAddrDst, 0))
	{
		std::cerr << "ERROR: Could't parse DST Ethernet Address" << std::endl;
		return -EXIT_FAILURE;
	}

	click_chatter("OFEtherEncap(%s) configured as %s", etherAddrDst->unparse().c_str(), etherAddrSrc->unparse().c_str());
	return EXIT_SUCCESS;
}

void OFEtherEncap::cleanup(CleanupStage)
{
	if (this->etherAddrDst != nullptr)
		delete this->etherAddrDst;
	this->etherAddrDst = nullptr;

	if (this->etherAddrSrc != nullptr)
		delete this->etherAddrSrc;
	this->etherAddrSrc = nullptr;
}

//This is executed on receive Event
Packet *OFEtherEncap::simple_action(Packet *p)
{
	uint8_t *data = (uint8_t *)p->data();

	struct etherHeaderWithData
	{
		struct click_ether etherHeader;
		uint8_t data[0];
	};

	etherHeaderWithData *etherPacket = (etherHeaderWithData *)std::calloc(1, sizeof(struct etherHeaderWithData) + p->length());
	std::memcpy(etherPacket->etherHeader.ether_dhost, this->etherAddrDst->data(), MAC_ADDRESS_LENGTH);
	std::memcpy(etherPacket->etherHeader.ether_shost, this->etherAddrSrc->data(), MAC_ADDRESS_LENGTH);

	etherPacket->etherHeader.ether_type = ETHERTYPE_IP;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
	etherPacket->etherHeader.ether_type = __builtin_bswap16(etherPacket->etherHeader.ether_type);
#endif

	std::memcpy(etherPacket->data, p->data(), p->length());

	WritablePacket *response = Packet::make(etherPacket, sizeof(struct click_ether) + p->length());

	p->kill();td::string(this->Name.c_str()) + " Plotter Nummer " + std::to_string(this->Plotter_nr) + ": ");

	//return message to be passed on (this means to tee is required for use)
	//message is not modified, therefore no copie has to be created

	std::free(etherPacket);
	etherPacket = nullptr;

	return response;
};
CLICK_ENDDECLS

EXPORT_ELEMENT(OFEtherEncap)
