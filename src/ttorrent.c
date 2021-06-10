
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
static const uint16_t MAX_PORT_NUMBER = 65535;
static const uint16_t MAX_PATH_SIZE = 4069;
static const uint16_t MAX_WELL_KNOWN_PORT = 1023;


enum { 	RAW_MESSAGE_SIZE = 13 };

//funcions

int set_torrent( char *metainfo_file_path, struct torrent_t * const torrent);
int client(struct torrent_t torrent);
int server(struct torrent_t torrent, uint16_t const port);


int main(int argc, char **argv) {

	set_log_level(LOG_DEBUG);

	log_printf(LOG_INFO, "Trivial Torrent (build %s %s) by %s", __DATE__, __TIME__, "L. Panareda");

	// ==========================================================================
	// Parse command line
	// ==========================================================================


	// Comprovar si servidor, client o error
	if(argc == 2) // Client
	{
		struct torrent_t torrent;
		
		if(set_torrent(*(argv + 1), &torrent) == -1)
			return -1;

		return client(torrent);
	}
	if(argc == 4) // Server
	{
		int port = atoi(*(argv + 2));

		if ((*(*(argv + 1)) != '-') || (*(*(argv + 1) + 1 ) != 'l'))
		{
			perror("ERROR: Usage: ttorrent [-l port] file.ttorrent \n Usage: ttorrent file.ttorrent");
			return -1;
		}
		
		if(port > MAX_PORT_NUMBER)
		{
			perror("ERROR: Invalid port");
			return -1;	
		}

		if(port < MAX_WELL_KNOWN_PORT)
		{
			perror("ERROR: Port is a well-known port, please use a port in the range [1024, 65535]");
			return -1;	
		}
		struct torrent_t torrent;
		
		if(set_torrent(*(argv + 3), &torrent) == -1)
			return -1;

		return server(torrent, (uint16_t) port);
	}
	if (argc < 4)
	{
		errno = E2BIG;
		perror("Error: too many arguments! \n Usage: ttorrent [-l port] file.ttorrent \n Usage: ttorrent file.ttorrent");
		return -1;
	} 
	else
	{
		perror("ERROR: Usage: ttorrent [-l port] file.ttorrent \n Usage: ttorrent file.ttorrent");
		return -1;

	}
	return 0;
}



