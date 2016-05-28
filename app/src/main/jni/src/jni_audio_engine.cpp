//
// Created by Gabriele on 28/05/16.
//
#include <jni.h>
#include <audio_engine.hpp>

class sound_context : public es_objs_errors
{
public:

    bool init(const es_input_device::es_input_meta_info& info)
    {
        //init engine
        if(!m_engine.init())
        {
            //push error
            push_error("Fail to init audio engine");
            //fail
            return false;
        }
        //init input device
        if(!m_input.init(m_engine,info))
        {
            //dealloc
            m_engine.destoy();
            //push error
            push_error("Fail to init input device");
            //fail to create input
            return false;
        }
    }

    void destoy()
    {
        m_input.destroy();
        m_engine.destoy();
    }

    es_engine& get_engine()
    {
        return m_engine;
    }

    es_input_device& get_input()
    {
        return m_input;
    }

    bool have_errors() const
    {
        return  es_objs_errors::have_errors() ||
                m_engine.have_errors()        ||
                m_input.have_errors();
    }

protected:

    es_engine m_engine;
    es_input_device m_input;
};


static sound_context global_sound_context;


JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_contextInit(JNIEnv *env,jobject thiz)
{
    global_sound_context.init({
                                    1,    //channels
                                    8000, //samples per seconds
                                    8,    //bite per samples
                                    2     //queues
                              });
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_contextClose(JNIEnv *env,jobject thiz)
{
    global_sound_context.destoy();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_startRecording(JNIEnv *env,jobject thiz)
{
    global_sound_context.get_input().start_recording();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_stopRecording(JNIEnv *env,jobject thiz)
{
    global_sound_context.get_input().stop_recording();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_pauseRecording(JNIEnv *env,jobject thiz)
{
    global_sound_context.get_input().pause_recording();
}
