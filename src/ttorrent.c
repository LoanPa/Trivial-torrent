// Trivial Torrent
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>


#include "file_io.h"
#include "logger.h"



static const uint32_t MAGIC_NUMBER = 0xde1c3230;

static const uint8_t MSG_REQUEST = 0;
static const uint8_t MSG_RESPONSE_OK = 1;
static const uint8_t MSG_RESPONSE_NA = 2;

enum { RAW_MESSAGE_SIZE = 13 };


void client(struct torrent_t* const torrent);
// void server(struct torrent_t* const torrent);
void remove_file_extension(char* const downloaded_file_name, const char* const metainfo_file_name);


// Main function
int main(int argc, char **argv) { // argc: Argument counter, argv: Argument value array

    set_log_level(LOG_DEBUG);

    log_printf(LOG_INFO, "Trivial Torrent (build %s %s) by %s", __DATE__, __TIME__, "Pablo");

    bool isClient = true;
    char* metainfo_file_name;
    int PORT = 8080;


    // =====================
    //  Parse command line
    // =====================

    if (argc > 1)
    {
        if (!strcmp(argv[1], "-l")) // Server
        {
            isClient = false;
            if(argc<4)
            {
                log_message(LOG_INFO, "Missing arguments");
                exit(EXIT_FAILURE);
            }
            else if(argc>4){
                log_message(LOG_INFO, "Too many arguments");
                exit(EXIT_FAILURE);
            }
            else
            {
                PORT= atoi(argv[2]);
                metainfo_file_name=argv[3];
            }
        }
        else // Client
        {
            metainfo_file_name = argv[1];
            if(argc > 2){
                log_message(LOG_INFO, "Too many arguments");
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        log_message(LOG_INFO, "Not specified arguments");
        exit(EXIT_FAILURE);
    }



    char downloaded_file_name[strlen(metainfo_file_name)];
    remove_file_extension(downloaded_file_name, metainfo_file_name); // Remove file extension to get downloaded file name


    log_printf(LOG_DEBUG, "Metainfo file name: %s \n Downloaded file name: %s \n PORT: %d \n Is client? %d \n",
        metainfo_file_name,
        downloaded_file_name,
        PORT,
        isClient
    );

    // =====================
    //  Load metainfo file
    // =====================

    struct torrent_t torrent;

    if (create_torrent_from_metainfo_file(metainfo_file_name, &torrent, downloaded_file_name))
    {
        perror("Error creating torrent from metainfo file");
        exit(EXIT_FAILURE);
    }

    log_message(LOG_DEBUG, "Torrent loaded successfully");


    if (isClient)
    {
        client(&torrent);
    }
    else // Servidor
    {
        // server(&torrent);
    }



    return 0;
}


void client(struct torrent_t* const torrent)
{
    // Load metainfo file
    log_printf(LOG_DEBUG, "Metainfo file name: %s \n Downloaded file size: %d \n Block count: %d \n Peer count: %d \n",
        torrent->metainfo_file_name,
        torrent->downloaded_file_size,
        torrent->block_count,
        torrent->peer_count
    );

    int socketFD;
    struct sockaddr_in serverAddress;

    for (uint64_t i = 0; i < torrent->peer_count; i++) // Para cada peer en el archivo
    {
        // Comprueba si ya ha sido descargado

        bool is_downloaded = true;
        uint64_t index=0;
        
        while((index < torrent->block_count) && (is_downloaded)){
            
            if(!torrent->block_map[index]){
                
                is_downloaded=false;
            }
            index++;
        }
        if(is_downloaded){
            
            log_message(LOG_DEBUG, "File already downloaded");
            break;
        }
        
        //Conecta al peer del servidor
        
        socketFD = socket(AF_INET, SOCK_STREAM, 0);
        if (socketFD < 0) // -1 si hay error de creación de socket
        {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
        log_printf(LOG_DEBUG, "Socket %d created successfully", i + 1); // Si no falla

        memset(&serverAddress, 0, sizeof(serverAddress)); // Inicializamos la estructura de dirección de server a 0

        char peer_addr[15];
        struct peer_information_t peer = torrent->peers[i];

        sprintf(peer_addr, "%d.%d.%d.%d", // Concatenate the array peer_address and store it on the string peer_addr
            peer.peer_address[0],
            peer.peer_address[1],
            peer.peer_address[2],
            peer.peer_address[3]
        );
        
        log_printf(LOG_DEBUG, "Peer address: %s:%d",peer_addr,ntohs(torrent->peers[i].peer_port));

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr(peer_addr); //error xavi
        serverAddress.sin_port = torrent->peers[i].peer_port;

        if (connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) // Si -1 error al conectarse al servidor peer
        {
            perror("Socket connection failed");
            continue;
        }
        log_printf(LOG_DEBUG, "Socket %d connected successfully", i + 1); //Si no conexión exitosa


        for (uint64_t j = 0; j < torrent->block_count; j++) // Para cada bloque incorrecto en el archivo descargado
        {
            if (torrent->block_map[j])
              {
                  log_printf(LOG_DEBUG, "Block %d is already downloaded", j + 1);
                  continue;
              }

              uint8_t buffer[RAW_MESSAGE_SIZE];

              buffer[0] = (uint8_t)(MAGIC_NUMBER >> 0) & 0xff;
              buffer[1] = (uint8_t)(MAGIC_NUMBER >> 8) & 0xff;
              buffer[2] = (uint8_t)(MAGIC_NUMBER >> 16) & 0xff;
              buffer[3] = (uint8_t)(MAGIC_NUMBER >> 24) & 0xff;
              buffer[4] = (uint8_t)MSG_REQUEST;
              buffer[5] = (uint8_t)(j >> 0) & 0xff;
              buffer[6] = (uint8_t)(j >> 8) & 0xff;
              buffer[7] = (uint8_t)(j >> 16) & 0xff;
              buffer[8] = (uint8_t)(j >> 24) & 0xff;
              buffer[9] = (uint8_t)(j >> 32) & 0xff;
              buffer[10] = (uint8_t)(j >> 40) & 0xff;
              buffer[11] = (uint8_t)(j >> 48) & 0xff;
              buffer[12] = (uint8_t)(j >> 56) & 0xff;

            if (send(socketFD, buffer, sizeof(buffer), 0) < 0) // Enviar una solicitud al servidor (-1 = error)
            {
                perror("Send request to the server failed");
                continue;
            }
            log_printf(LOG_DEBUG, "Request %d sent successfully", j + 1);

            ssize_t block_length = recv(socketFD, buffer, sizeof(buffer), MSG_WAITALL); // Espera hasta que el servidor le envie un mensaje


            if (block_length == -1)
            {
                perror("Error receiving message from server");
                continue;
            }
            
            if (!block_length)
            {
                log_printf(LOG_DEBUG, "Any message received from server");
                continue;
            }

            log_message(LOG_DEBUG, "Message from server received");

                         if (buffer[4] == MSG_RESPONSE_OK)
                         {
                             uint64_t block_size = get_block_size(torrent, j);
                             uint8_t buffer_block[block_size];

                                    block_length = recv(socketFD, buffer_block, block_size, MSG_WAITALL); // Wait for the server to send a block

                             if (block_length == -1)
                             {
                                 perror("Error receiving block from server");
                                 continue;
                             }
                             if (block_length==0) // If the server signals the unavailablity of the block, do nothing
                             {
                                 log_printf(LOG_DEBUG, "Block %d unavailable", j + 1);
                                 continue;
                             }

                             struct block_t block = { 0 };
                             block.size = block_size;

                             for (uint64_t k = 0; k < block_size; k++)
                             {
                                 block.data[k] = buffer_block[k]; // Copy content of the buffer received to struct block_t
                             }

                             if (store_block(torrent, j, &block)) // If the server responds with the block, store it to the downloaded file
                             {
                                 perror("Store block failed");
                                 continue;
                             }
                             log_printf(LOG_DEBUG, "Block %d stored successfully", j + 1);

                         }
                         else if (buffer[4] == MSG_RESPONSE_NA)
            {
                log_printf(LOG_DEBUG, "Block %d unavailable", j + 1);
                             }
                             else
                             {
                                 log_message(LOG_DEBUG, "Error receiving message from server");
                             }
        } // End for each block

        if (close(socketFD))
        {
            perror("Error closing socket");
        }
        log_printf(LOG_DEBUG, "Socket %d closed \n", i + 1);

} // End for each peer
    

    if (destroy_torrent(torrent))
         {
             perror("Error destroying torrent");
         }
         log_message(LOG_DEBUG, "Torrent destroyed");

}

void remove_file_extension(char* const downloaded_file_name, const char* const metainfo_file_name)
{
    size_t index = strlen(metainfo_file_name);

    strcpy(downloaded_file_name, metainfo_file_name);

    while (downloaded_file_name[--index] != '.'); // Search for the index of the file extension

    downloaded_file_name[index] = '\0'; // Remove the file extension
}