int client(struct torrent_t torrent)
{
	
		
	struct sockaddr_in sock_address_peer;
	memset(&sock_address_peer, '\0', sizeof(struct sockaddr_in));
	sock_address_peer.sin_family = AF_INET;
	
	
	
	
	// Declaracio message
	uint8_t message[RAW_MESSAGE_SIZE];

	message[0] = (uint8_t) (MAGIC_NUMBER);
	message[1] = (uint8_t) (MAGIC_NUMBER >> 8);	
	message[2] = (uint8_t) (MAGIC_NUMBER >> 16);
	message[3] = (uint8_t) (MAGIC_NUMBER >> 24);
	//message[4] = MSG_REQUEST;
	


		
	
	for (uint64_t peer_number = 0; peer_number < torrent.peer_count; peer_number++) /* 2. For each server peer in the metainfo file: */
	{
		log_printf(LOG_DEBUG, "Checking peer %d", peer_number);

		// a. Check for the existence of the associated downloaded file.
		uint64_t missing_block_number = 0;
		int is_file_downloaded = 1;
		log_message(LOG_INFO, "Checking if file is already on disk...");

		while ((missing_block_number < torrent.block_count) && (is_file_downloaded == 1)) //checking which blocks are already stored on the disk
		{
			if (torrent.block_map[missing_block_number] == 0)
				is_file_downloaded = 0;
			else
				++missing_block_number;
		}
		if(is_file_downloaded == 1)
		{
			log_message(LOG_INFO, "File already downloaded, no point in continuing");
			return 0; //Good ending
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
		log_printf(LOG_DEBUG, "Socket created");
		
		
		// Definim port
		sock_address_peer.sin_port = (torrent.peers + peer_number)->peer_port;
		log_printf(LOG_DEBUG, "Port: %d", sock_address_peer.sin_port);
		
		
		// Definim adreça
		sock_address_peer.sin_addr.s_addr = (torrent.peers + peer_number)->peer_address[3];
		sock_address_peer.sin_addr.s_addr = sock_address_peer.sin_addr.s_addr << 8;
		sock_address_peer.sin_addr.s_addr |= (torrent.peers + peer_number)->peer_address[2];
		sock_address_peer.sin_addr.s_addr = sock_address_peer.sin_addr.s_addr << 8;
		sock_address_peer.sin_addr.s_addr |= (torrent.peers + peer_number)->peer_address[1];
		sock_address_peer.sin_addr.s_addr = sock_address_peer.sin_addr.s_addr << 8;
		sock_address_peer.sin_addr.s_addr |= (torrent.peers + peer_number)->peer_address[0];
		
		log_printf(LOG_DEBUG, "Address: %d", sock_address_peer.sin_addr.s_addr);



		if(connect(sock, (struct sockaddr *) &sock_address_peer, sizeof( struct sockaddr)) == -1)
		{
			perror("Error: Connect() function exited with code -1");
			continue; //No es pot conectar, passem al seguent peer
		}
		log_printf(LOG_DEBUG, "Connected successfully");


		
		//Per cada peer comprovem cada bloc que ens falta
		for (uint64_t block_number = 0; block_number < torrent.block_count; block_number++)
		{		/* Per cada block que no tenim*/
			log_printf(LOG_DEBUG, "Bloack number: %d", block_number);
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
						* No estic segur si s'hauria de provar amb un altre bloc o directament canviar de peer
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

					if (recv(sock, data_message, expected_block_length , MSG_WAITALL) <= 0)
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
					
					if (store_block(&torrent, block_number, &recvd_block) == -1)
					{
						perror("Error: block was not stored correctly");	
						log_printf(LOG_INFO, "errno = %d", errno);
					}
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
	free(metainfo_file_name);
	free(file_name);
	*/
	destroy_torrent(&torrent);
	return 0;
}

int server(struct torrent_t torrent, uint16_t const port)
{
	



	log_message(LOG_INFO, "Checking disck file...");
	if (1) //this if is used so number_of_blocks is declared only within this scope
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



	struct sockaddr_in sock_address, s1_address;
	memset(&sock_address, '\0', sizeof(struct sockaddr_in));
	memset(&s1_address, '\0', sizeof(struct sockaddr_in));
	sock_address.sin_family = AF_INET;
	sock_address.sin_addr.s_addr = INADDR_ANY;
	sock_address.sin_port = htons(port);


	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1)
	{
		perror("Error: socket could not be created");
		return -1;
	}

	if (bind( sock, (struct sockaddr *) &sock_address, sizeof(struct sockaddr_in) ) == -1)
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

	
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int s1;
	for(;;) //Forever listen to incoming connections
	{
		
		s1 = accept(sock, (struct sockaddr *) &s1_address, &addr_len);

		if(s1 == -1)
		{
			log_printf(LOG_INFO, "Preparing to accept...");
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
			
			
		
			while(1)
			{
	
				log_printf(LOG_DEBUG, "Dins el while");
				uint64_t block_number = 0;

				if(recv(s1, message, RAW_MESSAGE_SIZE, 0) != RAW_MESSAGE_SIZE)
				{
					log_printf(LOG_INFO, "Client closed the connection");
					errno = 0;

					exit(0);
				}
				log_printf(LOG_DEBUG, "recv()'d successfully");
				
				if(message[4] == MSG_REQUEST)	// Si el client demana un bloc, carreguem el número del bloc que demana
					for (uint8_t i = 5; i < 13; i++)
					{
						block_number <<= 8;
						block_number |= message[i];
					}
				log_printf(LOG_DEBUG, "Block number %d loaded", block_number);

				log_printf(LOG_DEBUG, "Block availability: ", torrent.block_map[block_number]);
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


int set_torrent( char * metainfo_file_path, struct torrent_t * const torrent)
{
	
	uint16_t path_size = 0;

	while (*(metainfo_file_path + path_size ) != '\0')
	{
		++path_size;
		if(path_size > MAX_PATH_SIZE)
		{
			perror("Error: File path is too long!");
			errno = ENAMETOOLONG;
			return -1;
		}
	}

	// points to the '.' in anystr.ttorrent
	uint16_t  dot_position = path_size;
	
	while(( *(metainfo_file_path + dot_position) != '.') && (dot_position > 0))
		dot_position--; 

	if (strcmp(metainfo_file_path + dot_position, ".ttorrent") != 0)
	{
		perror("Error: The file must be in .ttorrent format");
		return -1;
	}

	// file_path is the same as metainfo_file_path until the last "."
	char *file_path = malloc(sizeof(char) * dot_position);
	memcpy(file_path, metainfo_file_path, dot_position);

	//1. Load a metainfo file
	if (create_torrent_from_metainfo_file (metainfo_file_path, torrent ,file_path ) == -1)
	{
		perror("Could not create torrent from metainfo file");
		return -1;
	}
	return 0;
}