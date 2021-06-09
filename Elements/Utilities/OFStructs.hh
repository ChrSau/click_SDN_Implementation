#ifndef OF_STRUCTS_HH
#define OF_STRUCTS_HH 1

#include <cstdlib>

#include "openflowWrapper.hh"

namespace OF
{
    namespace Structs
    {

        enum class OFVersions : int
        {
            Version0 = 0x01, // = 1.0
            Version1 = 0x02, // = 1.1
            Version2 = 0x03, // = 1.2
            Version3 = 0x04, // = 1.3.X
            Version4 = 0x05, // = 1.4.X
            Version5 = 0x06, // = 1.5.X
        };

        enum click_port_comm_type
        {
            CPCT_PACKET_OUT,
            CPCT_PACKET_IN,
            CPCT_PORT_DESC,
            CPCT_PING_REQUEST,
            CPCT_PING_RESPONSE,
            CPCT_PORT_STATUS
        };

        enum click_port_comm_identifier
        {
            CPCI_FROM_PIPELINE = 0x55AA55AA,
            CPCI_TO_PIPELINE = 0xAA55AA55,
            CPCI_TO_RESPONDER,
            CPCI_FROM_RESPONDER,
            CPCI_PING
        };

        struct click_port_comm_header
        {
            uint32_t identifier; /* CPCI_* */
            uint32_t length;     /* data length */
            uint8_t type;        /* CPCT_* */
            uint8_t pad[7];
            uint8_t data[0];
        };

        struct click_ping_data
        {
            uint64_t seqNummer;
        };

        struct my_ofp_oxm_header
        {
            uint16_t oxm_class;   /* OXM Class */
            uint8_t oxm_field_WM; /* OXM Field with Mask Bit */
            uint8_t oxm_length;   /* Payload Length */
            uint8_t oxm_value[0]; /* OXM Value as Byte Array */
        };

        const struct hardwarePorts
        {
            const uint8_t broadcastPort[6] = {0xff,
                                              0xff,
                                              0xff,
                                              0xff,
                                              0xff,
                                              0xff};

            const uint8_t multicastPort[3] = {0x01,
                                              0x00,
                                              0x5e};

            const uint8_t ip6MultiCastPort[2] = {0x33,
                                                 0x33};
        };

        struct my_ofp_flow_count
        {
            uint64_t ReceivedPacketsCounter;
            uint64_t ReceivedBytesCounter;
            uint32_t Duration_s_Counter;
            uint32_t Duration_ns_Counter;
        };

        struct my_ofp_flow_data
        {
            uint64_t Timeouts;
            uint64_t Cookie;
            uint64_t Flags;
            struct my_ofp_flow_count Counter;
            /*uint64_t ReceivedPacketsCounter;
    uint64_t ReceivedBytesCounter;
    uint32_t Duration_s_Counter;
    uint32_t Duration_ns_Counter;*/
            uint16_t Priority;
            uint8_t pad[6];
        };

        struct ofp_table_features_v5
        {
            uint16_t length;  /* Length is padded to 64 bits. */
            uint8_t table_id; /* Identifier of table. Lower numbered tables are consulted first. */
            uint8_t pad[5];   /* Align to 64-bits. */
            char name[OFP_MAX_TABLE_NAME_LEN];
            uint64_t metadata_match; /* Bits of metadata table can match. */
            uint64_t metadata_write; /* Bits of metadata table can write. */
            uint32_t capabilities;   /* Bitmap of OFPTC_* values. */
            uint32_t max_entries;    /* Max number of entries supported. */

            /* Table Feature Property list */
            struct ofp_table_feature_prop_header properties[0]; /* List of properties */
            // uint8_t tableFeaturesProbertys[1408]
        };

        struct my_ofp_aggregate_stats_reply
        {
            uint64_t packet_count; /* Number of packets in flows. */
            uint64_t byte_count;   /* Number of bytes in flows. */
            uint32_t flow_count;   /* Number of flows. */
            uint8_t pad[4];        /* Align to 64 bits. */
        };

        struct my_ofp_port
        {
            uint32_t port_no;
            uint16_t length;
            uint8_t pad[2];
            uint8_t hw_addr[OFP_ETH_ALEN];
            uint8_t pad2[2];
            char name[OFP_MAX_PORT_NAME_LEN];
            uint32_t config;
            uint32_t state;
            struct ofp_port_desc_prop_ethernet properties[1];
        };

        struct ofp_port_status
        {
            struct ofp_header header;
            uint8_t reason; /* One of OFPPR_*. */
            uint8_t pad[7]; /* Align to 64-bits. */
            struct my_ofp_port desc;
        };

        struct my_ofp_port_counter
        {
            uint64_t ReceivedPackets;             /* Required */
            uint64_t TransmittedPackets;          /* Required */
            uint64_t ReceivedBytes;               /* Optional */
            uint64_t TransmittedBytes;            /* Optional */
            uint64_t ReceiveDrops;                /* Optional */
            uint64_t TransmitDrops;               /* Optional */
            uint64_t ReceiveErrors;               /* Optional */
            uint64_t TransmitErrors;              /* Optional */
            uint64_t ReceiveFrameAlignmentErrors; /* Optional */
            uint64_t ReceiveOverrunErrors;        /* Optional */
            uint64_t ReceiveCRCErrors;            /* Optional */
            uint64_t Collisions;                  /* Optional */
            uint32_t Duration_s;                  /* Required */
            uint32_t Duration_ns;                 /* Optional */
        };

        /* Send packet (controller -> datapath). Openflow v1.4 */
        struct ofp_packet_out_v4
        {
            struct ofp_header header;
            uint32_t buffer_id;   /* ID assigned by datapath (OFP_NO_BUFFER if none). */
            uint32_t in_port;     /* Packet’s input port or OFPP_CONTROLLER. */
            uint16_t actions_len; /* Size of action array in bytes. */
            uint8_t pad[6];
            struct ofp_action_header actions[0]; /* Action list - 0 or more. */
            /* The variable size action list is optionally followed by packet data.
* This data is only present and meaningful if buffer_id == -1. */
            /* uint8_t data[0]; */
            /* Packet data. The length is inferred
from the length field in the header. */
        };

        struct my_ip_header
        {
            uint8_t versionAndHeaderLenght;
            uint8_t differentialServiceField;
            uint16_t totalLength;
            uint16_t identification;
            uint16_t flags;
            uint8_t TTL;
            uint8_t protocol;
            uint16_t headerChecksum;
            uint8_t source[4];
            uint8_t dest[4];
        };

        struct controllerData
        {
            uint8_t OFVersion;
            uint64_t generationId;
            uint32_t rolle;
        };

        /* Send packet (controller -> datapath). Openflow v1.4 */
        struct ofp_queue_counter
        {
            uint64_t Transmit_Packets;
            uint64_t Transmit_Bytes;
            uint64_t Transmit_Overrun_Errors;
        };

#define MAC_ADDRESS_LENGTH 6

        struct mac_address
        {
            uint8_t addr[MAC_ADDRESS_LENGTH];
        };

        struct ethernet_header
        {
            struct mac_address dst;
            struct mac_address src;
            uint16_t type;
        };

        struct ethernet_IEEE802_3_header
        {
            struct mac_address dst;
            struct mac_address src;
            uint16_t length;
            uint8_t dsap;
            uint8_t ssap;
            uint16_t control;
        };
    } // namespace Structs
} // namespace OF

#endif