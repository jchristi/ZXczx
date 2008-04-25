/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "SocketClass.h"

#include "thread.h"



/*
** notes on packets
**
** chat packet
** 1 byte  = N+1
** 1 byte  = 'C' (or 'X' if it's a special packet, for example for LAN servers discovery)
** N bytes = null terminated message string
** total = N+2
**
** order packet
**    1 byte  = 23  (not buffered)
** 0  1 byte  = 'O'
** 1  4 bytes = time of execution
** 5  4 bytes = unit number
** 9  1 byte  = order type
** 10 4 bytes = floating data 1
** 14 4 bytes = floating data 2
** 18 4 bytes = target unit
** 22 1 byte  = this is an additional order
** total = 24
**
** sync packet
**    1 byte  = 27  (not buffered)
** 0  1 byte  = 'S'
** 1  4 bytes = time of sync
** 5  4 bytes = unit number
** 9  4 bytes = floating x
** 13 4 bytes = floating y
** 17 4 bytes = floating vx
** 21 4 bytes = floating vy
** 25 2 bytes = orientation
** total = 28
**
** event packet
** 1 byte  = 4
** 1 byte  = 'E'
** 1 byte  = event type
** 1 byte  = player 1
** 1 byte  = player 2
** total = 5
*/

#define		TA3DSOCK_BUFFER_SIZE		2560

//chat order sync and event
//these are sent and received over the network
struct chat{
	unsigned short from; 			//uint16 who said
	char message[253];	//said what
};//max size = 254

struct order{
	int timestamp;//uint32 what time the order happened
	int unit;//uin32 order what unit
	char command;//uint8 move,fire,stop,guard,reclaim,etc
	float x;
	float y;
	int target;//uint32 enough to tell it a target unit
	char additional;//uint8 0=cancel current activity 1=add to order queue
};//max size = 22

struct sync{
	int timestamp;//uint32 what time is this snapshot
	int unit;//uint32 sync what unit
	float x,y;
	float vx,vy;
	short orientation;//uint16 where 0=0 and 65535~=2pi? ie rad=(rot1/65536.0)*2pi?
};//max size = 26

struct event{
	char type;//uint8 what type of event
	char player1;//uint8 optional parameters
	char player2;//uint8
};//max size = 3

typedef chat special;

inline chat* strtochat( struct chat *chat_msg, std::string msg )
{
	if( chat_msg == NULL )	return chat_msg;
	memset( chat_msg->message, 0, 253 );
	memcpy( chat_msg->message, msg.c_str(), min( 253, (int)msg.size() + 1 ) );
	return chat_msg;
}

inline std::string chattostr( struct chat *chat_msg )
{
	if( chat_msg == NULL )	return "";
	std::string msg( chat_msg->message, 253 );
	msg = msg.c_str();								// Make sure it represents a null terminated string
	return msg;
}

//TA3DSock- specialized low-level networking
//used internally by Network to send the data
//to the players. It is basically a TCP socket
class TA3DSock{

	char name[64];//player name

	Socket tcpsock;
	Mutex tcpmutex;

	//only touched by main thread
	char outbuf[TA3DSOCK_BUFFER_SIZE];
	int obp;

	//only touched by socket thread
	char tcpinbuf[TA3DSOCK_BUFFER_SIZE];
	int tibp;
	int tiremain;//how much is left to recv
	
	//byte shuffling
	void loadLong(uint32_t x);//uint32
	void loadShort(uint16_t x);//uint16
	void loadByte(uint8_t x);//uint8
	void loadString(const char* x);//null terminated
	void loadFloat(float x);

	void sendTCP();
	void recvTCP();

	int max(int a, int b) {return (a>b ? a : b);}

	public:
		TA3DSock() {obp=0;tibp=0;tiremain=-1;}
		~TA3DSock() {}

		int Open(const char* hostname,const char* port);
		int Accept(TA3DSock** sock);
		int Accept(TA3DSock** sock,int timeout);
		void Close();

		void sendTCP(byte *data, int size);

		char* getAddress() {return tcpsock.getNumber();}
		char* getPort() {return tcpsock.getService();}
		Socket& getSock() {return tcpsock;}
		int isOpen();

		//these are for outgoing packets
		int sendSpecial(struct chat* chat);
		int sendChat(struct chat* chat);
		int sendOrder(struct order* order);
		int sendSync(struct sync* sync);
		int sendEvent(struct event* event);
		void cleanPacket();

		//these are for incoming packets
		int makeSpecial(struct chat* chat);
		int makeChat(struct chat* chat);
		int makeOrder(struct order* order);
		int makeSync(struct sync* sync);
		int makeEvent(struct event* event);

		int getFilePort();				// For file transfer, first call this one to get the port which allows us to grab the right thread and buffer
		int getFileData(byte *buffer);	// Fill the buffer with the data and returns the size of the paquet

		char getPacket();//if packet is ready return the type, else return -1
		void pumpIn();//get input from sockets non-blocking
		int takeFive(int time);//sleep until sockets will not block
};
