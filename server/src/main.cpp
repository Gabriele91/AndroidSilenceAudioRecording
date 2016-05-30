#include <stdio.h>
#include <functional>
#include <vector>
#include <string.h>
#include <wave_riff.hpp>
#include <RakNet/RakPeer.h>
#include <RakNet/RakPeerInterface.h>
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/BitStream.h>
#include <RakNet/RakNetVersion.h>  // MessageID

#define MAX_CLIENTS 16
#define SERVER_PORT 8000
class rak_server_listener;
class rak_server;

enum rak_id_msg
{
	ID_MSG_CONFIG = ID_USER_PACKET_ENUM + 1,
	ID_MSG_START_REC,
	ID_MSG_END_REC,
    ID_MSG_RAW_VOICE
};

class rak_server_listener
{
public:
	virtual void incoming_connection(rak_server&,const RakNet::AddressOrGUID addrs) = 0;
    virtual void end_connection(rak_server&,const RakNet::AddressOrGUID addrs) = 0;
    virtual void get_raw_voice(rak_server&,const RakNet::AddressOrGUID addrs,std::vector < unsigned char >& buffer) = 0;
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
        RakNet::RakPeerInterface::DestroyInstance(m_peer);
    }

    bool init() 
    {
        RakNet::SocketDescriptor socket_desc(SERVER_PORT, 0);
        RakNet::StartupResult result = m_peer->Startup(MAX_CLIENTS, &socket_desc, 1);
        m_peer->SetMaximumIncomingConnections(MAX_CLIENTS);
        return result == RakNet::RAKNET_STARTED;
    }
    
    void stop_loop()
    {
        m_loop = false;
    }

    void send_stop_msg(const RakNet::AddressOrGUID addr)
    {
        RakNet::BitStream stream_output;
        stream_output.Write((RakNet::MessageID)ID_MSG_END_REC);
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
        RakNet::Packet *packet = nullptr;
        m_loop = true;

        while (m_loop)
        {

            for (packet = m_peer->Receive(); packet; m_peer->DeallocatePacket(packet), packet = m_peer->Receive())
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
                        stream.IgnoreBits(sizeof(RakNet::MessageID));
                        //buffer size
                        unsigned int size = stream.GetNumberOfUnreadBits() / 8;
                        //alloc buffer
                        std::vector < unsigned char > buffer(size);
                        //read buffer
                        stream.ReadBits(buffer.data(), size*8);
                        //value
                        listener.get_raw_voice(*this, packet->systemAddress, buffer);
                    }
                    break;

                    default:
                        printf("Message: %i\n", packet->data[0]);
                    break;
                }
            }
        }
        listener.update(*this);
    }

private:

    RakNet::RakPeerInterface *m_peer;
    bool m_loop;
};

class test_listener : public rak_server_listener
{
public:
    
    
	virtual void incoming_connection(rak_server& server, const RakNet::AddressOrGUID addr)
	{
        //metainfo
        input_meta_info info =
        {
            1,
            8000,
            16
        };
        //send tipe
		server.send_config_msg(addr, info.m_channels, info.m_samples_per_sec, info.m_bits_per_sample);
        //open file
        m_file = fopen("test.wav","w");
        //init file
        m_wav.init(m_file, info, wav_riff::LE_MODE);
        //send start rec
        server.send_start_msg(addr);
    }
    
	virtual void end_connection(rak_server&, const RakNet::AddressOrGUID)
	{
        close_wav_file();
	}
    
    virtual void get_raw_voice(rak_server&,const RakNet::AddressOrGUID addrs,std::vector < unsigned char >& buffer)
    {
        //append file
        m_wav.append((void*)buffer.data(), buffer.size(), wav_riff::LE_MODE);
    }
    
	virtual void update(rak_server&)
	{
	}
    
    virtual void fail_connection()
    {
        close_wav_file();
    }
    
public:
    
    FILE*    m_file { nullptr };
    wav_riff m_wav;
    
    void close_wav_file()
    {
        //if open
        if(m_file)
        {
            //compute size
            m_wav.complete();
            //close
            fclose(m_file);
            m_file = nullptr;
        }
    }
    
};

int main(void)
{
	rak_server server;
	server.init();
	test_listener tl;
	server.loop(tl);

	return 0;
}
