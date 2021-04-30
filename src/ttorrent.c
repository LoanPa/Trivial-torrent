
// Trivial Torrent



#include "file_io.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

// TODO: remove debug logs

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

		while (*(*(argv + 1) + pathSize ) != '\0')
			++pathSize;

		if(pathSize <= 9)
		{
			log_message(LOG_INFO, "The file must be in .ttorrent format");
			return -1;
		}

		char *metainfoFileName = malloc(sizeof(char)*pathSize);
		memcpy(metainfoFileName, *(argv + 1), pathSize);

		long int unsigned count = 9; // TODO: while buscant el "."

		pathSize-=count; // points to the '.' in anystr.ttorrent
		char *fileName = malloc(sizeof(char)*pathSize);
		memcpy(fileName, *(argv + 1), pathSize);

		//1. Load a metainfo file
		if (create_torrent_from_metainfo_file (metainfoFileName, &torrent ,fileName ) == -1)
		{
			log_message(LOG_INFO, "Could not create torrent from metainfo file");
			return -1;
		}


		/*
		struct sockaddr_in sockaddress;
		memset(&sockaddress, '\0', sizeof(struct sockaddr_in));

		sockaddress.sin_family = AF_INET;
		sockaddress.sin_port = 0;
		sockaddress.sin_addr.s_addr = INADDR_ANY;
		*/



		struct sockaddr_in sockaddress_peer;
		memset(&sockaddress_peer, '\0', sizeof(struct sockaddr_in));
		sockaddress_peer.sin_family = AF_INET;
		
		

		
		

		//TODO: Al posar això en una funció posar un paràmetre que indiqui el tipus de missatge

		// Declaracio message
		uint8_t message[RAW_MESSAGE_SIZE];

		message[0] = (uint8_t) (MAGIC_NUMBER);
		message[1] = (uint8_t) (MAGIC_NUMBER >> 8);	
		message[2] = (uint8_t) (MAGIC_NUMBER >> 16);
		message[3] = (uint8_t) (MAGIC_NUMBER >> 24);
		//message[4] = MSG_REQUEST;
		


			
		
		for (uint64_t peerNumber = 0; peerNumber < torrent.peer_count; peerNumber++) /* 2. For each server peer in the metainfo file: */
		{

			// a. Check for the existence of the associated downloaded file.
			uint64_t missingBlockNumber = 0;
			int fileDownloaded = 1;
			log_message(LOG_DEBUG, "Checking if file is already on disk...");
			while ((missingBlockNumber < torrent.block_count) && (fileDownloaded == 1))
        	{
				if (torrent.block_map[missingBlockNumber] == 0)
					fileDownloaded = 0;
				else
					++missingBlockNumber;
			}
			if(fileDownloaded == 1)
			{
				log_message(LOG_DEBUG, "File already downloaded, no point in continuing");
				return 0;
			}
			else
				log_message(LOG_DEBUG, "File not found on disk, proceding to download it");

			// Fi del file check


			int sock = socket(AF_INET, SOCK_STREAM, 0);


			if (sock == -1)
			{
				perror("Error: socket could not be created");
				return -1;
			}
			log_message(LOG_DEBUG, "Socket works"); // TODO: treure això
			
			
			
			
			// Definim port
			sockaddress_peer.sin_port = (torrent.peers + peerNumber)->peer_port;
			// Definim adreça
			sockaddress_peer.sin_addr.s_addr = (torrent.peers + peerNumber)->peer_address[3];
			sockaddress_peer.sin_addr.s_addr = sockaddress_peer.sin_addr.s_addr << 8;
			sockaddress_peer.sin_addr.s_addr |= (torrent.peers + peerNumber)->peer_address[2];
			sockaddress_peer.sin_addr.s_addr = sockaddress_peer.sin_addr.s_addr << 8;
			sockaddress_peer.sin_addr.s_addr |= (torrent.peers + peerNumber)->peer_address[1];
			sockaddress_peer.sin_addr.s_addr = sockaddress_peer.sin_addr.s_addr << 8;
			sockaddress_peer.sin_addr.s_addr |= (torrent.peers + peerNumber)->peer_address[0];
			

			
			if(connect(sock, (struct sockaddr *) &sockaddress_peer, sizeof( struct sockaddr)) == -1)
			{
				perror("Error: Connect() function exited with code -1");
				continue; //No es pot conectar, passem al seguent peer
			}
			log_printf(LOG_DEBUG, "Successfully connected to peer nº %d", peerNumber); // TODO: treure això

			
			//Per cada peer comprovem cada bloc que ens falta
			for (uint64_t block_number = 0; block_number < torrent.block_count; block_number++)
			{		/* Per cada block que no tenim*/

				if( torrent.block_map[block_number] == 0 )
				{
					message[4] = MSG_REQUEST;
					message[12] = (uint8_t) (block_number);
					message[11] = (uint8_t) (block_number >> 8);
					message[10] = (uint8_t) (block_number >> 16);
					message[9] = (uint8_t) (block_number >> 24);
					message[8] = (uint8_t) (block_number >> 32);
					message[7] = (uint8_t) (block_number >> 40);
					message[6] = (uint8_t) (block_number >> 48);
					message[5] = (uint8_t) (block_number >> 56);


					
					if (send(sock, message, sizeof(message), 0) == -1)
					{
						   /* 
						 	*	No estic segur si s'hauria de provar amb un altre block o directament canviar de peer
							* Considerarem la segona opció
							*/
						perror("Error: send() function exited with code -1");
						break;
					}
					log_message(LOG_DEBUG, "send() function executed successfully");
					
					if (recv(sock, &message, sizeof(message), 0 ) == -1)
					{
						/*
						A TENIR EN COMPTE,	
							return de la funció recv:
							These calls return the number of bytes received, or -1 if an
       						error occurred.  In the event of an error, errno is set to
				    		indicate the error.
							
						*/
						perror("Error: 1st recv() function exited with code -1");
						continue; //Provem amb un altre bloc però amb el mateix peer
					}
					log_message(LOG_DEBUG, "1st recv() function executed successfully");

					log_printf(LOG_DEBUG, "La resposta del missatge es: %d", message[4]);

					if (message[4] == MSG_RESPONSE_OK)
					{
						log_printf(LOG_DEBUG, "Peer nº %d has block nº %d", peerNumber, block_number);


						struct block_t recvd_block;


						uint8_t data_message[MAX_BLOCK_SIZE];
						
						//Necessari per a captar el codi d'error
						long int auxLen =recv(sock, &data_message, MAX_BLOCK_SIZE, MSG_WAITALL);
						log_message(LOG_DEBUG, "Ja soc aquí!");

						if (auxLen <= 0)
						{
							perror("Error: 2nd recv() function exited with code -1");
							continue; //Provem amb un altre bloc però amb el mateix peer
						}
						log_message(LOG_DEBUG, "2nd recv() function executed successfully");
						
						//Ara que sabem que el recv no els dona error fem l'assignació
						recvd_block.size = (uint64_t) auxLen;

						//Assignem a recvd_block el contingut de data_message
						memcpy(recvd_block.data, data_message, recvd_block.size);
						
						
						
						
						/*
						PER ON ANAVA:
						tenim aquest error: ttorrent: src/file_io.c:438: store_block: Assertion `block->size > 0' failed.
						i és perque el downloaded_block no està inicialitzat i hauria de contenir el bloc rebut.
						
						També crec que s'hauria de fer un altre buffer a part del message que contingues el bloc 
						
						
						Acabo d'adonar-me que primer es rep la confirmació de que el server te el bloc i després s'envia el bloc
						*/
						


						log_message(LOG_DEBUG, "Ens han enviat un bloc");
						if (store_block(&torrent, block_number, &recvd_block) == -1)
							perror("Error: block was not stored correctly");	
						else
							torrent.block_map[block_number] = 1;
						
												
					}// if del MSG_RESPONSE_OK
					log_printf(LOG_DEBUG, "For dels blocs, iteration nº: %d", block_number);

				}// Aquí acaba el if de els blocs que no tenim
			}// Aquí acaba el for dels blocs

			
			if(close(sock) == -1)
			{
				perror("Error: close() function exited with code -1");
				return -1;
			}
			log_message(LOG_DEBUG, "Socket closed");
			
			/*	
		2. For each server peer in the metainfo file:
				ii. If the server responds with the block, store it to the downloaded file.
				iii. Otherwise, if the server signals the unavailablity of the block, do nothing.
			c. Close the connection.
		*/	
			
			
		}
		

	}
	// server
	else // TODO: posar això bé
	{
		
		struct torrent_t torrent;

		long int unsigned pathSize = 0;

		while (*(*(argv + 3) + pathSize ) != '\0')
			++pathSize;

		if(pathSize <= 9)
		{
			log_message(LOG_INFO, "The file must be in .ttorrent format");
			return -1;
		}

		char *metainfoFileName = malloc(sizeof(char)*pathSize);
		memcpy(metainfoFileName, *(argv + 3), pathSize);

		long int unsigned count = 9; // TODO: while buscant el "."

		pathSize-=count; // points to the '.' in anystr.ttorrent
		char *fileName = malloc(sizeof(char)*pathSize);
		memcpy(fileName, *(argv + 3), pathSize);

		//1. Load a metainfo file
		if (create_torrent_from_metainfo_file (metainfoFileName, &torrent ,fileName ) == -1)
		{
			log_message(LOG_INFO, "Could load metainfo file correctly");
			return -1;
		}

		// TODO: tot això  ⤵
		/*
		*	1. Load a metainfo file (functionality is already implemented in the file_io API through the create_torrent_from_-
		*		metainfo_file function).
		*			a. Check for the existence of the associated downloaded file.
		*			b. Check which blocks are correct using the SHA256 hashes in the metainfo file.
		*	2. Forever listen to incoming connections, and for each connection:
		*			a. Wait for a message.
		*			b. If a message requests a block that can be served (correct hash), respond with the appropriate message,
		*				followed by the raw block data.
		*			c. Otherwise, respond with a message signaling the unavailability of the block.
		*			
		*/

		uint64_t missingBlockNumber = 0;
		int fileDownloaded = 1;
		log_message(LOG_DEBUG, "Checking blocks...");
		while ((missingBlockNumber < torrent.block_count) && (fileDownloaded == 1))
		{
			if (torrent.block_map[missingBlockNumber] != 0)
				fileDownloaded = 0;
			else
				++missingBlockNumber;
		}
	}

	// The following statements most certainly will need to be deleted at some point...

	
	(void) MSG_RESPONSE_NA;
	(void) MSG_RESPONSE_OK;

	return 0;
}

