
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

//funcions

int client(char* path);
int server(char* path, char* port)
;

/**
 * Main function.
 */






int main(int argc, char **argv) {

	set_log_level(LOG_DEBUG);

	log_printf(LOG_INFO, "Trivial Torrent (build %s %s) by %s", __DATE__, __TIME__, "L. Panareda");

	// ==========================================================================
	// Parse command line
	// ==========================================================================


	// Comprovar si servidor o client
	if(argc == 2) // TODO: canviar aixo que no sigui tan cutre
	{
		return client(*(argv + 1));

	}
	// server
	else // TODO: posar això bé
	{
		server(*(argv + 3), *(argv + 2));
		
	}

	// The following statements most certainly will need to be deleted at some point...

	
	(void) MSG_RESPONSE_NA;
	(void) MSG_RESPONSE_OK;

	return 0;
}



int client(char* path)
{
	
	// TODO: Crec que és possible prescindir de la variable path

	struct torrent_t torrent;

	long int unsigned pathSize = 0;

	while (*(path + pathSize ) != '\0')
		++pathSize;

	if(pathSize <= 9)
	{
		perror("The file must be in .ttorrent format");
		return -1;
	}

	char *metainfoFileName = malloc(sizeof(char)*pathSize);
	memcpy(metainfoFileName, path, pathSize);

	long int unsigned count = 9; // TODO: while buscant el "."

	pathSize -= count; // points to the '.' in anystr.ttorrent
	char *fileName = malloc(sizeof(char)*pathSize);
	memcpy(fileName, path, pathSize);

	//1. Load a metainfo file
	if (create_torrent_from_metainfo_file (metainfoFileName, &torrent ,fileName ) == -1)
	{
		perror("Could not create torrent from metainfo file");
		return -1;
	}
	
	
	struct sockaddr_in sockaddress_peer;
	memset(&sockaddress_peer, '\0', sizeof(struct sockaddr_in));
	sockaddress_peer.sin_family = AF_INET;
	
	
	
	
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
		log_message(LOG_INFO, "Checking if file is already on disk...");
		while ((missingBlockNumber < torrent.block_count) && (fileDownloaded == 1))
		{
			if (torrent.block_map[missingBlockNumber] == 0)
				fileDownloaded = 0;
			else
				++missingBlockNumber;
		}
		if(fileDownloaded == 1)
		{
			log_message(LOG_INFO, "File already downloaded, no point in continuing");
			return 0;//Good ending
		}
		else
			log_message(LOG_INFO, "File not found on disk, proceding to download it");

		// Fi del file check


		int sock = socket(AF_INET, SOCK_STREAM, 0);


		if (sock == -1)
		{
			perror("Error: socket could not be created");
			return -1;
		}
		
		
		
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
		

		
		//Per cada peer comprovem cada bloc que ens falta
		for (uint64_t block_number = 0; block_number < torrent.block_count; block_number++)
		{		/* Per cada block que no tenim*/

			if( torrent.block_map[block_number] == 0 )
			{
				message[4] = MSG_REQUEST;

				//Network byte order = Big endian
				// Big endian -> Més significatiu primer

				message[5] = (uint8_t) (block_number >> 56) & 0xff;
				message[6] = (uint8_t) (block_number >> 48) & 0xff;
				message[7] = (uint8_t) (block_number >> 40) & 0xff;
				message[8] = (uint8_t) (block_number >> 32) & 0xff;
				message[9] = (uint8_t) (block_number >> 24) & 0xff;
				message[10] = (uint8_t) (block_number >> 16) & 0xff;
				message[11] = (uint8_t) (block_number >> 8) & 0xff;
				message[12] = (uint8_t) (block_number) & 0xff;
				
				if (send(sock, message, sizeof(message), 0) == -1)
				{
						/* 
						*	No estic segur si s'hauria de provar amb un altre bloc o directament canviar de peer
						* Considerarem la segona opció
						*/
					perror("Error: send() function exited with code -1");
					break;
				}
				
				
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


				if (message[4] == MSG_RESPONSE_OK)
				{
					
					struct block_t recvd_block;
					
					
					uint64_t expected_block_length = get_block_size(&torrent, block_number);

					//Buffer per a contenir el bloc
					uint8_t data_message[expected_block_length];

					if (recv(sock, data_message, expected_block_length , 0) <= 0)
					{
						perror("Error: 2nd recv() function exited with code -1");
						continue; //Provem amb un altre bloc però amb el mateix peer
					}
					
					
					//Ara que sabem que el recv no els dona error fem l'assignació
					recvd_block.size = expected_block_length;
					
					
					for (uint64_t i = 0; i < recvd_block.size; i++)
					{
						recvd_block.data[i] = data_message[i];
					}
					
					//Assignem a recvd_block el contingut de data_message
					//memcpy(recvd_block.data, data_message, recvd_block.size);
					
					
					
					
					/*
					PER ON ANAVA:
					tenim aquest error: ttorrent: src/file_io.c:438: store_block: Assertion `block->size > 0' failed.
					i és perque el downloaded_block no està inicialitzat i hauria de contenir el bloc rebut.
					
					També crec que s'hauria de fer un altre buffer a part del message que contingues el bloc 
					
					
					Acabo d'adonar-me que primer es rep la confirmació de que el server te el bloc i després s'envia el bloc
					*/
					
					if (store_block(&torrent, block_number, &recvd_block) == -1)
						perror("Error: block was not stored correctly");	
					else
					{
						torrent.block_map[block_number] = 1;
						log_printf(LOG_INFO, "Block %d stored successfully :-D", block_number);
					}
											
				}// if del MSG_RESPONSE_OK
			}// Aquí acaba el if de els blocs que no tenim
		}// Aquí acaba el for dels blocs

		
		if(close(sock) == -1)
		{
			perror("Error: close() function exited with code -1");
			return -1;
		}
	
	}
	/*
	free(metainfoFileName);
	free(fileName);
	*/
	destroy_torrent(&torrent);
	return 0;
}

