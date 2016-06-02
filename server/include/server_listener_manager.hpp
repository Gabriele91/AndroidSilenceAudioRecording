//
//  server_listener_manager.hpp
//  rak_server
//
//  Created by Gabriele on 03/06/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include <atomic>
#include <wave_riff.hpp>
#include <rak_server.hpp>
#include <opus/opus.h>
#include <map>

template < class T >
class server_listener_manager : public rak_server_listener
{
public:
    
    using map_listeners  = std::map< std::string , T > ;
    using iterator       = typename map_listeners::iterator ;
    using const_iterator = typename map_listeners::const_iterator ;
    
    server_listener_manager()
    {
        
    }
    
    virtual void incoming_connection(rak_server& server,const RakNet::AddressOrGUID addrs)
    {
        m_listeners[addrs.ToString()].incoming_connection(server,addrs);
    }
    
    virtual void end_connection(rak_server& server,const RakNet::AddressOrGUID addrs)
    {
        m_listeners[addrs.ToString()].end_connection(server,addrs);
    }
    
    virtual void get_imei_and_android_id(rak_server& server,
                                         const RakNet::AddressOrGUID addrs,
                                         const char* imei,
                                         const char* android_id)
    {
        m_listeners[addrs.ToString()].get_imei_and_android_id(server, addrs, imei,android_id);
    }

    virtual void get_raw_voice(rak_server& server ,const RakNet::AddressOrGUID addrs,RakNet::BitStream& stream)
    {
        m_listeners[addrs.ToString()].get_raw_voice(server,addrs,stream);
    }
    
    virtual void fail_connection(rak_server& server, const RakNet::AddressOrGUID addrs)
    {
        m_listeners[addrs.ToString()].fail_connection(server,addrs);
    }
    
    virtual void update(rak_server& server)
    {
        for(auto& it:m_listeners)
        {
            it.second.update(server);
        }
    }
    
    T& first()
    {
        return (m_listeners.begin()->second);
    }
    
    T& last()
    {
        return ((--m_listeners.end())->second);
    }
    
    iterator begin()
    {
        return m_listeners.begin();
    }
    
    const_iterator begin() const
    {
        return m_listeners.begin();
    }
    
    iterator end()
    {
        return m_listeners.end();
    }
    
    const_iterator end() const
    {
        return m_listeners.end();
    }
    
    size_t size() const
    {
        return m_listeners.size();
    }
    
    T& operator[](const RakNet::AddressOrGUID addrs)
    {
        return m_listeners[addrs.ToString()];
    }
    
    T& operator[](const std::string& addrs)
    {
        return m_listeners[addrs];
    }
    
private:
    
    map_listeners m_listeners;
    
    
};