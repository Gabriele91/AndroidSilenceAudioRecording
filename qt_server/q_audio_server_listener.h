//
//  q_audio_server_listener.h
//  q_audio_server_listener
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
#include <QAudio>
#include <QIODevice>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QBuffer>
#include <QApplication>
#include <iostream>
#include <QDebug>

enum listener_state
{
    S_NONE,
    S_DISC,
    S_CONN,
    S_INFO,
    S_REC,
    S_PAUSE,
    S_STOP
};
using atomic_listener_state = std::atomic < listener_state >;

class q_audio_server_listener : public rak_server_listener
{
public:

    q_audio_server_listener()
    {
    }

    ~q_audio_server_listener()
    {
        if(m_decoder) opus_decoder_destroy(m_decoder);
    }

    virtual void init(const input_meta_info& info)
    {
        //default metainfo
        m_info = info;
        //init already called
        if(m_decoder) opus_decoder_destroy(m_decoder);
        //alloc decoder
        m_decoder = opus_decoder_create(m_info.m_samples_per_sec, m_info.m_channels, &m_error);
        //alloc buffer
        m_buf_dec.resize(m_info.m_channels*m_info.m_samples_per_sec*m_info.m_bits_per_sample/8);
    }

    virtual void incoming_connection(rak_server& server, const RakNet::AddressOrGUID addr)
    {
        //save addr
        m_addr = addr;
        //connected
        m_state = S_CONN;
        //is connected
        m_connected = true;
        //callback
        call_connection_cb();
    }

    virtual void end_connection(rak_server&, const RakNet::AddressOrGUID)
    {
        //connected
        m_state = S_DISC;
        //not connected
        m_connected = false;
        //callback
        call_connection_cb();
    }

    virtual void get_raw_voice(rak_server& server,const RakNet::AddressOrGUID addrs,RakNet::BitStream& stream)
    {
        //ptr buffer
        int         buff16_size = (int)m_buf_dec.size() / sizeof(opus_int16);
        opus_int16* buff16_ptr  = (opus_int16*)m_buf_dec.data();
        //get count of blocks
        unsigned int buff16_div;
        stream.Read(buff16_div);
        //for each
        for(int i=0;i!=buff16_div;++i)
        {
            //get size of a block;
            int block_size = 0;
            stream.Read(block_size);
            //alloc buffer
            std::vector < unsigned char > buffer(block_size);
            stream.ReadBits(buffer.data(), block_size*8);
            //get
            int samples_out =
            opus_decode(m_decoder,
                        buffer.data(),
                        block_size,
                        buff16_ptr,
                        buff16_size,
                        0);
            //test
            assert(samples_out >= 0);
            //dec
            buff16_size -= samples_out;
            //next
            buff16_ptr  += samples_out;
        }
        //data size
        size_t data_size = m_buf_dec.size()-(buff16_size * sizeof(opus_int16));
        //audio in debug
        qDebug() << "sound arrived: " << data_size;
        //write file buffer
        append_to_file(m_buf_dec.data(),data_size);
        //sound to output buffer
        applay_to_output_buffer((const char*)m_buf_dec.data(),data_size);
    }

    virtual void update(rak_server& server)
    {
    }

    virtual void get_imei_and_android_id(rak_server& server,
                                         const RakNet::AddressOrGUID,
                                         const char* imei,
                                         const char* android_id)
    {
        m_imei = imei;
        m_android_id = android_id;
    }

    virtual void fail_connection(rak_server&, const RakNet::AddressOrGUID)
    {
        //connected
        m_state = S_DISC;
        //not connected
        m_connected = false;
        //callback
        call_connection_cb();
    }

    atomic_listener_state& state()
    {
        return m_state;
    }

    RakNet::AddressOrGUID& address()
    {
        return m_addr;
    }

    void send_start(rak_server& server)
    {
        server.mutex().lock();
        //send type
        server.send_start_msg(m_addr);
        //start
        m_state = S_REC;
        //end
        server.mutex().unlock();
    }

    void send_pause(rak_server& server)
    {
        server.mutex().lock();
        //send type
        server.send_pause_msg(m_addr);
        //pause
        m_state = S_PAUSE;
        //end
        server.mutex().unlock();
    }

    void send_stop(rak_server& server)
    {
        server.mutex().lock();
        //send type
        server.send_stop_msg(m_addr);
        //stop
        m_state = S_STOP;
        //end
        server.mutex().unlock();
    }

    void send_meta_info(rak_server& server)
    {
        server.mutex().lock();
        //send tipe
        server.send_config_msg(m_addr, m_info.m_channels, m_info.m_samples_per_sec, m_info.m_bits_per_sample);
        //send info
        m_state = S_INFO;
        //end
        server.mutex().unlock();
    }

    void reset_state()
    {
        //no change state
        if(m_state == S_DISC) return;
        //else return to connect
        m_state = S_CONN;

    }

    const std::string& get_imei() const
    {
        return m_imei;
    }

    const std::string& get_android_id() const
    {
        return m_android_id;
    }

    bool connected()
    {
        return m_connected;
    }


    void change_connession_callback(std::function<void(bool)> callback)
    {
        m_connection_cb = callback;
    }

    bool open_output_file(const std::string& path,
                          const wav_riff::info_fields& riff_meta_info)
    {
        //open file
        m_file = fopen(path.c_str(),"w");
        //open
        if(m_file)
        {
            //save path
            m_path = QString(path.c_str());
            //set file
            m_wav.set_file(m_file);
            //init file
            m_wav.init(m_info, wav_riff::LE_MODE, riff_meta_info);
            //ok
            return true;
        }

        return false;
    }

    const QString& get_output_path() const
    {
        return m_path;
    }

    bool close_output_file()
    {
        if(m_file)
        {
            //compute size
            m_wav.complete();
            //close
            fclose(m_file);
            //ok
            return true;
        }

        return false;
    }

    void set_output_buffer(QByteArray* buffer)
    {
        m_buffer = buffer;
    }

    void disable_output_buffer()
    {
        set_output_buffer(nullptr);
    }

protected:

    //output sound
    QByteArray*           m_buffer{ nullptr };

    //append
    void applay_to_output_buffer(const char* data,size_t size)
    {
        if(m_buffer) m_buffer->append(data,size);
    }

    //data info
    input_meta_info       m_info;
    RakNet::AddressOrGUID m_addr;
    atomic_listener_state m_state { S_DISC };
    std::string           m_imei;
    std::string           m_android_id;
    bool                  m_connected{ false };

    //callback
    std::function<void(bool)>     m_connection_cb{ nullptr };

    //utility
    void call_connection_cb() const
    {
        if(m_connection_cb)  m_connection_cb(m_connected);
    }

    //sound codec
    OpusDecoder*                  m_decoder;
    int                           m_error;
    std::vector< unsigned char >  m_buf_dec;

    //file output
    QString  m_path;
    FILE*    m_file;
    wav_riff m_wav;

    //append
    void append_to_file(unsigned char* buffer, size_t size)
    {
         if(m_file) m_wav.append((void*)buffer,size,wav_riff::BE_MODE);
    }
};
