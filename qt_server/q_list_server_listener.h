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

    using map_android_id = std::map< std::string , std::string > ;
    using map_listeners  = std::map< std::string , row_map_listener > ;
    using iterator       = map_listeners::iterator ;
    using const_iterator = map_listeners::const_iterator ;

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
        //none
    }

    virtual void get_imei_and_android_id(rak_server& server,
                                         const RakNet::AddressOrGUID addrs,
                                         const char* c_imei,
                                         const char* c_android_id)
    {
        std::string android_id(c_android_id);
        //save android id
        add_android_id_into_map(addrs,android_id);
        //get row
        row_map_listener& row = m_listeners[android_id];
        //listener
        if(!row.m_item) row.m_item = create_new_row(row);
        //call listner
        row.m_listener.incoming_connection(server, addrs);
        //call listner
        row.m_listener.get_imei_and_android_id(server, addrs, c_imei, c_android_id);
        //set info
        row.m_item->setText(build_item_string(c_android_id, c_imei,true));
    }

    virtual void end_connection(rak_server& server,const RakNet::AddressOrGUID addrs)
    {
        //listener
        auto it_listeners = get_row_from_addrs(addrs);
        //get row;
        if(it_listeners != m_listeners.end())
        {
            //get row
            auto& row = it_listeners->second;
            //set info
            row.m_item->setText(build_item_string(QString::fromUtf8(row.m_listener.get_android_id().c_str()),
                                                  QString::fromUtf8(row.m_listener.get_imei().c_str()),
                                                  false));
            //call listner
            row.m_listener.end_connection(server,addrs);
        }
    }

    virtual void get_raw_voice(rak_server& server ,
                               const RakNet::AddressOrGUID addrs,
                               RakNet::BitStream& stream)
    {
        //listener
        auto it_listeners = get_row_from_addrs(addrs);
        qDebug() << "voice from:"<< addrs.ToString();
        //get row;
        if(it_listeners != m_listeners.end())
        {
            //get row
            auto& row = it_listeners->second;
            //call listner
            row.m_listener.get_raw_voice(server,addrs,stream);
        }
    }

    virtual void fail_connection(rak_server& server, const RakNet::AddressOrGUID addrs)
    {
        //listener
        auto it_listeners = get_row_from_addrs(addrs);
        //get row;
        if(it_listeners != m_listeners.end())
        {
            //get row
            auto& row = it_listeners->second;
            //set info
            row.m_item->setText(build_item_string(QString::fromUtf8(row.m_listener.get_android_id().c_str()),
                                                  QString::fromUtf8(row.m_listener.get_imei().c_str()),
                                                  false));
            //call listner
            row.m_listener.fail_connection(server,addrs);
        }
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

    void clear()
    {
        for(auto& element:m_listeners)
        {
            delete element.second.m_item ;
        }
        m_listeners.clear();
    }

    std::string get_android_id(const std::string& addrs)
    {
        if(exists_android_id(addrs)) return m_android_ids[addrs];
        else std::string();
    }

    void add_android_id_into_map(const std::string& addrs,const std::string& android_id)
    {
        m_android_ids.insert({addrs,android_id});
    }

    void add_android_id_into_map(const RakNet::AddressOrGUID& addrs,const std::string& android_id)
    {
        m_android_ids.insert({addrs.ToString(),android_id});
    }

    iterator get_row_from_addrs(const std::string& addrs)
    {
        //search id
        auto it_id = m_android_ids.find(addrs);
        //test id
        if(it_id != m_android_ids.end())
        {
            return m_listeners.find(it_id->second);
        }
        //else return false
        return m_listeners.end();
    }

    iterator get_row_from_addrs(const RakNet::AddressOrGUID& addrs)
    {
        //search id
        auto it_id = m_android_ids.find(addrs.ToString());
        //test id
        if(it_id != m_android_ids.end())
        {
            return m_listeners.find(it_id->second);
        }
        //else return false
        return m_listeners.end();
    }

    bool exists_android_id(const std::string& addrs) const
    {
       return m_android_ids.end() != m_android_ids.find(addrs);
    }

    bool exists_android_id(const RakNet::AddressOrGUID& addrs) const
    {
       return m_android_ids.end() != m_android_ids.find(addrs.ToString());
    }


private:

    QString build_item_string(const QString& android_id,const QString& imei, bool connected = true)
    {
        return "Android id: "+android_id+
                (imei.size() ?" | IMEI: "+imei:"")+
                " | status: "+
                (connected? "connected":"disconnected");
    }

    map_android_id m_android_ids;
    map_listeners  m_listeners;
    QListWidget*   m_list_widget;

};
