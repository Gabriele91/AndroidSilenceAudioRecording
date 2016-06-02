//
//  server_websocket.hpp
//  rak_server
//
//  Created by Gabriele on 02/06/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include <server_listener.hpp>
#define ASIO_STANDALONE
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS
#define _WEBSOCKETPP_CPP11_FUNCTIONAL_
#define _WEBSOCKETPP_CPP11_FUNCTIONAL_
#define _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_MEMORY_
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class server_websocket
{
public:
    
    //type of server
    using asio_server  = websocketpp::server<websocketpp::config::asio> ;
    using message_ptr  = asio_server::message_ptr ;
    
    static  void on_message(asio_server* s, websocketpp::connection_hdl hdl, message_ptr msg)
    {
        
    }
    
    class handler : public asio_server::message_handler
    {
        void on_message(asio_server::connection_ptr con, std::string msg)
        {
            
        }
    };
    
    server_websocket(rak_server& server,server_listener& listener)
    :m_server(server)
    ,m_listener(listener)
    {
        // Set logging settings
        m_asio_server.set_access_channels(websocketpp::log::alevel::all);
        m_asio_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
        // Initialize Asio
        m_asio_server.init_asio();
        // Register our message handler
        m_asio_server.set_message_handler( handler());
    }
    
    void loop()
    {
        // Listen on port 9002
        m_asio_server.listen(9002);
        // Start the server accept loop
        m_asio_server.start_accept();
        // Start the ASIO io_service run loop
        m_asio_server.run();
    }
    
    void open_file(const std::string& file)
    {
        m_server.mutex().lock();
        //alloc file
        m_listener.create_file(file);
        //stop
        m_server.mutex().unlock();
    }
    
    void close_file()
    {
        m_server.mutex().lock();
        //close file
        m_listener.close_wav_file();
        //stop
        m_server.mutex().unlock();
    }
    
    void star_rec(const std::string& file)
    {
        m_server.mutex().lock();
        m_server.send_stop_msg(m_listener.address());
        m_listener.state() = S_REC;
        m_server.mutex().unlock();
    }
    
    void stop_rec(const std::string& file)
    {
        m_server.mutex().lock();
        m_server.send_stop_msg(m_listener.address());
        m_listener.state() = S_STOP;
        m_server.mutex().unlock();
    }
    
    ~server_websocket()
    {
        
    }
    
private:

    rak_server&      m_server;
    server_listener& m_listener;
    asio_server      m_asio_server;
    
};

