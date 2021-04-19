
// Trivial Torrent

// TODO: some includes here

#include "file_io.h"
#include "logger.h"

#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
// TODO: hey!? what is this?

/**
 * This is the magic number (already stored in network byte order).
 * See https://en.wikipedia.org/wiki/Magic_number_(programming)#In_protocols
 */
static const uint32_t MAGIC_NUMBER = 0xde1c3231; // = htonl(0x31321cde);

static const uint8_t MSG_REQUEST = 0;
static const uint8_t MSG_RESPONSE_OK = 1;
static const uint8_t MSG_RESPONSE_NA = 2;

enum { 	RAW_MESSAGE_SIZE = 13 };


/**
 * Main function.
 */



int main(int argc, char **argv) {

	set_log_level(LOG_DEBUG);

	log_printf(LOG_INFO, "Trivial Torrent (build %s %s) by %s", __DATE__, __TIME__, "L. Panareda");

	// ==========================================================================
	// Parse command line
	// ==========================================================================

	// TODO: some magical lines of code here that call other functions and do various stuff.






	// Comprovar si servidor o client
	if(argc == 2) // TODO: canviar aixo que no sigui tan cutre
	{
		/*
		1. Load a metainfo file (functionality is already available in the file_io API through the create_torrent_from_-
		metainfo_file function).
			a. Check for the existence of the associated downloaded file.
			b. Check which blocks are correct using the SHA256 hashes in the metainfo file.
		2. For each server peer in the metainfo file:
			a. Connect to that server peer.
			b. For each incorrect block in the downloaded file (the hash does not match in 1b).
				i. Send a request to the server peer.
				ii. If the server responds with the block, store it to the downloaded file.
				iii. Otherwise, if the server signals the unavailablity of the block, do nothing.
			c. Close the connection.
		3. Terminate.

		*/

		// 1 //

		

		struct torrent_t torrent;

		long int unsigned pathSize = 0;

		while(*(*(argv + 1) + pathSize ) != '\0')
			++pathSize;

		if(pathSize <= 9)
		{
			log_message(LOG_INFO, "The file must be in .ttorrent format");
			return -1;
		}

		char *metainfoFileName = malloc(sizeof(char)*pathSize);
		memcpy(metainfoFileName, *(argv + 1), pathSize);

		pathSize-=9; // points to the '.' in anystr.ttorrent
		char *fileName = malloc(sizeof(char)*pathSize);
		memcpy(fileName, *(argv + 1), pathSize);

		//1. Load a metainfo file (functionality is already available in the file_io API through the create_torrent_from_-metainfo_file function).			
		if (create_torrent_from_metainfo_file (metainfoFileName, &torrent ,fileName ) == -1)
		{
			log_message(LOG_INFO, "Could not create torrent from metainfo file");
			return -1;
		}

		/*	
		2. For each server peer in the metainfo file:
			a. Connect to that server peer.
			b. For each incorrect block in the downloaded file (the hash does not match in 1b).
				i. Send a request to the server peer.
				ii. If the server responds with the block, store it to the downloaded file.
				iii. Otherwise, if the server signals the unavailablity of the block, do nothing.
			c. Close the connection.
		*/

		/* Creació del socket */
		
		
		


		

		uint8_t message[RAW_MESSAGE_SIZE];
		message[0] = (uint8_t) (MAGIC_NUMBER);
		message[1] = (uint8_t) (MAGIC_NUMBER >> 8);
		message[2] = (uint8_t) (MAGIC_NUMBER >> 16);
		message[3] = (uint8_t) (MAGIC_NUMBER >> 24);

		// Per cada requested block

		(void) message; //TODO: treure això

		for (uint64_t block_number = 0; block_number < torrent.block_count; block_number++)
		{		/* Per cada block que no tenim*/

			
			if(torrent.block_map[block_number])
			{
				message[4] = MSG_REQUEST;
				message[5] = (uint8_t) (block_number);
				message[6] = (uint8_t) (block_number >> 8);
				message[7] = (uint8_t) (block_number >> 16);
				message[8] = (uint8_t) (block_number >> 24);
				message[9] = (uint8_t) (block_number >> 32);
				message[10] = (uint8_t) (block_number >> 40);
				message[11] = (uint8_t) (block_number >> 48);
				message[12] = (uint8_t) (block_number >> 56);
			}
		}
		

		struct sockaddr_in sockaddress;
		memset(&sockaddress, '\0', sizeof(struct sockaddr_in));

		sockaddress.sin_family = AF_INET;
		sockaddress.sin_port = 0;
		sockaddress.sin_addr.s_addr = INADDR_ANY;

		int sock = socket(AF_INET, SOCK_STREAM, 0);

		struct sockaddr_in sockaddress_peer;
		memset(&sockaddress_peer, '\0', sizeof(struct sockaddr_in));
		sockaddress_peer.sin_family = AF_INET;
		sockaddress_peer.sin_port = 0;
		
		
		for (uint64_t j = 0; j < torrent.peer_count; j++) /* 2. For each server peer in the metainfo file: */
		{

			// a. Check for the existence of the associated downloaded file.
			uint64_t i = 0;
			int fileDownloaded = 1;
			log_message(LOG_DEBUG, "Checking if file is already on disk...");
			while ((i < torrent.block_count) && (fileDownloaded == 1))
        	{
				if (torrent.block_map[i] == 0)
					fileDownloaded = 0;
				++i;
			}
			if(fileDownloaded == 1) // 
			{
				log_message(LOG_DEBUG, "File already downloaded, no point in continuing");
				break;
			}
			else
				log_message(LOG_DEBUG, "File not found on disk, proceding to download it");




			if (sock == -1)
			{
				perror("Error: socket could not be created");
				return 1;
			}

			
			sockaddress_peer.sin_port = (torrent.peers + i)->peer_port;
			printf("Peer port:%c", sockaddress_peer.sin_port);
			sockaddress_peer.sin_addr.s_addr = (uint32_t) inet_addr("127.0.0.1"); // TODO:

			/*
			sockaddress_peer.sin_addr.s_addr += (torrent.peers + i)->peer_address[0];

			sockaddress_peer.sin_addr.s_addr = sockaddress_peer.sin_addr.s_addr << 8;
			
			sockaddress_peer.sin_addr.s_addr += (torrent.peers + i)->peer_address[1];
			
			sockaddress_peer.sin_addr.s_addr = sockaddress_peer.sin_addr.s_addr << 8;
			
			sockaddress_peer.sin_addr.s_addr += (torrent.peers + i)->peer_address[2];
			
			sockaddress_peer.sin_addr.s_addr = sockaddress_peer.sin_addr.s_addr << 8;
			
			sockaddress_peer.sin_addr.s_addr += (torrent.peers + i)->peer_address[3];
			*/
			
			if(connect(sock, (struct sockaddr *) &sockaddress_peer, sizeof(struct sockaddr_in)) == -1)
			{
				perror("Error: Connect() function exited with code -1");
				return -1;
			}
			log_message(LOG_DEBUG, "Socket connected succesfully");
			
			
		}
		

	}

	// The following statements most certainly will need to be deleted at some point...

	
	(void) MAGIC_NUMBER;
	(void) MSG_REQUEST;
	(void) MSG_RESPONSE_NA;
	(void) MSG_RESPONSE_OK;

	return 0;
}

