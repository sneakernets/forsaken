/*
 *
 * Copyright (C) 2009  Daniel Aquino
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>, or write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef NET_INCLUDED
#define NET_INCLUDED

/*
 *  Network State
 */

typedef enum {
	NETWORK_CONNECTED,
	NETWORK_CONNECTING,
	NETWORK_DISCONNECTED,
} network_state_t;

network_state_t network_state;

/*
 *  Main Functions
 */

// 0 for default port
int  network_setup( char* player_name, int local_port ); // must be called first
int  network_join( char* address, int port );
void network_host();

void network_pump();	// process network routines, fire events, marshal packets
void network_cleanup();	// stop and cleanup networking

/*
 *  Players
 */

#define NETWORK_MAX_NAME_LENGTH 15
#define INET_ADDRSTRLEN 16

typedef struct _network_player_t network_player_t;

struct  _network_player_t {
	char name[ NETWORK_MAX_NAME_LENGTH ]; // 14 chars for player name + 1 /0
	char ip[INET_ADDRSTRLEN];
	int  port;
	long int ping;
	long int packet_loss;
	long int bw_in;
	long int bw_out;
	network_player_t * prev;
	network_player_t * next;
	void* data; // internal use only
};

typedef struct {
	int length;
	network_player_t * first;
	network_player_t * last;
} network_players_t;

network_players_t network_players;

void network_set_player_name( char* name );

/*
 *  Sending Data
 */

// unsequenced	| unreliable	- is the default
// reliable		| unsequenced	- is not supported
// reliable						- is always sequenced
// sequenced	| unreliable	- will drop late pkts & instantly deliver arrived pkts (position updates)
// sequenced	| reliable		- will wait for late pkts to arive

// multiple types of pkts can all be sent on the same channel
// reliable | sequenced will take part in the same sequencing order on a channel
// unreliable | sequenced pkts will get dropped if they are late...

// 1 pkt is handled at a time and normally you want to process all pkts that are in queue
// thus regardless of pkt order you will process the arrived pkt within that frame...
// there is no point in over channelizing everything...

// channels are mainly good for separating sequenced streams...
// for instance if you have a stream of position updates and another stream for voice pkts
// you don't want your position pkt dropped cause a newer voice pkt came in first...
// thus you would create channels to seperate those streams...

typedef enum {
	NETWORK_RELIABLE	= 2, // note: reliable is also sequenced
	NETWORK_SEQUENCED	= 4, //
	NETWORK_FLUSH		= 6, // flushes the current packet
} network_flags_t;

// channel 0 is reserved
// data is copied... so you can free it right after function call
void network_send( network_player_t*, void* data, int size, network_flags_t flags, int channel );
void network_broadcast( void* data, int size, network_flags_t flags, int channel );

/*
 *  Network Events
 */

typedef enum { // data = struct network_player_t or NULL (local player)
	NETWORK_JOIN	= 2,
	NETWORK_LEFT	= 4,
	NETWORK_HOST	= 6,
	NETWORK_NAME	= 8, // player changed name
	NETWORK_DATA	= 10, // data = network_packet_t
} network_event_type_t;

typedef struct {
	int size;
	void* data;
	network_player_t* from;
} network_packet_t;

// this function will be most likely converted to an argument to network_setup
void network_event( network_event_type_t, void* data );

#endif // NET_INCLUDED