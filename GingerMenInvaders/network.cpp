//***************************************************************************//
//* File Name: network.cpp                                                  *//
//* Author:    Tom Portegys, portegys@ilstu.edu                             *//
//* Date Made: 4/5/03                                                       *//
//* File Desc: Networking functionality for multi-player game.              *//
//*            Uses UDP protocol.                                           *//
//* Rev. Date: 3/23/05                                                      *//
//* Rev. Desc:                                                              *//
//*                                                                         *//
//***************************************************************************//

#ifdef NETWORK
#include "network.hpp"

// Initialize master.
bool Network::initMaster(char *playerName)
{
   // Terminated?
   if (terminated)
   {
      return(false);
   }

   // OK so far.
   status           = OK;
   statusMessage[0] = '\0';

   // Save player name.
   if (playerName != NULL)
   {
      strncpy(this->playerName, playerName, strlen(playerName));
   }
   else
   {
      strcpy(this->playerName, "Unknown");
   }

   // Set master status.
   master = true;

#ifndef UNIX
   // Load version 1.1 of Winsock
   WSAStartup(MAKEWORD(1, 1), &wsda);
#endif

   // Set up my networking.
   if (!setupMyAddress())
   {
      return(false);
   }

   // Master plays until slaves connect.
   myIndex = masterIndex = 0;
   currentPlayers[myIndex] = true;

   return(true);
}


// Initialize slave.
bool Network::initSlave(char *masterHost, char *playerName)
{
   // Terminated?
   if (terminated)
   {
      return(false);
   }

   // OK so far.
   status           = OK;
   statusMessage[0] = '\0';

   // Save player name.
   if (playerName != NULL)
   {
      strncpy(this->playerName, playerName, strlen(playerName));
   }
   else
   {
      strcpy(this->playerName, "Unknown");
   }

   // Save master host name.
   strncpy(this->masterHost, masterHost, strlen(masterHost));

   // Set slave status.
   master  = false;
   myIndex = 0;

#ifndef UNIX
   // Load version 1.1 of Winsock
   WSAStartup(MAKEWORD(1, 1), &wsda);
#endif

   // Set up my networking.
   if (!setupMyAddress())
   {
      return(false);
   }

   // Address master.
   if (!setupMasterAddress())
   {
      return(false);
   }

   // Cannot be master and slave simultaneously.
   if (isMyAddr(masterAddr))
   {
      master  = true;
      myIndex = masterIndex = 0;
      currentPlayers[myIndex] = true;
      strcpy(statusMessage, "Cannot connect to self: continuing as master");
      status = INFO;
      return(true);
   }
   else
   {
      if (status == FATAL)
      {
         return(false);
      }
   }

   // Connect to master.
   while (true)
   {
      messageAddr  = masterAddr;
      message.type = INIT;
      strncpy(message.common.initMsg.playerName, this->playerName, PLAYER_NAME_SIZE);
      message.common.initMsg.playerName[PLAYER_NAME_SIZE] = '\0';
      if (!sendMessage())
      {
         return(false);
      }
      if (!getMessage(true))
      {
         return(false);
      }

      if (message.type == INIT_ACK)
      {
         // Redirect to true master?
         if (message.common.initAckMsg.status == REDIRECT)
         {
            strncpy(this->masterHost, message.common.initAckMsg.redirectHost, HOST_NAME_SIZE);
            if (!setupMasterAddress())
            {
               return(false);
            }
            continue;
         }
         if (message.common.initAckMsg.status == ACCEPT)
         {
            // Use assigned player number.
            myIndex = message.common.initAckMsg.playerIndex;
            currentPlayers[myIndex]     = true;
            masterIndex                 = message.common.initAckMsg.masterIndex;
            currentPlayers[masterIndex] = true;
            masterTimeouts              = 0;
            sprintf(statusMessage, "Connection accepted by %s", message.common.initAckMsg.masterName);
            status = INFO;
            break;
         }
         if (message.common.initAckMsg.status == REFUSE)
         {
            master  = true;
            myIndex = masterIndex = 0;
            currentPlayers[myIndex] = true;
            strcpy(statusMessage, "Connection refused: continuing as master");
            status = INFO;
            break;
         }
         if (message.common.initAckMsg.status == NO_CAPACITY)
         {
            master  = true;
            myIndex = masterIndex = 0;
            currentPlayers[myIndex] = true;
            strcpy(statusMessage, "Cannot add new player: continuing as master");
            status = INFO;
            break;
         }
      }
      if (message.type == TIME_OUT)
      {
         master  = true;
         myIndex = masterIndex = 0;
         currentPlayers[myIndex] = true;
         strcpy(statusMessage, "Connection attempt timed-out: continuing as master");
         status = INFO;
         break;
      }
      master  = true;
      myIndex = masterIndex = 0;
      currentPlayers[myIndex] = true;
      sprintf(statusMessage, "Unexpected message type=%d arrived: continuing as master", message.type);
      status = INFO;
      break;
   }
   return(true);
}


