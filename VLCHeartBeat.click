// W2WAlarmRelay.click

//Standard router-configuration for the participants in the alarm-system
//evaluates network situation baased on alarm-messages
//generates feedback messages in specified periods

//214 --> locally personallized variable for the participants address
//DB      --> widely used by packets to define the debug level and the number of messages that are plottet

//define ethernet interface for vlc module
define($EIFACENAME enx24f5a2f13a13);
AddressInfo($EIFACENAME 192.168.2.52 24:f5:a2:f1:3a:13);
define($VLCIP 192.168.2.235);
define($VLCPORT 42000);
define($SENDPORT 54321);
define($BROADCASTPORT 9999);
define($BROADCASTIP 255.255.255.255);
// define($OTHERPOLEIPETHER 192.168.2.211);
// define($OTHERPOLEIPWIFI 10.20.2.211);

//ethernet out
VLCInit(TICK 10000, DB 1, AD 214)
	-> CKCA :: CamKinChecksumAdder(DB 2)
	-> UDPIPEncap($EIFACENAME, $SENDPORT, $VLCIP, $VLCPORT, true)
	-> EARP :: ARPQuerier($EIFACENAME)
	-> OFHexPlotter(Name "From VLC", NR 1)
	-> EOQ :: Queue
	-> ToDevice($EIFACENAME);


//ethernet in
FromDevice($EIFACENAME)
	-> ECL :: Classifier (12/0800, 12/0806 20/0002, 12/0806 20/0001)
	-> CheckIPHeader(14)
	-> Strip(42)
	-> IPC :: IPClassifier(udp port $BROADCASTPORT)
	-> HexPlotter(NR 1, PA 1)
	-> IPPrint()
	-> Discard;

// FromDevice($WIFI)
// 	// -> ECLWIFI :: Classifier (12/0800, 12/0806 20/0002, 12/0806 20/0001)
// 	-> Print("WLAN")
//     -> CheckIPHeader(14)
//     -> IPPrint()
//     -> Discard;


// TimedSource()
//     -> CamKinChecksumAdder(DB 2)
// 	-> UDPIPEncap($WIFI, $BROADCASTPORT, $BROADCASTIP, $BROADCASTPORT, true)
//     // -> EARP2 :: ARPQuerier($WIFI)
//     -> EOQ2 :: Queue
//     -> IPPrint("Sending Broadcast")
//     -> ToDevice($WIFI);


// TimedSource()
//     -> CamKinChecksumAdder(DB 2)
// 	-> UDPIPEncap($EIFACENAME, $BROADCASTPORT, $BROADCASTIP, $BROADCASTPORT, true)
//     -> IPPrint("Sending Broadcast Ethernet")
//     -> EOQ;



// TL :: VLCTrafficLight(TICK 500,DB 5,AD 214)
// 	-> CKCA;

// TL[1]
// 	-> VLCWrapper(DB 1,AD 214)
// 	-> RRSwitch;


    


//ehternet ARP-Stuff
ECL[1]
	-> [1]EARP;
ECL[2]
	-> ARPResponder($EIFACENAME)
	-> EOQ;

// ECLWIFI[1]
//     -> [1]EARP2;
// ECLWIFI[2]
//     -> ARPResponder($WIFI)
//     -> EOQ2;
