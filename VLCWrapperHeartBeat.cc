//click includes
#include <click/config.h>
#include <click/args.hh>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/udp.h>

//our includes
#include "VLCWrapperHeartBeat.hh"
#include "vlc_proto.hh"

#include <iostream>

CLICK_DECLS

//constructor with registration of the timer element
VLCWrapperHeartBeat::VLCWrapperHeartBeat(){};
//basic destructor
VLCWrapperHeartBeat::~VLCWrapperHeartBeat(){};

//initializing the timer to zum immidietly
int VLCWrapperHeartBeat::initialize(ErrorHandler *)
{
    //initialize things
    //click_chatter("Initalizing");
    //create_crc32_table(Crc32Table, sizeof(Crc32Table) / sizeof(uint32_t));
    click_chatter("VLCWrapperHeartBeat initialized");

    return 0;
}

//configuring the two parameters
int VLCWrapperHeartBeat::configure(Vector<String> &conf, ErrorHandler *errh)
{

    _tick_nr = 0;
    headroom = sizeof(click_ip) + sizeof(click_udp) + sizeof(click_ether);

    if (Args(conf, this, errh)
            .read_mp("DB", _DBlevel)
            .read_mp("PORT", _PORT)
            .complete() < 0)
    {
        return -1;
    }
    click_chatter("VLCWrapperHeartBeat configured");
    return 0;
}

// void plotVLCBytes(char* toPlot, int32_t size){
// 	String lo;
// 	int restLength = size - 1;
// 	int counter = 0;
// 	while(restLength >= 0){
// 		if(counter%16 == 0 && counter > 0){
// 			lo = lo + "\n";
// 		}else if(counter%8 == 0 && counter > 0){
// 			lo = lo + "   ";
// 		}

// 		int dp = toPlot[counter];
// 		char ps[3];
// 		snprintf(ps,3,"%02x",dp);

// 		//lo = lo + String(dp) + " ";
// 		lo = lo + String(ps) + " ";
// 		//click_chatter("%d --> %s --> %s",dp,String(dp).c_str(),lo.c_str());

// 		counter++;
// 		restLength--;
// 	}
// 	click_chatter("%s",lo.c_str());
// }

// VLCdata VLCheaderInit(VLCdata in, int dataLength){
// 	in.header.endianess = 0xff00;

// 	in.header.tcipv_dcv = 0;
// 	in.header.tcipv_cv = 0;
// 	in.header.tcipv_mcs = 0;

// 	in.header.frameType = 0x3001;
// 	in.header.sendCount = 0;
// 	in.header.timestamp = 0;

// 	in.header.dataChecksum = 0;
// 	in.header.dataLength = dataLength;
// 	in.header.headerChecksum = 0;

// 	return in;
// }

// VLCdata VLCBroadcastSet(VLCdata in, int dataLength, int address){
// 	in.sourceIP = 0x0A140200 + address;
// 	in.destinationIP = 0xFFFFFFFF; //always broadcasts
// 	in.sourceID = 0;
// 	in.destinationID = 0;
// 	in.sourcePort = 9999;
// 	in.destinationPort = 9999;
// 	in.TTL = 0;
// 	in.PayloadType = 0;
// 	in.DataLength = dataLength;

// 	return in;
// }

Packet *VLCWrapperHeartBeat::simple_action(Packet *p)
{
    //simple action does nothin in this element
    //if(_DBlevel>5){click_chatter("Started Wrapping");}
    // char* data = (char*) p->data();
    // int dataLength = p->length();
    // int camkinLength = dataLength + sizeof(VLCdata) - sizeof(VLCGenHeader) - VLC_3001_MAXSIZE;

    // //if(_DBlevel>10){
    // //	click_chatter("| %d | %d | %d |",sizeof(VLCGenHeader),sizeof(VLCdata) - sizeof(VLCGenHeader) - VLC_3001_MAXSIZE,p->length());
    // //}

    // struct VLCdata V;
    // V = VLCheaderInit(V,camkinLength);
    // V = VLCBroadcastSet(V,dataLength,_AD);
    // for(int i=0;i<dataLength;i++){
    // 	V.data[i].byte = (int)data[i];
    // }

    // WritablePacket *P = WritablePacket::make(headroom, &V, sizeof(VLCdata) - VLC_3001_MAXSIZE + p->length(),0);

    // _tick_nr++;

    // p->kill();

    for (unsigned int i = 0; i < p->length(); i++)
    {
        std::cout << std::hex << (int)p->data()[i];
        if (i % 8 == 0 && i > 0)
            std::cout << " ";
        if (i % 16 == 0 && i > 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;

    return p;
};
CLICK_ENDDECLS
//export our element
EXPORT_ELEMENT(VLCWrapperHeartBeat)