// Get state of master.
bool Network::getMaster()
{
   int  i;
   bool gotMaster;

   // Terminated?
   if (terminated)
   {
      return(false);
   }

   status           = OK;
   statusMessage[0] = '\0';
   masterSynch      = slaveSynch = false;

   gotMaster = false;
   while (true)
   {
      if (!gotMaster)
      {
         // Expecting master info.
         if (!getMessage(true))
         {
            return(false);
         }
      }
      else
      {
         // Check for inits and straggler messages.
         if (!getMessage(false))
         {
            return(false);
         }
         if (message.type == TIME_OUT)
         {
            return(true);
         }
      }

      switch (message.type)
      {
      case MASTER_INFO:
         if (!newMaster && gotMaster)
         {
            // Straggler - request re-synch.
            slaveSynch = true;
         }
         gotMaster = true;

         // Mastership might change.
         masterTimeouts = 0;
         masterSynch    = message.common.masterMsg.synchCmd;
         if (!newMaster)
         {
            masterIndex = message.common.masterMsg.masterIndex;
            masterAddr  = messageAddr;
         }
         masterPayload.size = message.common.masterMsg.payload.size;
         memcpy(masterPayload.data, message.common.masterMsg.payload.data, masterPayload.size);
         break;

      case INIT:
         // Redirect request to master.
         message.type = INIT_ACK;
         message.common.initAckMsg.status = REDIRECT;
         strncpy(message.common.initAckMsg.redirectHost,
                 inet_ntoa(masterAddr.sin_addr), HOST_NAME_SIZE);
         if (!sendMessage())
         {
            return(false);
         }
         break;

      case PLAYER_EXIT:
         // Master assigning me as new master.
         for (i = 0; i < MAX_PLAYERS; i++)
         {
            if (i == myIndex)
            {
               continue;
            }
            playerAddrs[i]    = message.common.exitMsg.addresses[i];
            currentPlayers[i] = message.common.exitMsg.currentPlayers[i];
         }

         // Assume mastership.
         masterAddr  = playerAddrs[myIndex];
         masterIndex = myIndex;
         master      = newMaster = true;
         break;

      // Assume message lost.
      case TIME_OUT:
         if (newMaster)
         {
            masterTimeouts = 0;
            return(true);
         }
         masterTimeouts++;
         if (masterTimeouts >= MAX_MSG_TIME_OUTS)
         {
            masterTimeouts = 0;
            masterAddr     = playerAddrs[myIndex];
            masterIndex    = myIndex;
            master         = true;
            for (i = 0; i < MAX_PLAYERS; i++)
            {
               currentPlayers[i] = false;
            }
            currentPlayers[myIndex] = true;
            strcpy(statusMessage, "Connection timed-out: continuing as master");
            status    = INFO;
            newMaster = true;
         }
         else
         {
            // Request re-synchronization.
            slaveSynch = true;
         }
         return(true);
      }
   }
   return(false);
}


// Send state of master to slaves.
bool Network::sendMaster()
{
   // Terminated?
   if (terminated)
   {
      return(false);
   }

   status           = OK;
   statusMessage[0] = '\0';

   // Send update to slaves.
   message.type = MASTER_INFO;
   message.common.masterMsg.masterIndex  = myIndex;
   message.common.masterMsg.synchCmd     = masterSynch;
   message.common.masterMsg.payload.size = masterPayload.size;
   memcpy(message.common.masterMsg.payload.data, masterPayload.data, masterPayload.size);
   for (int i = 0; i < MAX_PLAYERS; i++)
   {
      if (currentPlayers[i] && (i != myIndex))
      {
         messageAddr = playerAddrs[i];
         if (!sendMessage())
         {
            return(false);
         }
      }
   }
   newMaster = false;
   return(true);
}


