// W2WAlarmRelay.click

//Standard router-configuration for the participants in the alarm-system
//evaluates network situation baased on alarm-messages
//generates feedback messages in specified periods

//211 --> locally personallized variable for the participants address
//DB      --> widely used by packets to define the debug level and the number of messages that are plottet

//defining some variables for the interface (wlan)
define($EIFACENAME eth0);											// local interface name
define($WIFI wlan0);
AddressInfo($EIFACENAME 192.168.2.211 b8:27:eb:e3:50:49);				// incorrect address information for interface illeravant since only broadcasts are used
define($ERECVPORT 4322);											// port that receives an alarm
define($ESENDPORT 1235);											// port that sends the alarm
define($ROUTERIP 255.255.255.255);									// broadcast address for the wlan network

define($VLCIP 192.168.2.235)
define($VLCport 42000)
define($OTHERPOLEIPETHER 192.168.2.214);
define($OTHERPOLEIPWIFI 10.20.2.214);

CKCAWIFI :: CamKinChecksumAdder(DB 1)
	-> EncWIFI :: UDPIPEncap($WIFI, 
						54321, 
						$OTHERPOLEIPWIFI,
						$VLCport,true)    
    -> EarpWifi :: ARPQuerier($WIFI)
	-> outQWifi :: Queue
	-> ToDevice($WIFI);

VLCTLDC :: VLCTrafficLightDirectControll(DB 1, AD 211)
    -> CKCAWIFI;

VLCInit(TICK 10000,DB 1,AD 211)
	-> CKCA :: CamKinChecksumAdder(DB 1)
	-> Enc :: UDPIPEncap($EIFACENAME, 
						444, 
						$VLCIP,
						$VLCport,true)
	-> Earp :: ARPQuerier($EIFACENAME)
	-> outQ :: Queue
	-> ToDevice($EIFACENAME);

FromDevice($EIFACENAME)
	-> ecl :: Classifier(12/0800,									// Classifying incomming packets in to IP packets
                         12/0806 20/0002,							// ARP Replies
                         12/0806 20/0001							// ARP Queries
                        )											// and other
	-> CheckIPHeader(14)
	-> Strip(42)
    -> IPC :: IPClassifier(src host $VLCIP, src host $OTHERPOLEIPETHER)
    -> Print("VLC")
	-> VLCTLDC;


FromDevice($WIFI)
    -> eclWifi :: Classifier(12/0800,									// Classifying incomming packets in to IP packets
                         12/0806 20/0002,							// ARP Replies
                         12/0806 20/0001							// ARP Queries
                        )											// and other
    -> CheckIPHeader(14)
    -> Strip(42)
    -> IPClassifier(src host $OTHERPOLEIPWIFI)
    -> Print("WLAN")
    -> VLCTLDC;

IPC[1] 
    -> Print("Ethernet")
    -> VLCTLDC;

VLCTLDC[1] 
	-> CKCA;
	
ecl[1]
	-> [1]Earp;													// if a arp replie is receivd it is send to the arpquerier
ecl[2]
	-> ARPResponder($EIFACENAME)									// arp queries are responded to
    -> outQ;

eclWifi[1]
	-> [1]EarpWifi;													// if a arp replie is receivd it is send to the arpquerier
eclWifi[2]
	-> ARPResponder($WIFI)									// arp queries are responded to
    -> outQWifi;
