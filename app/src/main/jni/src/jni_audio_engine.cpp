//
// Created by Gabriele on 28/05/16.
//
#include <android/log.h>
#include <jni.h>
#include <jni_audio_engine.hpp>
#include <wav_riff.hpp>

namespace java_global
{
    //global values
    sound_context sound_ctx;
}

//test
class test_sound_callback : public  sound_callback
{
public:

    enum sound_state
    {
        START_REC,
        END_REC
    };
    sound_state                     m_state;
    size_t                          m_s_count;
    //buffer
    FILE*                           m_file;
    wav_riff                        m_wav;
    sound_context*                  m_context;
    es_input_device::es_buffer_read m_buffer;

    //init
    virtual void init(sound_context* context)
    {
        //save
        m_context = context;
        //init buffer
        m_buffer.init(m_context->get_input().get_meta_info());
        //append all
        m_context->get_input().enqueue_all(m_buffer);
        //init file output
        m_file = fopen("/sdcard/test.wav","w+");
        //manager
        m_wav.init(m_file,m_context->get_input().get_meta_info(),wav_riff::LE_MODE);
        //count
        m_state   = START_REC;
        m_s_count = 0;
    }

    //update
    virtual void update(SLAndroidSimpleBufferQueueItf bq)
    {
        //test
        if(m_s_count++ > 9) m_state = END_REC;
        //get current
        if(m_state == START_REC)
        {
            m_wav.append(m_buffer.current(),m_buffer.m_one_size,wav_riff::LE_MODE);
        }
        else if(m_state == END_REC && m_file)
        {
            //compute size
            m_wav.complete();
            //close
            std::fclose(m_file);
            m_file = nullptr;
        }
        //current is read
        m_buffer.enqueue(bq);
    }
}
test_callback;



//java methods
extern "C"
{

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_contextInit( JNIEnv *env,
                                                                                              jobject thiz,
                                                                                              jint channels,
                                                                                              jint samples_per_second,
                                                                                              jint bits_per_samples)
{
    java_global::sound_ctx.init({
                                      (SLuint32)channels,              //channels
                                      (SLuint32)samples_per_second,    //samples per second
                                      (SLuint32)bits_per_samples,      //bits per samples
                                      (SLuint32)2                      //queues
                                });


    java_global::sound_ctx.set_callback(&test_callback);

}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_contextClose( JNIEnv *env, jobject thiz)
{
    java_global::sound_ctx.destoy();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_startRecording(JNIEnv *env, jobject thiz)
{
    java_global::sound_ctx.get_input().start_recording();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_stopRecording(JNIEnv *env, jobject thiz)
{
    java_global::sound_ctx.get_input().stop_recording();
}

JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_pauseRecording(JNIEnv *env, jobject thiz)
{
    java_global::sound_ctx.get_input().pause_recording();
}

JNIEXPORT jboolean JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_haveErrors(JNIEnv *env, jobject thiz)
{
    return (jboolean) java_global::sound_ctx.global_have_errors();
}

JNIEXPORT jobjectArray JNICALL Java_com_forensic_unipg_silenceaudiorecording_AudioEngine_getErrors(JNIEnv *env, jobject thiz)
{
    //elements
    size_t n_elements = java_global::sound_ctx.global_count_errors();
    //alloc array of strings
    jobjectArray object_array = (jobjectArray)env->NewObjectArray(n_elements,env->FindClass("java/lang/String"),env->NewStringUTF(""));
    //get all elements
    {
        //index
        int i = 0;
        //get all errors of context
        for(const std::string& error : java_global::sound_ctx)
        {
            env->SetObjectArrayElement(object_array, i++, env->NewStringUTF(error.c_str()));
        }
        //get all errors of engine
        for(const std::string& error : java_global::sound_ctx.get_engine())
        {
            env->SetObjectArrayElement(object_array, i++, env->NewStringUTF(error.c_str()));
        }
        //get all errors of input device
        for(const std::string& error : java_global::sound_ctx.get_input())
        {
            env->SetObjectArrayElement(object_array, i++, env->NewStringUTF(error.c_str()));
        }
    }
    //return
    return object_array;
}


}