// Get state of slaves.
bool Network::getSlave()
{
   int  i, count;
   bool needInfo[MAX_PLAYERS];

   // Terminated?
   if (terminated)
   {
      return(false);
   }

   status           = OK;
   statusMessage[0] = '\0';
   masterSynch      = slaveSynch = false;

   // How many slaves?
   for (i = count = 0; i < MAX_PLAYERS; i++)
   {
      if (currentPlayers[i] && (i != myIndex))
      {
         needInfo[i] = true;
         count++;
      }
      else
      {
         needInfo[i] = false;
      }
   }

   while (true)
   {
      if (count > 0)
      {
         // Expecting slave info.
         if (!getMessage(true))
         {
            return(false);
         }
      }
      else
      {
         // Check for inits and straggler messages.
         if (!getMessage(false))
         {
            return(false);
         }
         if (message.type == TIME_OUT)
         {
            return(true);
         }
      }

      switch (message.type)
      {
      case SLAVE_INFO:
         if (count == 0)
         {
            // Straggler message arrived; must re-synch.
            masterSynch = true;
         }
         else
         {
            count--;
         }
         i = message.common.slaveMsg.playerIndex;
         if (!currentPlayers[i])
         {
            break;
         }
         if (message.common.slaveMsg.synchReq)
         {
            // Slave request re-synch.
            masterSynch = slaveSynch = true;
         }
         needInfo[i]           = false;
         slavePayloads[i].size = message.common.slaveMsg.payload.size;
         memcpy(slavePayloads[i].data, message.common.slaveMsg.payload.data, message.common.slaveMsg.payload.size);
         playerTimeouts[i] = 0;
         break;

      case INIT:
         // New player request.
         message.type = INIT_ACK;
         for (i = 0; i < MAX_PLAYERS; i++)
         {
            if (!currentPlayers[i])
            {
               break;
            }
         }
         if (i < MAX_PLAYERS)
         {
            message.common.initAckMsg.status      = ACCEPT;
            message.common.initAckMsg.playerIndex = i;
            message.common.initAckMsg.masterIndex = masterIndex;
            strncpy(message.common.initAckMsg.masterName, playerName, PLAYER_NAME_SIZE);
            message.common.initAckMsg.masterName[PLAYER_NAME_SIZE] = '\0';
            if (!sendMessage())
            {
               return(false);
            }
            currentPlayers[i] = true;
            playerTimeouts[i] = 0;
            playerAddrs[i]    = messageAddr;
            needInfo[i]       = true;
            count++;
            sprintf(statusMessage, "Accepted player %s", message.common.initMsg.playerName);
            status      = INFO;
            masterSynch = true;                   // Re-synchronize.
         }
         else
         {
            message.common.initAckMsg.status = NO_CAPACITY;
            if (!sendMessage())
            {
               return(false);
            }
         }
         break;

      case PLAYER_EXIT:
         // Player exiting.
         i = message.common.exitMsg.playerIndex;
         if (currentPlayers[i])
         {
            currentPlayers[i] = false;
            playerTimeouts[i] = 0;
            if (needInfo[i])
            {
               needInfo[i] = false;
               if (count > 0)
               {
                  count--;
               }
            }
         }
         masterSynch = true;                      // Re-synchronize.
         break;

      case TIME_OUT:
         // Assume all non-responding players are gone.
         for (i = 0; i < MAX_PLAYERS; i++)
         {
            if (needInfo[i])
            {
               playerTimeouts[i]++;
               if (playerTimeouts[i] >= MAX_MSG_TIME_OUTS)
               {
                  currentPlayers[i] = false;
                  playerTimeouts[i] = 0;
               }
            }
         }
         masterSynch = true;                      // Re-synchronize.
         return(true);
      }
   }
   return(true);
}


// Send state of slave to master.
bool Network::sendSlave()
{
   // Terminated?
   if (terminated)
   {
      return(false);
   }

   status           = OK;
   statusMessage[0] = '\0';

   messageAddr  = masterAddr;
   message.type = SLAVE_INFO;
   message.common.slaveMsg.playerIndex  = myIndex;
   message.common.slaveMsg.synchReq     = slaveSynch;
   message.common.slaveMsg.payload.size = slavePayloads[myIndex].size;
   memcpy(message.common.slaveMsg.payload.data, slavePayloads[myIndex].data, slavePayloads[myIndex].size);
   if (!sendMessage())
   {
      return(false);
   }
   return(true);
}


