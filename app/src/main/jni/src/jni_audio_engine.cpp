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

        return true;
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

    bool global_have_errors() const
    {
        return  es_objs_errors::have_errors() ||
                m_engine.have_errors()        ||
                m_input.have_errors();
    }

    size_t global_count_errors() const
    {
        return count_errors() +
               m_engine.count_errors() +
               m_input.count_errors();
    }

protected:

    es_engine m_engine;
    es_input_device m_input;
};


static sound_context global_sound_context;

extern "C"
{

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_contextInit( JNIEnv *env,
                                                                                              jobject thiz,
                                                                                              jint channels,
                                                                                              jint samples_per_second,
                                                                                              jint bits_per_samples)
{
    global_sound_context.init({
                                      (SLuint32)channels,              //channels
                                      (SLuint32)samples_per_second,    //samples per second
                                      (SLuint32)bits_per_samples,      //bits per samples
                                      (SLuint32)2                      //queues
                              });
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_contextClose( JNIEnv *env, jobject thiz)
{
    global_sound_context.destoy();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_startRecording(JNIEnv *env, jobject thiz)
{
    global_sound_context.get_input().start_recording();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_stopRecording(JNIEnv *env, jobject thiz)
{
    global_sound_context.get_input().stop_recording();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_pauseRecording(JNIEnv *env, jobject thiz)
{
    global_sound_context.get_input().pause_recording();
}

JNIEXPORT jboolean JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_haveErrors(JNIEnv *env, jobject thiz)
{
    return (jboolean) global_sound_context.global_have_errors();
}

JNIEXPORT jobjectArray JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_getErrors(JNIEnv *env, jobject thiz)
{
    //elements
    size_t n_elements = global_sound_context.global_count_errors();
    //alloc array of strings
    jobjectArray object_array = (jobjectArray)env->NewObjectArray(n_elements,env->FindClass("java/lang/String"),env->NewStringUTF(""));
    //get all elements
    {
        //index
        int i = 0;
        //get all errors of context
        for(const std::string& error : global_sound_context)
        {
            env->SetObjectArrayElement(object_array, i++, env->NewStringUTF(error.c_str()));
        }
        //get all errors of engine
        for(const std::string& error : global_sound_context.get_engine())
        {
            env->SetObjectArrayElement(object_array, i++, env->NewStringUTF(error.c_str()));
        }
        //get all errors of input device
        for(const std::string& error : global_sound_context.get_input())
        {
            env->SetObjectArrayElement(object_array, i++, env->NewStringUTF(error.c_str()));
        }
    }
    //return
    return object_array;
}


}