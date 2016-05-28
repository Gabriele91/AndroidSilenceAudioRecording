package com.forensic.unipg.silenceaudiorecording;

import android.content.Context;
import android.media.AudioManager;

/**
 * Created by Gabriele on 28/05/16.
 */
public class AudioEngine
{
    static public AudioEngine instance(Context context)
    {
        if(sAudioEngine==null)
        {
            sAudioEngine= new AudioEngine(context);
        }
        return sAudioEngine;
    }

    static public AudioEngine instance(int sample_rate)
    {
        if(sAudioEngine==null)
        {
            sAudioEngine= new AudioEngine(sample_rate);
        }
        return sAudioEngine;
    }

    static public AudioEngine instance(int sample_rate,int sample_size)
    {
        if(sAudioEngine==null)
        {
            sAudioEngine= new AudioEngine(sample_rate,sample_size);
        }
        return sAudioEngine;
    }


    protected AudioEngine(Context context)
    {
        AudioManager myAudioMgr = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        String nativeSampleRate =  myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        contextInit(1,Integer.parseInt(nativeSampleRate),16);
    }

    protected AudioEngine(int sample_rate)
    {
        contextInit(1,sample_rate,16);
    }

    protected AudioEngine(int sample_rate,int sample_size)
    {
        contextInit(1,sample_rate,sample_size);
    }

    protected void finalize() throws Throwable
    {
        contextClose();
    }

    //native
    public native boolean haveErrors();
    public native String[] getErrors();

    protected native void contextInit( int channels,  int samples_per_second, int bits_per_samples);
    protected native void contextClose();
    protected native void startRecording();
    protected native void stopRecording();
    protected native void pauseRecording();

    //attributes
    protected static AudioEngine sAudioEngine = null;

}
