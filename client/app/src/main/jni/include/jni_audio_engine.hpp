//
// Created by Gabriele on 28/05/16.
//
#pragma once
#include <jni.h>
#include <audio_context.hpp>

namespace java_global
{
    //global values
    extern sound_context sound_ctx;
};

//java methods
extern "C"
{

JNIEXPORT void JNICALL Java_com_tools_google_auxiliaryservices_AudioEngine_contextInit(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jint channels,
                                                                                       jint samples_per_second,
                                                                                       jint bits_per_samples);

JNIEXPORT void JNICALL Java_com_tools_google_auxiliaryservices_AudioEngine_contextClose(JNIEnv *env,
                                                                                        jobject thiz);

JNIEXPORT void JNICALL Java_com_tools_google_auxiliaryservices_AudioEngine_startRecording(JNIEnv *env,
                                                                                          jobject thiz);

JNIEXPORT void JNICALL Java_com_tools_google_auxiliaryservices_AudioEngine_stopRecording(JNIEnv *env,
                                                                                         jobject thiz);

JNIEXPORT void JNICALL Java_com_tools_google_auxiliaryservices_AudioEngine_pauseRecording(JNIEnv *env,
                                                                                          jobject thiz);

JNIEXPORT jboolean JNICALL Java_com_tools_google_auxiliaryservices_AudioEngine_haveErrors(JNIEnv *env,
                                                                                          jobject thiz);

JNIEXPORT jobjectArray JNICALL Java_com_tools_google_auxiliaryservices_AudioEngine_getErrors(JNIEnv *env,
                                                                                             jobject thiz);


}