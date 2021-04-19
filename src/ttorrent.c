
// Trivial Torrent

// TODO: some includes here

#include "file_io.h"
#include "logger.h"

// TODO: hey!? what is this?

/**
 * This is the magic number (already stored in network byte order).
 * See https://en.wikipedia.org/wiki/Magic_number_(programming)#In_protocols
 */
static const uint32_t MAGIC_NUMBER = 0xde1c3231; // = htonl(0x31321cde);

static const uint8_t MSG_REQUEST = 0;
static const uint8_t MSG_RESPONSE_OK = 1;
static const uint8_t MSG_RESPONSE_NA = 2;

enum { RAW_MESSAGE_SIZE = 13 };


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



			long int unsigned pathSize =0;

			while(*(*(argv + 1) + pathSize ) != '\0')
				++pathSize;

			if(pathSize <= 9)
			{
				log_message(LOG_INFO, "Incorrect file name");
				return -1;
			}

			char *metainfoFileName = malloc(sizeof(char)*pathSize);
			memcpy(metainfoFileName, *(argv + 1), pathSize);

			pathSize-=9; // points to the '.' in anystr.ttorrent
			char *fileName = malloc(sizeof(char)*pathSize);
			memcpy(fileName, *(argv + 1), pathSize);


		if (create_torrent_from_metainfo_file (metainfoFileName, &torrent ,fileName ) == -1)
		{
			log_message(LOG_INFO, "Could not create torrent from metainfo file");
			return -1;
		}

		// 1.a Check for the existence of the associated downloaded file.//

		// 1.b  Check which blocks are correct using the SHA256 hashes in the metainfo file.//
			// verify_block (const struct block_t * const block, const sha256_hash_t target_digest)

	}

	// The following statements most certainly will need to be deleted at some point...

	(void) argc;
	(void) argv;
	(void) MAGIC_NUMBER;
	(void) MSG_REQUEST;
	(void) MSG_RESPONSE_NA;
	(void) MSG_RESPONSE_OK;

	return 0;
}
