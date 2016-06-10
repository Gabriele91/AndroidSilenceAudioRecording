#include <stdio.h>
#include <iostream>
#include <string.h>
#include <rak_server.hpp>
//#define USE_RAW_AUDIO
#include <server_listener.hpp>
#include <server_listener_manager.hpp>
#include <server_websocket.hpp>

#ifdef _WIN32
    #define SHELL_CLEAR system("cls");
#else
    #define SHELL_CLEAR system("clear");
#endif

#define MAX_CLIENTS 16
#define SERVER_PORT 8000

#if 0
int main()
{
    ////////
    //ini server
    rak_server server;
    server.init(SERVER_PORT,MAX_CLIENTS);
    //init listener
    server_listener_manager<server_listener> manager_listeners;
    server.loop(manager_listeners);
    //ini web soket
    server_websocket interface(server,manager_listeners);
    interface.loop();
    ////////
    return 0;
}
#else
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
                //send meta info
                listener.send_meta_info(server);
                //
                SHELL_CLEAR
                std::cout << "is connected: start to rec or exit?\n";
                //get input
                std::string command;
                std::cin >> command;
                //parse
                if(command == "start")
                {
					SHELL_CLEAR
					std::cout << "file name?\n";
					std::string filename;
					std::cin >> filename;
                    //alloc file
                    listener.create_file(filename+".wav");
                    //start
                    listener.send_start(server);
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
                std::cout << "recodring: stop or pause?\n";
                //get input
                std::string command;
                std::cin >> command;
                //parse
                if(command == "stop") listener.send_stop(server);
                if(command == "pause") listener.send_pause(server);
    
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
                if(command == "continue") listener.send_start(server);
                if(command == "stop") listener.send_stop(server);
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
                    listener.reset_state();
                }
            }
            break;
                
            default:  break;
        }
    }

	return 0;
}
#endif
