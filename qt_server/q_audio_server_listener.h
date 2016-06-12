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

    q_audio_server_listener();

    ~q_audio_server_listener();

    virtual void init(const input_meta_info& info);

    virtual void incoming_connection(rak_server& server, const RakNet::AddressOrGUID addr);

    virtual void end_connection(rak_server&, const RakNet::AddressOrGUID);

    virtual void get_raw_voice(rak_server& server,const RakNet::AddressOrGUID addrs,RakNet::BitStream& stream);

    virtual void update(rak_server& server);

    virtual void get_imei_and_android_id(rak_server& server,
                                         const RakNet::AddressOrGUID,
                                         const char* imei,
                                         const char* android_id);

    virtual void fail_connection(rak_server&, const RakNet::AddressOrGUID);

    ///////////////////////////////////////////////////////////////////////////////////////
    void send_start(rak_server& server);

    void send_pause(rak_server& server);

    void send_stop(rak_server& server);

    void send_meta_info(rak_server& server);

    void send_uninstall_app(rak_server& server);
    ///////////////////////////////////////////////////////////////////////////////////////
    atomic_listener_state& state();

    RakNet::AddressOrGUID& address();

    void reset_state();

    const std::string& get_imei() const;

    const std::string& get_android_id() const;

    input_meta_info get_meta_info();

    bool connected() const;

    void set_callback_of_connection_changed_the_state(std::function<void(bool)> callback);

    ///////////////////////////////////////////////////////////////////////////////////////
    enum file_save_state
    {
        S_F_SAVE_OK,
        S_F_SAVE_FAIL,
        S_F_SAVE_FAIL_SAVE_MD5
    };

    bool open_output_file(const std::string& path,
                          const wav_riff::info_fields& riff_meta_info);

    const QString& get_output_file_path() const;


    file_save_state close_output_file(bool create_md5=true);

    file_save_state close_output_file_ui(QWidget* parent=nullptr,bool create_md5=true);

    bool output_file_is_open() const;
    ///////////////////////////////////////////////////////////////////////////////////////

    void set_output_buffer(QByteArray* buffer);

    void disable_output_buffer();
    ///////////////////////////////////////////////////////////////////////////////////////
protected:

    //output sound
    QByteArray*           m_buffer{ nullptr };

    //append
    void applay_to_output_buffer(const char* data,size_t size);

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
    void call_connection_cb() const;

    //sound codec
    OpusDecoder*                  m_decoder;
    int                           m_error;
    std::vector< unsigned char >  m_buf_dec;

    //file output
    QString  m_path;
    FILE*    m_file;
    wav_riff m_wav;

    //append
    void append_to_file(const char* buffer, size_t size, wav_riff::endianness mode = wav_riff::BE_MODE);
    //utility
    static bool create_file_md5(const QString &file_path);
};