int server(char* path, char* port)
{
	//TODO: tota aquesta part s'hauria de posar en una funció pròpia

	//torrent contains all the information about the torrent
	struct torrent_t torrent;

	long int unsigned pathSize = 0;

	while (*(path + pathSize ) != '\0')
		++pathSize;

	if(pathSize <= 9)
	{
		log_message(LOG_INFO, "The file must be in .ttorrent format");
		return -1;
	}

	// TODO: FIX MEMORY LEAKS!

	char *metainfoFileName = malloc(sizeof(char)*pathSize);
	memcpy(metainfoFileName, path, pathSize);

	long int unsigned count = 9; // TODO: while buscant el "."

	pathSize-=count; // points to the '.' in anystr.ttorrent
	char *fileName = malloc(sizeof(char)*pathSize);
	memcpy(fileName, path, pathSize);

	//1. Load a metainfo file
	if (create_torrent_from_metainfo_file (metainfoFileName, &torrent ,fileName ) == -1)
	{
		log_message(LOG_INFO, "Could not load metainfo file correctly");
		return -1;
	}



	log_message(LOG_INFO, "Checking disck file...");
	if (1)
	{
		uint64_t number_of_blocks = 0;
		for (uint64_t block_number = 0; block_number < torrent.block_count; block_number++)
		{
			if (torrent.block_map[block_number])
			{
				log_printf(LOG_INFO, "Block number %d available", block_number);
				++number_of_blocks;
			}
			else
				log_printf(LOG_INFO, "Block number %d not available", block_number);
		}
		log_printf(LOG_INFO, "%d/%d blocks available", number_of_blocks, torrent.block_count);
	}


	// TODO: tot això  ⤵
	/*
	+	1. Load a metainfo file (functionality is already implemented in the file_io API through the create_torrent_from_-
	+		metainfo_file function).
	+			a. Check for the existence of the associated downloaded file.
	+			b. Check which blocks are correct using the SHA256 hashes in the metainfo file.
	+	2. Forever listen to incoming connections, and for each connection:
	+			a. Wait for a message.
	+			b. If a message requests a block that can be served (correct hash), respond with the appropriate message,
	-				followed by the raw block data.
	+			c. Otherwise, respond with a message signaling the unavailability of the block.			
	*/

	struct sockaddr_in sockaddress, s1address;
	memset(&sockaddress, '\0', sizeof(struct sockaddr_in));
	memset(&s1address, '\0', sizeof(struct sockaddr_in));
	sockaddress.sin_family = AF_INET;
	sockaddress.sin_addr.s_addr = INADDR_ANY;
	sockaddress.sin_port = htons((uint16_t) atoi(port));


	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1)
	{
		perror("Error: socket could not be created");
		return -1;
	}

	if (bind( sock, (struct sockaddr *) &sockaddress, sizeof(struct sockaddr_in) ) == -1)
	{
		perror("Error: bind() function exited with code -1");
		return -1;
	}

	if (listen(sock, SOMAXCONN))
	{
		perror("Error: listen() function exited with code -1");
		return -1;
	}
	
	uint8_t message[RAW_MESSAGE_SIZE];

	
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int s1;
	for(;;) //Forever listen to incoming connections
	{
		
		s1 = accept(sock, (struct sockaddr *) &s1address, &addrlen);

		if(s1 == -1)
		{
			perror("Error: accept() function exited with code -1");
			log_printf(LOG_INFO, "errno = %d", errno);
			return -1;
		}
		
		int pid = fork();
		if(pid == -1)
		{
			perror("Error: fork() function exited with code -1");
			return -1;
		}
		
		if (pid > 0)	/* parent process */
		{
			log_message(LOG_DEBUG, "Parent process");
			
			if(close(s1 == -1))
			{
				perror("Error: close(s1) exited with code -1");
				log_printf(LOG_INFO, "errno = %d", errno);

				return -1;
			}
			
			continue; // seguim amb el for
		}
		
		if(pid == 0)
		{
			log_printf(LOG_DEBUG, "Child process");
			
			if(close(sock) == -1)
			{
				perror("Error: close(sock) exited with code -1");
				log_printf(LOG_INFO, "errno = %d", errno);
				return -1;
			}
			log_printf(LOG_DEBUG, "sock closed succesfully");
			
			
		
			while(s1)
			{
				uint64_t block_number = 0;

				if(recv(s1, message, RAW_MESSAGE_SIZE, 0) != RAW_MESSAGE_SIZE)
				{
					log_printf(LOG_INFO, "Client closed the connection");
					exit(0);
				}
				log_printf(LOG_DEBUG, "recv()'d successfully");
				
				if(message[4] == MSG_REQUEST)	// Si el client demana un bloc, carreguem el número del bloc que demana
					for (uint8_t i = 5; i < 13; i++)
					{
						block_number <<= 8;
						block_number |= message[i];
						log_printf(LOG_DEBUG, "Loading block number %d...", block_number);
					}
				log_printf(LOG_DEBUG, "Block number %d loaded", block_number);

				log_printf(LOG_DEBUG, "Block number %d", torrent.block_map[block_number]);
				if (torrent.block_map[block_number] == 0)
				{
					message[4] = MSG_RESPONSE_NA;
					log_message(LOG_INFO, "Block not available :-(");
				}
				else
				{
					message[4] = MSG_RESPONSE_OK;
					log_message(LOG_INFO, "Block available :-)");
				}

				log_message(LOG_DEBUG, "Preparing to send() the answer");
				if(send(s1, message, RAW_MESSAGE_SIZE, 0) == -1)//Aquí només diem si tenim o no el bloc
				{
					perror("Error:  1st send() function exited with code -1");
					log_printf(LOG_INFO, "errno = %d", errno);
					return -1;
				}
				log_message(LOG_DEBUG, "El primer send() funciona");

				if (torrent.block_map[block_number] == 0)
				{
					log_printf(LOG_DEBUG, "Block %d not available", block_number);
					continue; // block not available
				}
	
				struct block_t requested_block;
				load_block(&torrent,block_number, &requested_block);

				if(send(s1, requested_block.data, requested_block.size, 0) == -1)//Ara enviem tot el bloc
				{
					perror("Error: 2nd send() function exited with code -1");
					log_printf(LOG_INFO, "errno = %d", errno);
					return -1;
				}
			}

			exit(0);
		}
		

		





	}//End of the listen for

	return 0;
}
