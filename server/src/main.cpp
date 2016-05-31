#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <list>
#include <iostream>
#include <string.h>
#include <wave_riff.hpp>
#include <RakNet/RakPeer.h>
#include <RakNet/RakPeerInterface.h>
#include <RakNet/MessageIdentifiers.h>
#include <RakNet/BitStream.h>
#include <RakNet/RakNetVersion.h>  // MessageID
#include <curses.h>

#define SHELL_CLEAR system("clear"); ////system("cls");
#define MAX_CLIENTS 16
#define SERVER_PORT 8000
class rak_server_listener;
class rak_server;

enum rak_id_msg
{
    ID_MSG_CONFIG = ID_USER_PACKET_ENUM + 1,
    ID_MSG_START_REC,
    ID_MSG_PAUSE_REC,
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
        //stop
        stop_loop();
        //shutdown
        m_peer->Shutdown(0);
        //destoy
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



enum listener_state
{
    S_NONE,
    S_DISC,
    S_CONN,
    S_REC,
    S_PAUSE,
    S_STOP
};
using atomic_listener_state = std::atomic < listener_state >;

class test_listener : public rak_server_listener
{
public:
    
    test_listener()
    {
        //metainfo
        m_info = input_meta_info
        {
            1,
            8000,
            16
        };
    }
    
	virtual void incoming_connection(rak_server& server, const RakNet::AddressOrGUID addr)
	{
        //save addr
        m_addr = addr;
        //send tipe
		server.send_config_msg(addr, m_info.m_channels, m_info.m_samples_per_sec, m_info.m_bits_per_sample);
        //connected
        m_state = S_CONN;
    }
    
	virtual void end_connection(rak_server&, const RakNet::AddressOrGUID)
	{
        //connected
        m_state = S_DISC;
        //close file
        close_wav_file();
	}
    
    virtual void get_raw_voice(rak_server& server,const RakNet::AddressOrGUID addrs,std::vector < unsigned char >& buffer)
    {
        //append file
        m_wav.append((void*)buffer.data(), buffer.size(), wav_riff::LE_MODE);
    }
    
	virtual void update(rak_server& server)
    {
	}
    
    virtual void fail_connection()
    {
        //connected
        m_state = S_DISC;
        //close file
        close_wav_file();
    }
    
    atomic_listener_state& state()
    {
        return m_state;
    }
    
    RakNet::AddressOrGUID& address()
    {
        return m_addr;
    }
    
    void create_file(const std::string& path)
    {
        close_wav_file();
        //open file
        m_file = fopen(path.c_str(),"w");
        //init file
        m_wav.init(m_file, m_info, wav_riff::LE_MODE);
        
    }
    
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
    
protected:
    
    //data info
    input_meta_info       m_info;
    FILE*                 m_file { nullptr };
    wav_riff              m_wav;
    RakNet::AddressOrGUID m_addr;
    atomic_listener_state m_state { S_DISC };
    
 
};

int main(void)
{
    //ini server
	rak_server server;
	server.init();
    //init listener
	test_listener listener;
	server.loop(listener);
    //local state
    listener_state l_state { S_NONE };
    //manager
    while(true)
    {
        l_state = listener.state();
        //select
        switch (l_state)
        {
            case S_DISC:
                    SHELL_CLEAR
                    std::cout << "not connected\n";
            break;
                
            case S_CONN:
            {
                SHELL_CLEAR
                std::cout << "is connected: start to rec or exit?\n";
                //get input
                std::string command;
                std::cin >> command;
                //parse
                if(command == "start")
                {
                    server.mutex().lock();
                    //alloc file
                    listener.create_file("test.wav");
                    //start
                    server.send_start_msg(listener.address());
                    listener.state() = S_REC;
                    //stop
                    server.mutex().unlock();
                }
                if(command == "exit")
                {
                    return 0;
                }
            }
            break;
                
            case S_REC:
            {
                SHELL_CLEAR
                std::cout << "recodring: stop?\n";
                //get input
                std::string command;
                std::cin >> command;
                //parse
                if(command == "stop")
                {
                    server.mutex().lock();
                    server.send_stop_msg(listener.address());
                    listener.state() = S_STOP;
                    server.mutex().unlock();
                }
                //parse
                if(command == "pause")
                {
                    server.mutex().lock();
                    server.send_pause_msg(listener.address());
                    listener.state() = S_PAUSE;
                    server.mutex().unlock();
                }
            }
                break;
                
            case S_PAUSE:
            {
                SHELL_CLEAR
                std::cout << "recodring in pause, continue or stop?\n";
                //get input
                std::string command;
                std::cin >> command;
                //parse
                if(command == "continue")
                {
                    server.mutex().lock();
                    server.send_start_msg(listener.address());
                    listener.state() = S_REC;
                    server.mutex().unlock();
                }
                //parse
                if(command == "stop")
                {
                    server.mutex().lock();
                    server.send_stop_msg(listener.address());
                    listener.state() = S_STOP;
                    server.mutex().unlock();
                }
            }
            break;
                
            case S_STOP:
            {
                SHELL_CLEAR
                std::cout << "recodring stopped, save?\n";
                //get input
                std::string command;
                std::cin >> command;
                //parse
                if(command == "save")
                {
                    //close file
                    listener.close_wav_file();
                    //end write
                    listener.state() = S_CONN;
                }
            }
            break;
                
            default:  break;
        }
    }

	return 0;
}