// Notify players of exiting player.
bool Network::exitNotify(EXIT_STATUS status)
{
   int i, j;

   // Terminated?
   if (terminated)
   {
      return(false);
   }
   terminated = true;

   this->status     = OK;
   statusMessage[0] = '\0';

   message.type = PLAYER_EXIT;
   message.common.exitMsg.status = status;
   currentPlayers[myIndex]       = false;

   if (master)
   {
      for (i = 0; i < MAX_PLAYERS; i++)
      {
         message.common.exitMsg.addresses[i]      = playerAddrs[i];
         message.common.exitMsg.currentPlayers[i] = currentPlayers[i];
      }
      masterSynch = true;

      // Send player states to new master.
      j = -1;
      for (i = 0; i < MAX_PLAYERS; i++)
      {
         if (currentPlayers[i] && (j == -1))
         {
            j = i;
         }
         else
         {
            currentPlayers[i] = false;
         }
      }
      if (j != -1)
      {
         messageAddr = playerAddrs[j];
         if (!sendMessage())
         {
            return(false);
         }
      }
   }
   else
   {
      message.common.exitMsg.playerIndex = myIndex;
      messageAddr = masterAddr;
      if (!sendMessage())
      {
         return(false);
      }
   }

   // Wait for message to be sent to prevent receive error.
#ifdef UNIX
   sleep(EXIT_DELAY);
#else
   Sleep(EXIT_DELAY);
#endif

   return(true);
}


// Set up my address.
bool Network::setupMyAddress()
{
   unsigned long a[1];

   // Create UDP socket
   mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#ifdef UNIX
   if (mySocket == -1)
   {
      sprintf(statusMessage, "Socket call failed with: %d", errno);
      status = FATAL;
      return(false);
   }
#else
   if (mySocket == SOCKET_ERROR)
   {
      sprintf(statusMessage, "Socket call failed with: %d", WSAGetLastError());
      status = FATAL;
      return(false);
   }
#endif

   // Fill in the interface information.
   myAddr.sin_family = AF_INET;
#ifdef LOOP_AROUND
   if (master)
   {
      myAddr.sin_port = htons(NETWORK_PORT);
   }
   else
   {
      myAddr.sin_port = htons(NETWORK_PORT + 1);
   }
#else
   myAddr.sin_port = htons(NETWORK_PORT);
#endif
   myAddr.sin_addr.s_addr = INADDR_ANY;

   // Bind socket to port.
#ifdef UNIX
   if (bind(mySocket, (struct sockaddr *)&myAddr, sizeof(myAddr)) == -1)
   {
      sprintf(statusMessage, "Bind call failed with: %d", errno);
      status = FATAL;
      return(false);
   }
#else
   if (bind(mySocket, (struct sockaddr *)&myAddr, sizeof(myAddr)) == SOCKET_ERROR)
   {
      sprintf(statusMessage, "Bind call failed with: %d", WSAGetLastError());
      status = FATAL;
      return(false);
   }
#endif

   // Make socket non-blocking.
   a[0] = 1;
#ifdef UNIX
   if (ioctl(mySocket, FIONBIO, a) == -1)
   {
      sprintf(statusMessage, "Cannot make socket non-blocking, error: %d", errno);
      status = FATAL;
      return(false);
   }
#else
   if (ioctlsocket(mySocket, FIONBIO, a) == SOCKET_ERROR)
   {
      sprintf(statusMessage, "Cannot make socket non-blocking");
      status = FATAL;
      return(false);
   }
#endif
   return(true);
}


// Set up master address.
bool Network::setupMasterAddress()
{
   struct hostent *host;

   // Fill in the host information
   masterAddr.sin_family      = AF_INET;
   masterAddr.sin_port        = htons(NETWORK_PORT);
   masterAddr.sin_addr.s_addr = inet_addr(masterHost);

   // Resolve non-numeric address?
#ifdef UNIX
   if (masterAddr.sin_addr.s_addr == -1)
#else
   if (masterAddr.sin_addr.s_addr == INADDR_NONE)
#endif
   {
      host = NULL;
      host = gethostbyname(masterHost);
      if (host == NULL)
      {
         sprintf(statusMessage, "Unknown host: %s", masterHost);
         status = FATAL;
         return(false);
      }
      memcpy(&masterAddr.sin_addr, host->h_addr_list[0], host->h_length);
   }
   return(true);
}


