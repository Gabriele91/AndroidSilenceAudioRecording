//
// Created by Gabriele on 29/05/16.
//
#include <jni.h>
#include <rak_client.hpp>
#include <jni_audio_engine.hpp>
#include <atomic>
#include <cstring>

namespace java_global
{
    //global values
    rak_client client;
}


class test_callback : public rak_client_callback
                     ,public sound_callback
{
public:

    enum sound_state
    {
        NOT_INIT,
        START_REC,
        END_REC
    };
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
            //alloc
            m_buff_temp.resize(m_buffer.m_one_size);
            //copy
            std::memcpy(m_buff_temp.data(),m_buffer.current(),m_buff_time.size());
            //enable write
            m_write = true;

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
        //init buffer
        m_buffer.init(m_sound_ctx.get_input().get_meta_info());
        //append all
        m_sound_ctx.get_input().enqueue_all(m_buffer);
        //state not rak
        m_state = END_REC;
    }

    virtual void msg_start_rec( )
    {
        if(m_state == END_REC)
        {
            m_state = START_REC;
            m_sound_ctx.get_input().start_recording();
        }
    }

    virtual void msg_end_rec( )
    {
        if(m_state == START_REC)
        {
            m_state = END_REC ;
            m_sound_ctx.get_input().stop_recording();
        }
    }

    virtual void new_connection(RakNet::AddressOrGUID addr)
    {
        m_addr = addr;
    }

    virtual void end_connection(RakNet::AddressOrGUID addr)
    {
        free_sound_ctx();
    }

    virtual void rak_update(rak_client& client)
    {
        if(m_write)
        {
            //send message
            RakNet::BitStream rak_stream;
            rak_stream.Write(rak_id_msg::ID_MSG_RAW_VOICE);
            rak_stream.WriteBits(m_buff_temp.data(),m_buff_temp.size()*8);
            //send
            client.get_interface()->Send(&rak_stream, HIGH_PRIORITY, UNRELIABLE,0,m_addr,false);
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

protected:

    //data sound
    sound_context                   m_sound_ctx;
    sound_state                     m_state      {NOT_INIT};
    es_input_device::es_buffer_read m_buffer;
    //data rak net
    RakNet::AddressOrGUID           m_addr;
    std::atomic< bool >             m_write      { false };
    std::vector < unsigned  char >  m_buff_temp;

}
test_rak_callback;

extern "C"
{

    JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_RakClient_start( JNIEnv *env, jclass clazz )
    {
        java_global::client.init(&test_rak_callback,"95.250.196.2");
        java_global::client.loop();
    }
    JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_RakClient_stop( JNIEnv *env, jclass clazz )
    {
        java_global::client.stop_loop();
    }
}