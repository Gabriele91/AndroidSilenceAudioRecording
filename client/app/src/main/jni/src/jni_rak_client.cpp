//
// Created by Gabriele on 29/05/16.
//
#include <jni.h>
#include <opus/opus.h>
#include <rak_client.hpp>
#include <jni_audio_engine.hpp>
#include <atomic>
#include <cstring>
#include <include/wav_riff.hpp>
//not use opus
//#define USE_RAW_AUDIO
//else use opus

enum client_status
{
    C_S_START           = 0,
    C_S_CONNECTED       = 1,
    C_S_DISCONECTED     = 2,
    C_S_FAIL_TO_CONNECT = 3,
    C_S_FAIL_TO_START   = 4
};


class rak_sound_callback : public rak_client_callback
                          ,public sound_callback
{
public:

    enum sound_state
    {
        NOT_INIT,
        START_REC,
        PAUSE_REC,
        END_REC
    };

    //////////////////////////////////////////////////////////////////////
    //init status
    rak_sound_callback()
    {
        m_status = C_S_START;
    }
    ~rak_sound_callback()
    {
        //change state
        m_status = C_S_DISCONECTED;

#ifndef USE_RAW_AUDIO
        //delete encoder
        if(m_encoder)
        {
            //delete
            opus_encoder_destroy(m_encoder);
            //to null
            m_encoder = nullptr;
        }
#endif
    }
    //special case
    void set_fail_to_start()
    {
        m_status = C_S_FAIL_TO_START;
    }
    //////////////////////////////////////////////////////////////////////
    //sound_callback
    //init
    virtual void init(sound_context* context)
    {
    }

    //update
    virtual void update(SLAndroidSimpleBufferQueueItf bq)
    {
        //get current
        if(m_state == START_REC)
        {
            if(!m_write)
            {
                //alloc
                m_buff_temp.resize(m_buffer.m_one_size);
                //copy
                std::memcpy(m_buff_temp.data(), m_buffer.current(), m_buff_temp.size());
                //enable write
                m_write = true;
            }
        }
        //current is read
        m_buffer.enqueue(bq);
    }
    //////////////////////////////////////////////////////////////////////
    //rak_client_callback
    virtual void msg_config(unsigned int channels,  unsigned int samples, unsigned int bits )
    {
        //delete
        free_sound_ctx();
        //init
        m_sound_ctx.init({
                            (SLuint32)channels,   //channels
                            (SLuint32)samples,    //samples per second
                            (SLuint32)bits,       //bits per samples
                            (SLuint32)2           //queues
                         });
        //set callback
        m_sound_ctx.set_callback(this);
        //init buffer
        m_buffer.init(m_sound_ctx.get_input().get_meta_info());
        //append all
        m_sound_ctx.get_input().enqueue_all(m_buffer);
        //state not rak
        m_state = END_REC;

#ifndef USE_RAW_AUDIO
        #define BITRATE 12000
        //already init encoder? free
        if(m_encoder)
        {
            //delete
            opus_encoder_destroy(m_encoder);
            //to null
            m_encoder = nullptr;
        }
        //init encoder
        m_encoder=opus_encoder_create(samples,channels,OPUS_APPLICATION_AUDIO,&m_opus_err);
        //set the BITRATE
        m_opus_err = opus_encoder_ctl(m_encoder, OPUS_SET_BITRATE(BITRATE));
#endif
    }


    virtual void msg_start_rec( )
    {
        if(m_state == END_REC || m_state == PAUSE_REC)
        {
            m_state = START_REC;
            m_sound_ctx.get_input().start_recording();
        }
    }

    virtual void msg_pause_rec( )
    {
        if(m_state == START_REC)
        {
            m_state = PAUSE_REC;
            m_sound_ctx.get_input().pause_recording();
        }
    }

    virtual void msg_end_rec( )
    {
        if(m_state == START_REC || m_state == PAUSE_REC)
        {
            m_state = END_REC ;
            m_sound_ctx.get_input().stop_recording();
        }
    }

    virtual void new_connection(RakNet::AddressOrGUID addr)
    {
        m_status = C_S_CONNECTED;
        m_addr   = addr;
    }

    virtual void end_connection(RakNet::AddressOrGUID addr)
    {
        m_status = C_S_DISCONECTED;
        free_sound_ctx();
    }

    virtual void fail_connection()
    {
        m_status = C_S_FAIL_TO_CONNECT;
        free_sound_ctx();
    }

    virtual void rak_update(rak_client& client)
    {
        if(m_write)
        {
#ifdef USE_RAW_AUDIO
            //send message
            RakNet::BitStream rak_stream;
            //set message
            rak_stream.Write((RakNet::MessageID)ID_MSG_RAW_VOICE);
            //write buffer
            rak_stream.WriteBits(m_buff_temp.data(),m_buff_temp.size()*8);
            //send
            client.get_interface()->Send(&rak_stream, HIGH_PRIORITY, UNRELIABLE,0,m_addr,false);
#else
            //encode
            m_encode_buffer.resize(m_buff_temp.size());
            //??*le to be*??//
            #if 0
            case 0: newsize=sampling_rate/400; break;
            case 1: newsize=sampling_rate/200; break;
            case 2: newsize=sampling_rate/100; break;
            case 3: newsize=sampling_rate/50; break;
            case 4: newsize=sampling_rate/25; break;
            case 5: newsize=3*sampling_rate/50; break;
            #endif
            //int16 buffer size
            unsigned int buff16_div        = 25;
            unsigned int buff16_size       = m_buff_temp.size() / sizeof(opus_int16) ;
            unsigned int buff16_size_frame = buff16_size / buff16_div;
            opus_int16* buff16_ptr         = (opus_int16*)m_buff_temp.data();
            //write n
            //send message
            RakNet::BitStream rak_stream;
            rak_stream.Write((RakNet::MessageID)ID_MSG_RAW_VOICE);
            rak_stream.Write(buff16_div);
            //
            for(int i=0;i!=buff16_div;++i)
            {
                //page ptr
                opus_int16* buff16_ptr_page = &buff16_ptr[buff16_size_frame*i];
                //encode
                int new_size = opus_encode(m_encoder,
                                           buff16_ptr_page,
                                           buff16_size_frame,
                                           m_encode_buffer.data(),
                                           m_encode_buffer.size() );



                if(new_size > 0)
                {
                    //write size
                    rak_stream.Write(new_size);
                    //write buffer
                    rak_stream.WriteBits(m_encode_buffer.data(), new_size * 8);
                }
                else
                {
                    __android_log_print(ANDROID_LOG_ERROR,
                                        "rak_client",
                                        "Opus compression failed with identifier %d.\n",
                                        new_size);
                }
            }
            //send
            client.get_interface()->Send(&rak_stream,HIGH_PRIORITY, RELIABLE_ORDERED, 0, m_addr, false);

#endif
             //end write
            m_write = false;
        }
    }

    void free_sound_ctx()
    {
        if(m_state != NOT_INIT)
        {
            //stop rec
            if(m_state == START_REC)
            {
                m_sound_ctx.get_input().stop_recording();
            }
            //delete
            m_sound_ctx.destoy();
            //reset
            m_state = NOT_INIT;
        }
    }
    //////////////////////////////////////////////////////////////////////
    client_status get_status() const
    {
        return m_status;
    }

protected:

    //data sound
    sound_context                   m_sound_ctx;
    sound_state                     m_state      {NOT_INIT};
    es_input_device::es_buffer_read m_buffer;

    //data rak net
    RakNet::AddressOrGUID           m_addr;
    std::atomic< bool >             m_write      { false };
    std::vector < unsigned  char >  m_buff_temp;
    client_status                   m_status;

    //opus
#ifndef USE_RAW_AUDIO
    OpusEncoder*                   m_encoder        {nullptr};
    int                            m_opus_err       { 0 };
    std::vector <  unsigned char > m_encode_buffer;
#endif

};

namespace java_global
{
    //global values
    rak_client          client;
    rak_sound_callback  client_callback;
}

extern "C"
{

    JNIEXPORT jboolean JNICALL Java_com_forensic_unipg_silenceaudiorecording_RakClient_start( JNIEnv *env, jclass clazz, jstring host )
    {
        //get ip
        const char* c_str_host= env->GetStringUTFChars( host , NULL );
        //init
        bool status = java_global::client.init(&java_global::client_callback,c_str_host);
        //start loop
        if(status) java_global::client.loop();
        else       java_global::client_callback.set_fail_to_start();
        //return state
        return (jboolean)status;
    }

    JNIEXPORT jboolean JNICALL Java_com_forensic_unipg_silenceaudiorecording_RakClient_stop( JNIEnv *env, jclass clazz )
    {
        //stop
        java_global::client.stop_loop();
        //stopped?
        if(!java_global::client.is_loop())
        {
            //disable connection
            java_global::client.destroy();
            //return
            return true;
        }
        return false;
    }

    JNIEXPORT jint JNICALL Java_com_forensic_unipg_silenceaudiorecording_RakClient_state( JNIEnv *env, jclass clazz )
    {
        return java_global::client_callback.get_status();
    }
}