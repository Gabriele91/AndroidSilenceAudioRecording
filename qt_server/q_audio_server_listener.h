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
        //init q audio
        init_q_audio();
        //init decoder
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
        //write sound output buffer
        m_buffer.append((const char*)m_buf_dec.data(),data_size);
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

    size_t output_pos() const
    {
        return m_audio_output_device.pos();
    }

    short output_value()
    {
        if(m_audio_output_device.isOpen())
        {
            //get pos
            auto dev_pos = m_audio_output_device.pos();
            //test
            if(dev_pos+1<m_buffer.size())
            {
                //get
                const char buffer[] =
                {
                    m_buffer.at(dev_pos),
                    m_buffer.at(dev_pos+1)
                };
                return *((short*)buffer);
            }
        }
        return 0;
    }

    void output_play()
    {
        //set buffer
        m_audio_output_device.close();
        m_audio_output_device.reset();
        m_audio_output_device.setBuffer(&m_buffer);
        m_audio_output_device.open(QIODevice::ReadOnly);
        //start
        m_q_audio_out->reset();
        m_q_audio_out->start(&m_audio_output_device);
    }

    void output_stop()
    {
        m_q_audio_out->stop();
        m_audio_output_device.close();
        m_buffer.resize(0);
    }

    void output_set_volume(double volume)
    {
        if(m_q_audio_out) m_q_audio_out->setVolume(volume);
    }

    double output_get_volume()
    {
        if(m_q_audio_out) return m_q_audio_out->volume();
        return 1.0;
    }

    void change_connession_callback(std::function<void(bool)> callback)
    {
        m_connection_cb = callback;
    }

    bool save_file(const std::string& path, bool clear=false)
    {
        //attributes
        FILE* m_file = nullptr;
        wav_riff m_wav;
        //open file
        m_file = fopen(path.c_str(),"w");
        //open?
        if(m_file)
        {
            //init file
            m_wav.init(m_file, m_info, wav_riff::LE_MODE);
            //append
            m_wav.append((void*)m_file_buffer.data(), m_file_buffer.size(), wav_riff::BE_MODE);
            //compute size
            m_wav.complete();
            //close
            fclose(m_file);
            //clear
            if(clear)
            {
                m_file_buffer.clear();
            }
            //success
            return true;
        }
        return false;
    }

    void clear_buffer_file()
    {
        m_file_buffer.clear();
    }

protected:

    //audio device
    QAudioOutput*  m_q_audio_out   { nullptr };
    QIODevice*     m_q_audio_device{ nullptr };
    QByteArray     m_buffer;
    QBuffer        m_audio_output_device;

    //init
    void init_q_audio()
    {
        if(m_q_audio_out || m_q_audio_device)
        {
            //stop rip
            if(m_q_audio_out->state() == QAudio::StoppedState)
            {
                m_q_audio_out->stop();
            }
            //close device
            if(m_q_audio_device && m_q_audio_device->isOpen())
            {
                m_q_audio_device->close();
            }
            //delete device
            if(m_q_audio_device) delete m_q_audio_device;
            if(m_q_audio_out)    delete m_q_audio_out;
        }
        //set format
        QAudioFormat format;
        format.setSampleRate(m_info.m_samples_per_sec);
        format.setChannelCount(m_info.m_channels);
        format.setSampleSize(m_info.m_bits_per_sample);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::SignedInt);
        //if 0 buffer
        if(!m_buffer.size())
        {
            m_buffer.resize(m_info.m_samples_per_sec*
                            m_info.m_channels*
                            (m_info.m_bits_per_sample / 8));
            m_buffer.fill(0);
        }
        //audio info
        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        //test
        if (!info.isFormatSupported(format))
        {
            //remade format
            format = info.nearestFormat(format);
        }
        //autput
        m_q_audio_out   = new QAudioOutput(format);
        //set buffer
        m_audio_output_device.close();
        m_audio_output_device.setBuffer(&m_buffer);
        m_audio_output_device.open(QIODevice::ReadOnly);
        //volume max
        m_q_audio_out->setVolume(1.0);
        //connect
        QObject::connect(m_q_audio_out,&QAudioOutput::stateChanged,
        [this](QAudio::State state)
        {
            if (state == QAudio::IdleState && m_state == S_REC)
            {
                m_audio_output_device.close();
                m_audio_output_device.setBuffer(&m_buffer);
                m_audio_output_device.open(QIODevice::ReadOnly);
                qDebug()<< "added some data\n";
            }
        });
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
    std::vector< unsigned char >   m_file_buffer;
    //append
    void append_to_file(unsigned char* buffer, size_t size)
    {
        if(!size) return;
        //old size
        size_t current_size = m_file_buffer.size();
        //new size
        size_t new_size = current_size + size;
        //alloc
        m_file_buffer.resize(new_size);
        //copy data
        std::memcpy(&m_file_buffer[current_size],buffer,size);
        //end
    }
};
