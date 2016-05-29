//
// Created by Gabriele on 29/05/16.
//
#include <jni.h>
#include <rak_client.hpp>

namespace java_global
{
    //global values
    rak_client client;
}


class test_listener : public rak_client_callback
{
public:

    virtual void msg_config(unsigned int channels,  unsigned int samples, unsigned int bits )
    {

    }

    virtual void msg_start_rec( )
    {

    }

    virtual void msg_end_rec( )
    {

    }

    virtual void new_connection()
    {

    }

    virtual void end_connection()
    {

    }
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