//***************************************************************************//
//* File Name: network.hpp                                                  *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 4/5/03                                                       *//
//* File Desc: Networking functionality for multi-player game.              *//
//*            Uses UDP protocol.                                           *//
//* Rev. Date: 3/23/05                                                      *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifdef NETWORK
#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__

#ifdef UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
typedef int                  SOCKET;
typedef struct sockaddr_in   SOCKADDR_IN;
#else
#include <winsock.h>
#endif
#include <stdio.h>
#include <stdlib.h>

// Network port.
#define NETWORK_PORT                   4507

// Quantities.
#define MAX_PLAYERS                    3
#define HOST_NAME_SIZE                 50
#define PLAYER_NAME_SIZE               50
#define MAX_MASTER_PAYLOAD             2048
#define MAX_SLAVE_PAYLOAD              256
#define NETWORK_STATUS_MESSAGE_SIZE    100

// Time-out for message (ms).
#define MSG_WAIT                       1000
#define MSG_RETRY                      10
#define MAX_MSG_TIME_OUTS              5

// Exit delay.
#define EXIT_DELAY                     1000

class Network
{
public:

   // Exit status.
   typedef enum
   {
      WINNER, KILLED, QUIT
   } EXIT_STATUS;

   // Constructor.
   Network()
   {
      for (int i = 0; i < MAX_PLAYERS; i++)
      {
         currentPlayers[i] = false;
         playerTimeouts[i] = 0;
      }
      master         = newMaster = false;
      masterSynch    = slaveSynch = false;
      masterTimeouts = 0;
      terminated     = false;
   }


   // Destructor.
   ~Network()
   {
      shutdown(mySocket, 2);
#ifdef UNIX
      close(mySocket);
#else
      closesocket(mySocket);
      WSACleanup();
#endif
   }


   // Player is master vs. slave.
   bool master;

   // Set when slave is appointed new master.
   bool newMaster;

   // Set for master/slave synchronization.
   // This happens when a new player joins or
   // a packet is lost.
   bool masterSynch;
   bool slaveSynch;

   // Initialize.
   bool initMaster(char *playerName = NULL);
   bool initSlave(char *masterHost, char *playerName = NULL);

   // Update master/slave game state.
   // Messages are sent/received in the buffers below.
   // Application must load/unload the payloads using myIndex.
   bool getMaster();
   bool sendMaster();
   bool getSlave();
   bool sendSlave();

   struct MASTER_PAYLOAD
   {
      int           size;
      unsigned char data[MAX_MASTER_PAYLOAD];
   }
   masterPayload;

   struct SLAVE_PAYLOAD
   {
      int           size;
      unsigned char data[MAX_SLAVE_PAYLOAD];
   }
   slavePayloads[MAX_PLAYERS];

   // Player exit.
   bool exitNotify(EXIT_STATUS);

   // Messaging functions.
   bool setupMyAddress();
   bool setupMasterAddress();
   bool sendMessage();
   bool getMessage(bool wait);
   bool isMyAddr(SOCKADDR_IN addr);

   // Network connections.
   SOCKADDR_IN playerAddrs[MAX_PLAYERS];
   bool        currentPlayers[MAX_PLAYERS];
   int         playerTimeouts[MAX_PLAYERS];
   int         masterTimeouts;
   SOCKADDR_IN myAddr, masterAddr;
   SOCKET      mySocket;
   int         myIndex, masterIndex;
   char        masterHost[HOST_NAME_SIZE + 1];
   char        playerName[PLAYER_NAME_SIZE + 1];

   // Structure to store info returned from WSAStartup.
#ifndef UNIX
   WSADATA wsda;
#endif

   // Message types.
   typedef enum
   {
      INIT, INIT_ACK, PLAYER_EXIT, MASTER_INFO, SLAVE_INFO, TIME_OUT
   } MESSAGE_TYPE;

   // INIT message.
   struct INIT_MSG
   {
      char playerName[PLAYER_NAME_SIZE + 1];
   };

   // INIT_ACK message.
   typedef enum
   {
      ACCEPT, REFUSE, NO_CAPACITY, REDIRECT
   } INIT_STATUS;

   struct INIT_ACK_MSG
   {
      INIT_STATUS status;
      int         playerIndex;
      int         masterIndex;
      char        masterName[PLAYER_NAME_SIZE + 1];
      char        redirectHost[HOST_NAME_SIZE + 1];
   };

   // PLAYER_EXIT message.
   struct PLAYER_EXIT_MSG
   {
      int         playerIndex;
      int         status;

      // If master is exiting, the following allows
      // remaining players to determine new master:
      // the lowest numbered remaining player.
      bool        currentPlayers[MAX_PLAYERS];
      SOCKADDR_IN addresses[MAX_PLAYERS];
   };

   // MASTER_INFO message.
   struct MASTER_INFO_MSG
   {
      int                   masterIndex;
      bool                  synchCmd;             // Synchronization command.
      struct MASTER_PAYLOAD payload;
   };

   // SLAVE_INFO message.
   struct SLAVE_INFO_MSG
   {
      int                  playerIndex;
      bool                 synchReq;              // Synchronization request.
      struct SLAVE_PAYLOAD payload;
   };

   // Message buffer
   struct MESSAGE
   {
      MESSAGE_TYPE type;
      union
      {
         struct INIT_MSG        initMsg;
         struct INIT_ACK_MSG    initAckMsg;
         struct PLAYER_EXIT_MSG exitMsg;
         struct MASTER_INFO_MSG masterMsg;
         struct SLAVE_INFO_MSG  slaveMsg;
      }
                   common;
   }
   message;

   // From/to address.
   SOCKADDR_IN messageAddr;

   // Status.
   typedef enum { OK, INFO, WARNING, FATAL }
   STATUS;
   STATUS status;
   char   statusMessage[NETWORK_STATUS_MESSAGE_SIZE + 1];
   bool   terminated;
};
#endif                                            // #ifndef __NETWORK_HPP__
#endif                                            // #ifdef NETWORK
