//
//  rak_server.hpp
//  rak_server
//
//  Created by Gabriele on 02/06/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once

#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <RakNet/RakPeer.h>
#include <RakNet/RakPeerInterface.h>
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/BitStream.h>
#include <RakNet/RakNetVersion.h>  

class rak_server_listener;
class rak_server;

enum rak_id_msg
{
    ID_MSG_CONFIG = ID_USER_PACKET_ENUM + 1,
    ID_MSG_START_REC,
    ID_MSG_PAUSE_REC,
    ID_MSG_END_REC,
    ID_MSG_RAW_VOICE,
    ID_MSG_IMEI
};

class rak_server_listener
{
public:
    virtual void incoming_connection(rak_server&,const RakNet::AddressOrGUID addrs) = 0;
    virtual void end_connection(rak_server&,const RakNet::AddressOrGUID addrs) = 0;
    virtual void get_imei_and_android_id(rak_server&,const char* imei,const char* android_id) = 0;
    virtual void get_raw_voice(rak_server&,const RakNet::AddressOrGUID addrs,RakNet::BitStream& stream) = 0;
    virtual void fail_connection() = 0;
    virtual void update(rak_server&) = 0;
};

class rak_server
{
public:
    
    rak_server()
    {
        m_peer = RakNet::RakPeerInterface::GetInstance();
    }
    
    ~rak_server()
    {
        //stop
        stop_loop();
        //shutdown
        m_peer->Shutdown(0);
        //destoy
        RakNet::RakPeerInterface::DestroyInstance(m_peer);
    }
    
    bool init(int port,int max_clients)
    {
        RakNet::SocketDescriptor socket_desc(port, 0);
        RakNet::StartupResult result = m_peer->Startup(max_clients, &socket_desc, 1);
        m_peer->SetMaximumIncomingConnections(max_clients);
        return result == RakNet::RAKNET_STARTED;
    }

    void stop_loop()
    {
        //stop
        m_loop = false;
        //can join
        if(m_thread.joinable())
        {
            m_thread.join();
        }
    }
    
    void send_stop_msg(const RakNet::AddressOrGUID addr)
    {
        RakNet::BitStream stream_output;
        stream_output.Write((RakNet::MessageID)ID_MSG_END_REC);
        m_peer->Send(&stream_output, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
    }
    
    void send_pause_msg(const RakNet::AddressOrGUID addr)
    {
        RakNet::BitStream stream_output;
        stream_output.Write((RakNet::MessageID)ID_MSG_PAUSE_REC);
        m_peer->Send(&stream_output, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
    }
    
    void send_start_msg(const RakNet::AddressOrGUID addr)
    {
        RakNet::BitStream stream_output;
        stream_output.Write((RakNet::MessageID)ID_MSG_START_REC);
        m_peer->Send(&stream_output, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
    }
    
    void send_config_msg(const RakNet::AddressOrGUID addr,
                         unsigned int channels = 0,
                         unsigned int samples = 0,
                         unsigned int bits = 0)
    {
        RakNet::BitStream stream_output;
        stream_output.Write((RakNet::MessageID)ID_MSG_CONFIG);
        stream_output.Write(channels);
        stream_output.Write(samples);
        stream_output.Write(bits);
        m_peer->Send(&stream_output, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
    }
    void loop(rak_server_listener& listener)
    {
        //stop last loop
        stop_loop();
        //start loop
        m_loop = true;
        //init thread
        m_thread = std::thread([this,&listener]()
                               {
                                   while (m_loop)
                                   {
                                       
                                       for (RakNet::Packet *packet = m_peer->Receive();
                                            packet;
                                            m_peer->DeallocatePacket(packet), packet = m_peer->Receive())
                                       {
                                           switch (packet->data[0])
                                           {
#if 0
                                               case ID_REMOTE_DISCONNECTION_NOTIFICATION: break;
                                               case ID_REMOTE_CONNECTION_LOST: break;
                                               case ID_REMOTE_NEW_INCOMING_CONNECTION: break;
                                               case ID_CONNECTION_REQUEST_ACCEPTED: break;
#endif
                                                   
                                               case ID_NEW_INCOMING_CONNECTION:
                                                   listener.incoming_connection(*this, packet->systemAddress);
                                                   break;
                                                   
                                               case ID_CONNECTION_ATTEMPT_FAILED:
                                                   listener.fail_connection();
                                                   break;
                                                   
                                               case ID_NO_FREE_INCOMING_CONNECTIONS: break;
                                               case ID_DISCONNECTION_NOTIFICATION:
                                                   
                                               case ID_CONNECTION_LOST:
                                                   listener.end_connection(*this,packet->systemAddress);
                                                   break;
                                                   
                                               case ID_MSG_RAW_VOICE:
                                               {
                                                   RakNet::BitStream stream(packet->data,packet->length,false);
                                                   //jmp id
                                                   stream.IgnoreBytes(sizeof(RakNet::MessageID));
                                                   //value
                                                   listener.get_raw_voice(*this, packet->systemAddress, stream);
                                               }
                                               break;
                                               
                                               case ID_MSG_IMEI:
                                               {
                                                   RakNet::BitStream stream(packet->data,packet->length,false);
                                                   //jmp id
                                                   stream.IgnoreBytes(sizeof(RakNet::MessageID));
                                                   //read string
                                                   RakNet::RakString rk_imei;
                                                   stream.Read(rk_imei);
                                                   //read string
                                                   RakNet::RakString rk_android_id;
                                                   stream.Read(rk_android_id);
                                                   //value
                                                   listener.get_imei_and_android_id(*this,
                                                                                    rk_imei.C_String(),
                                                                                    rk_android_id.C_String());
                                               }
                                               break;
                                               
                                               default:
                                                   printf("Message: %i\n", packet->data[0]);
                                               break;
                                           }
                                       }
                                       listener.update(*this);
                                   }
                               });
    }
    
    std::mutex& mutex()
    {
        return m_mutex;
    }
    
private:
    
    RakNet::RakPeerInterface *m_peer;
    bool                      m_loop;
    std::thread               m_thread;
    std::mutex                m_mutex;
};