// Send message from message buffer.
bool Network::sendMessage()
{
   int ret, len;

   // Compute message length.
   len = sizeof(MESSAGE_TYPE);
   switch (message.type)
   {
   case INIT:
      len += sizeof(struct INIT_MSG);
      break;

   case INIT_ACK:
      len += sizeof(struct INIT_ACK_MSG);
      break;

   case PLAYER_EXIT:
      len += sizeof(struct PLAYER_EXIT_MSG);
      break;

   case MASTER_INFO:
      len += sizeof(struct MASTER_INFO_MSG) -
             (MAX_MASTER_PAYLOAD - message.common.masterMsg.payload.size);
      break;

   case SLAVE_INFO:
      len += sizeof(struct SLAVE_INFO_MSG) -
             (MAX_SLAVE_PAYLOAD - message.common.slaveMsg.payload.size);
      break;
   }

   // Send message.
   ret = sendto(mySocket, (char *)&message, len, 0,
                (struct sockaddr *)&messageAddr, sizeof(messageAddr));
#ifdef UNIX
   if (ret == -1)
   {
      sprintf(statusMessage, "Sendto call failed with: %d", errno);
      status = FATAL;
      return(false);
   }
#else
   if (ret == SOCKET_ERROR)
   {
      sprintf(statusMessage, "Sendto call failed with: %d", WSAGetLastError());
      status = FATAL;
      return(false);
   }
#endif
   return(true);
}


// Get a message into message buffer.
bool Network::getMessage(bool wait)
{
   int ret;

#ifdef UNIX
   socklen_t addrLen;
#else
   int addrLen;
#endif

   for (int timer = 0; timer < MSG_WAIT; timer += MSG_RETRY)
   {
      addrLen = sizeof(messageAddr);
      ret     = recvfrom(mySocket, (char *)&message, sizeof(struct MESSAGE), 0,
                         (struct sockaddr *)&messageAddr, &addrLen);

#ifdef UNIX
      if (ret == -1)
      {
         if (errno == EWOULDBLOCK)
         {
            if (!wait)
            {
               break;
            }
            sleep(MSG_RETRY);
            continue;
         }
         sprintf(statusMessage, "Recvfrom call failed with: %d", errno);
         status = FATAL;
         return(false);
      }
#else
      if (ret == SOCKET_ERROR)
      {
         if (WSAGetLastError() == WSAEWOULDBLOCK)
         {
            if (!wait)
            {
               break;
            }
            Sleep(MSG_RETRY);
            continue;
         }
         sprintf(statusMessage, "Recvfrom call failed with: %d", WSAGetLastError());
         status = FATAL;
         return(false);
      }
#endif

      // Got message.
      return(true);
   }

   // Time-out.
   message.type = TIME_OUT;
   return(true);
}


// Is this my (local) address?
bool Network::isMyAddr(SOCKADDR_IN testAddr)
{
   char ac[80];

#ifdef UNIX
   if (gethostname(ac, sizeof(ac)) == -1)
   {
      sprintf(statusMessage, "Error %d when getting local host name", errno);
      status = FATAL;
      return(false);
   }
#else
   if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
   {
      sprintf(statusMessage, "Error %d when getting local host name", WSAGetLastError());
      status = FATAL;
      return(false);
   }
#endif

   struct hostent *phe = gethostbyname(ac);
   if (phe == 0)
   {
      sprintf(statusMessage, "Bad host lookup");
      status = FATAL;
      return(false);
   }

   // Check all my addresses.
   for (int i = 0; phe->h_addr_list[i] != 0; ++i)
   {
      struct in_addr iaddr;
      SOCKADDR_IN    saddr;
      memcpy(&iaddr, phe->h_addr_list[i], sizeof(struct in_addr));
      saddr.sin_addr.s_addr = inet_addr(inet_ntoa(iaddr));
      if (testAddr.sin_addr.s_addr == saddr.sin_addr.s_addr)
      {
         return(true);
      }
   }
   return(false);
}


#endif
