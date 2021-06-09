
// Declare Elements
define($EIFACENAME eno2);
AddressInfo($EIFACENAME 143.93.153.63 c8-f7-50-01-5c-2d);

define($EIFACENAME2 enx24f5a2f13a13);
AddressInfo($EIFACENAME2 192.168.2.52 24:f5:a2:f1:3a:13);

define($EIFACENAME3 enx58ef68b516a1);
AddressInfo($EIFACENAME3 192.168.2.53 58:ef:68:b5:16:a1);

define($WIFI wlo1);


define($VLCIP 192.168.2.234);
define($VLCPORT 42000);
define($SENDPORT 54321);
define($BROADCASTPORT 9999);
define($BROADCASTIP 255.255.255.255);

elementclass Responder {
$src_ip, $port, $DB |
    OFR :: OFResponder(HOSTIPADDRESS $src_ip, PORT $port, DB $DB, IFNAME $EIFACENAME)

    Soc :: OFSocket(TCP, $src_ip, $port, VERBOSE true, CLIENT true)

    Class :: OFClassifier(DB 0)

    // input
    input -> [0]OFR

    // Responder
    OFR[0] -> Soc

    OFR[1] -> Discard

    OFR[2] -> output

    OFR[3] -> Discard 

    // Socket
    Soc[0] -> Class

    Soc[1] -> Discard
    
    // Classifier
    Class[0] -> [1]OFR

    Class[1] -> output
}

elementclass Interface {
    $ifname, $SPEED, $DUPLEX, $DB |

    Interface :: OFInterfaceWrapper(IFNAME $ifname, SPEED $SPEED, DUPLEX $DUPLEX, DB $DB)

    Class :: Classifier(0/aa55aa55 8/00, 0/aa55aa55 8/02, -) // First output are Packets for direct output, Second Output are Packets for the Wrapper, Third are dropped

    toDev :: OFQueue(DB 1, QUEUESIZE 256) -> ToDevice($ifname)

    input -> Class

    Class[0] -> Strip(16) /*-> OFHexPlotter(Name "Stripped: $ifname ", NR 1)*/ -> toDev

    Class[1] -> Interface

    Class[2] -> Discard

    FromDevice($ifname) -> [1]Interface

    Interface[1] -> toDev

    Interface[0] -> output

    ||
    
    $ifname, $SPEED, $DUPLEX, $DB, $VLC, $INTERVAL |
    
    Interface :: OFInterfaceWrapper(IFNAME $ifname, SPEED $SPEED, DUPLEX $DUPLEX, DB $DB, INTERVAL 2000, VLC true)

    Class :: Classifier(0/aa55aa55 8/00, 0/aa55aa55 8/02, -) // First output are Packets for direct output, Second Output are Packets for the Wrapper, Third are dropped

    toDev :: OFQueue(DB 1, QUEUESIZE 256) -> ToDevice($ifname)

    input -> Class

    Class[0] -> Strip(16) /*-> OFHexPlotter(Name "Stripped: $ifname ", NR 1)*/ -> toDev

    Class[1] -> Interface

    Class[2] -> Discard

    FromDevice($ifname) -> [1]Interface

    Interface[1] -> toDev

    Interface[0] -> output
}



// Init Elements
// Res :: Responder(143.93.156.156, 6653, 1);
// Res :: Responder(192.168.2.52, 6653, 1);
Res :: Responder(127.0.0.1, 6653, 1);

OFPipe :: OFPipeline(DB 2);

If1 :: Interface($EIFACENAME, 100, Full, 0);

If2 :: Interface($EIFACENAME2, 100, Full, 0);

If3 :: Interface($EIFACENAME3, 100, Full, 0);


// Schedule Elements
Res[0] 
    -> OFPipe;

OFPipe[0] 
    -> [0]Res;

OFPipe[1] 
    -> If1
    -> [1]OFPipe;

OFPipe[2] 
    -> If2
    -> [2]OFPipe;


OFPipe[3] 
    -> If3
    -> [3]OFPipe;


// VLCInit(TICK 10000, DB 1, AD 214)
//     -> OFHexPlotter(Name "To VLC: ", NR 1)
// 	-> CKCA :: CamKinChecksumAdder(DB 2)
//     -> OFHexPlotter(Name "To VLC: ", NR 2)
//     -> UDPIPEncap($EIFACENAME2, $SENDPORT, $VLCIP, $VLCPORT, true)
//     -> OFHexPlotter(Name "To VLC: ", NR 3)
//     -> Discard;
//     // -> EARP :: ARPQuerier($EIFACENAME)
// 	// -> EOQ :: Queue
// 	// -> ToDevice($EIFACENAME);

// FromDevice($EIFACENAME2)
// 	// -> ECL :: Classifier (12/0800, 12/0806 20/0002, 12/0806 20/0001)
// 	// -> CheckIPHeader(14)
// 	// -> Strip(42)
// 	// -> IPC :: IPClassifier(udp port $BROADCASTPORT)
//     -> OFHexPlotter(Name "From VLC: ", NR 1)
// 	// -> HexPlotter(NR 1, PA 1)
// 	// -> IPPrint()
// 	-> Discard;