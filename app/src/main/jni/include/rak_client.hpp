//
// Created by Gabriele on 29/05/16.
//
#pragma once

#include <RakNet/RakNetTypes.h>
#include <RakNet/RakNetDefines.h>
#include <RakNet/RakPeerInterface.h>
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/BitStream.h>
#include <thread>
#include <android/log.h>

#define DEFAULT_PORT 62348
enum  rak_id_msg;
class rak_client;
class rak_client_callback;

enum rak_id_msg
{
    ID_MSG_CONFIG = ID_USER_PACKET_ENUM+1,
    ID_MSG_START_REC,
    ID_MSG_END_REC,
    ID_MSG_RAW_VOICE
};

class rak_client_callback
{
public:

    virtual void msg_config(unsigned int channels,
                            unsigned int samples,
                            unsigned int bits ) = 0;
    virtual void msg_start_rec( ) = 0;
    virtual void msg_end_rec( ) = 0;
    virtual void new_connection(RakNet::AddressOrGUID) = 0;
    virtual void end_connection(RakNet::AddressOrGUID) = 0;
    virtual void rak_update(rak_client& client) = 0;
};

class rak_client
{
public:

    rak_client()
    {
        //get instance
        m_peer = RakNet::RakPeerInterface::GetInstance();
        //get
        m_loop = false;
    }

    RakNet::RakPeerInterface* get_interface()
    {
        return m_peer;
    }

    ~rak_client()
    {
        if(m_peer) destroy();
    }

    bool init(rak_client_callback* callback, const char* host, int port = DEFAULT_PORT)
    {
        //set callback
        m_callback = callback;
        //socket_descriptor
        RakNet::SocketDescriptor socket_descriptor;
        //start up
        m_peer->Startup(1,&socket_descriptor, 1);
        //connection
        return m_peer->Connect(host, port, 0,0) == RakNet::CONNECTION_ATTEMPT_STARTED;
    }

    void destroy()
    {
        RakNet::RakPeerInterface::DestroyInstance(m_peer);
    }

    bool is_loop() const
    {
        return m_loop;
    }

    void stop_loop()
    {
        m_loop = false;
        m_thread.join();
    }

    bool loop()
    {

        if(is_loop()) return false;
        //enable loop
        m_loop = true;
        //separate thread
        m_thread = std::thread((std::thread &&) [this] ()
                {
                    //infinite loop
                    while (m_loop)
                    {
                        for (RakNet::Packet*
                             packet=m_peer->Receive();
                             //test exit
                             packet;
                             //next
                             m_peer->DeallocatePacket(packet),
                             packet=m_peer->Receive())

                        {
                            switch (packet->data[0])
                            {
                                case ID_REMOTE_DISCONNECTION_NOTIFICATION: break;
                                case ID_REMOTE_CONNECTION_LOST: break;
                                case ID_REMOTE_NEW_INCOMING_CONNECTION: break;
                                case ID_NEW_INCOMING_CONNECTION:  break;
                                case ID_NO_FREE_INCOMING_CONNECTIONS:  break;

                                case ID_CONNECTION_REQUEST_ACCEPTED:
                                    m_callback->new_connection(packet->systemAddress);
                                    break;

                                case ID_DISCONNECTION_NOTIFICATION:
                                case ID_CONNECTION_LOST:
                                    m_callback->end_connection(packet->systemAddress);
                                    break;

                                case ID_MSG_CONFIG:
                                {
                                    RakNet::BitStream buffer_stream_in(packet->data,packet->length,false);
                                    buffer_stream_in.IgnoreBytes(sizeof(RakNet::MessageID));
                                    //values
                                    unsigned int channels = 0;
                                    unsigned int samples  = 0;
                                    unsigned int bits     = 0;
                                    //read
                                    buffer_stream_in.Read(channels);
                                    buffer_stream_in.Read(samples);
                                    buffer_stream_in.Read(bits);
                                    //callback
                                    m_callback->msg_config(channels,samples,bits);
                                }
                                break;

                                case ID_MSG_END_REC:
                                case ID_MSG_START_REC:
                                {
                                    RakNet::BitStream buffer_stream_in(packet->data,packet->length,false);
                                    buffer_stream_in.IgnoreBytes(sizeof(RakNet::MessageID));
                                    //cases
                                    if(packet->data[0]==ID_MSG_START_REC)
                                        m_callback->msg_start_rec();
                                    else
                                        m_callback->msg_end_rec();
                                }
                                break;
                                default:
                                    __android_log_print(ANDROID_LOG_ERROR,
                                                        "rak_client",
                                                        "Message with identifier %i has arrived.\n",
                                                        packet->data[0]);
                                    break;
                            }
                        }
                        //rak thread update
                        m_callback->rak_update(*this);
                    }
                });
        return true;
    }

private:

    std::thread               m_thread;
    rak_client_callback*      m_callback;
    bool                      m_loop{ false   };
    RakNet::RakPeerInterface* m_peer{ nullptr };

};