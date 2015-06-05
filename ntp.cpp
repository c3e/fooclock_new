#include "ntp.h"

// NTP Servers:
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov


//const int timeZone = 1;     // Central European Time
const int timeZone = 2;     // Central European Time (summertime)

//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)


EthernetUDP Udp;
EthernetUDP u;
unsigned int localPort		= 8888;  // local port to listen for UDP packets



const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

byte messageBuffer[200];


void setupNTP(){
	Udp.begin(localPort);
	setSyncProvider(getNtpTime);
}

int init_udp_socket(){
	byte tmp[200];
	//byte * p;
	//int ret = 0;
	//p = messageBuffer;
	u.begin(2343);
	//uint32_t beginWait = millis();
	while (true) {
		u.parsePacket();
		
		int size = u.read(tmp,199);
		
		if (size <= 0) continue;
		Serial.println("Parsing Packet ...");
	//	u.remoteIP().printTo(Serial);
		tmp[size] = 0;
		memcpy(messageBuffer,tmp,200);
	//	Serial.println((char*)tmp);
	//	return 1;
		//memcpy(p, tmp, size);
		Serial.println(size);
		//p += size;
		//ret += size;
		//return messageBuffer;
		return size;
	}
	return 0;
}

time_t getNtpTime()
{
	while (Udp.parsePacket() > 0) ; // discard any previously received packets
	//Serial.println("Transmit NTP Request");
	sendNTPpacket(timeServer);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Serial.println("Receive NTP Response");
			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
		}
	}
	//Serial.println("No NTP Response :-(");
	return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0]   = 0b11100011;   // LI, Version, Mode
	packetBuffer[1]   = 0;     // Stratum, or type of clock
	packetBuffer[2]   = 6;     // Polling Interval
	packetBuffer[3]   = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(address, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();
}