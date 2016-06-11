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
#include <q_thread_utilities.h>



enum audio_player_state
{
    S_P_PLAY,
    S_P_PAUSE,
    S_P_STOP
};

class q_audio_player
{

public:

    //init
    void init(const input_meta_info& meta_info)
    {
        //meta info
        m_info = meta_info;
        //output file
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
            if (state == QAudio::IdleState && m_state == S_P_PLAY)
            {
                m_audio_output_device.close();
                m_audio_output_device.setBuffer(&m_buffer);
                m_audio_output_device.open(QIODevice::ReadOnly);
                //post restart
                post_to_main_thread([=]()
                {
                    //start
                    m_q_audio_out->reset();
                    m_q_audio_out->start(&m_audio_output_device);
                });
            }
        });
    }

    void play()
    {
        if(m_q_audio_out)
        {
            //set buffer
            m_audio_output_device.close();
            m_audio_output_device.reset();
            m_audio_output_device.setBuffer(&m_buffer);
            m_audio_output_device.open(QIODevice::ReadOnly);
            //play
            m_state = S_P_PLAY;
            //start
            m_q_audio_out->reset();
            m_q_audio_out->start(&m_audio_output_device);
        }
        else m_state = S_P_STOP;
    }

    void stop()
    {
        //in any cases
        m_state = S_P_STOP;
        //stop audio
        if(m_q_audio_out)
        {
            m_audio_output_device.close();
            m_buffer.resize(0);
            m_q_audio_out->stop();
        }
    }

    void pause()
    {
        if(m_q_audio_out)
        {
            m_q_audio_out->stop();
            m_state = S_P_PAUSE;
        }
        else m_state = S_P_STOP;
    }

    void set_volume(double volume)
    {
        if(m_q_audio_out) m_q_audio_out->setVolume(volume);
    }

    double output_get_volume()
    {
        if(m_q_audio_out) return m_q_audio_out->volume();
        return 1.0;
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

    QByteArray* get_buffer_output()
    {
        return &m_buffer;
    }


private:

    //audio device
    input_meta_info    m_info;
    audio_player_state m_state         { S_P_STOP };
    QAudioOutput*      m_q_audio_out   { nullptr };
    QIODevice*         m_q_audio_device{ nullptr };
    QByteArray         m_buffer;
    QBuffer            m_audio_output_device;

};
