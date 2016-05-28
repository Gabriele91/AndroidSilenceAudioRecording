#pragma once
#include <jni.h>

extern "C"
{
	JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_SilenceAudioRecording_NativeInit(JNIEnv *env,jobject thiz);
	JNIEXPORT void JNICALL Java_com_forensic_unipg_silenceaudiorecording_SilenceAudioRecording_NativeClose(JNIEnv *env,jobject thiz);
}