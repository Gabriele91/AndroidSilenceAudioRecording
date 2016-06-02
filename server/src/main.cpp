#include <stdio.h>
#include <iostream>
#include <string.h>
#include <rak_server.hpp>
//#define USE_RAW_AUDIO
#include <server_listener.hpp>

#ifdef _WIN32
    #define SHELL_CLEAR system("cls");
#else
    #define SHELL_CLEAR system("clear");
#endif

#define MAX_CLIENTS 16
#define SERVER_PORT 8000




int main(void)
{
    //ini server
	rak_server server;
	server.init(SERVER_PORT,MAX_CLIENTS);
    //init listener
	server_listener listener;
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
