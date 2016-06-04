//
//  q_list_server_listener.h
//  q_list_server_listener
//
//  Created by Gabriele on 02/06/16.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include <atomic>
#include <wave_riff.hpp>
#include <rak_server.hpp>
#include <opus/opus.h>
#include <map>
#include <QListWidget>
#include <QListWidgetItem>
#include <q_audio_server_listener.h>

class q_list_server_listener : public rak_server_listener
{
public:

    struct row_map_listener
    {
        q_audio_server_listener  m_listener;
        QListWidgetItem*         m_item;
    };

    using map_listeners  = std::map< std::string , row_map_listener > ;
    using iterator       = typename map_listeners::iterator ;
    using const_iterator = typename map_listeners::const_iterator ;

    q_list_server_listener(QListWidget* q_list_widge = nullptr)
    {
        m_list_widget = q_list_widge;
    }

    void set_list(QListWidget* q_list_widge)
    {
        m_list_widget = q_list_widge;
    }

    QListWidgetItem* create_new_row(row_map_listener& row)
    {
        //item
        QListWidgetItem* item = new QListWidgetItem();
        //add ptr to row
        item->setData(Qt::UserRole,QVariant::fromValue<void*>(&row));
        //add item
        m_list_widget->addItem(item);
        //return item
        return item;
    }

    virtual void incoming_connection(rak_server& server,const RakNet::AddressOrGUID addrs)
    {
        //get row;
        auto& row = m_listeners[addrs.ToString()];
        //add into map
        row.m_listener.incoming_connection(server,addrs);
        //listener
        if(!row.m_item) row.m_item = create_new_row(row);
        //set info
        row.m_item->setText(build_item_string(QString::fromUtf8(row.m_listener.get_android_id().c_str()),
                                              QString::fromUtf8(row.m_listener.get_imei().c_str()),
                                              true));
    }

    virtual void end_connection(rak_server& server,const RakNet::AddressOrGUID addrs)
    {
        //get row;
        auto& row = m_listeners[addrs.ToString()];
        //call listner
        row.m_listener.end_connection(server,addrs);
        //set info
        row.m_item->setText(build_item_string(QString::fromUtf8(row.m_listener.get_android_id().c_str()),
                                              QString::fromUtf8(row.m_listener.get_imei().c_str()),
                                              true));
    }

    virtual void get_imei_and_android_id(rak_server& server,
                                         const RakNet::AddressOrGUID addrs,
                                         const char* imei,
                                         const char* android_id)
    {
        //get row;
        auto& row = m_listeners[addrs.ToString()];
        //call listner
        row.m_listener.get_imei_and_android_id(server, addrs, imei,android_id);
        //set info
        row.m_item->setText(build_item_string(android_id,imei,true));
    }

    virtual void get_raw_voice(rak_server& server ,const RakNet::AddressOrGUID addrs,RakNet::BitStream& stream)
    {
        //get row;
        auto& row = m_listeners[addrs.ToString()];
        //call listner
        row.m_listener.get_raw_voice(server,addrs,stream);
    }

    virtual void fail_connection(rak_server& server, const RakNet::AddressOrGUID addrs)
    {
        //get row;
        auto& row = m_listeners[addrs.ToString()];
        //call listner
        row.m_listener.fail_connection(server,addrs);
    }

    virtual void update(rak_server& server)
    {
        for(auto& it:m_listeners)
        {
            it.second.m_listener.update(server);
        }
    }

    row_map_listener& first()
    {
        return (m_listeners.begin()->second);
    }

    row_map_listener& last()
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

    row_map_listener& operator[](const RakNet::AddressOrGUID addrs)
    {
        return m_listeners[addrs.ToString()];
    }

    row_map_listener& operator[](const std::string& addrs)
    {
        return m_listeners[addrs];
    }


private:

    QString build_item_string(const QString& android_id,const QString& imei, bool connected = true)
    {
        return "Android id: "+android_id+
                (imei.size() ?" | IMEI: "+imei:"")+
                " | status: "+
                (connected? "connected":"disconnected");
    }

    map_listeners m_listeners;
    QListWidget*  m_list_widget;

};
