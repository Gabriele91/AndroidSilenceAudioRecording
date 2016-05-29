package com.forensic.unipg.silenceaudiorecording;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class SilenceAudioRecording extends AppCompatActivity {

    static
    {
        System.load("libSilenceAudioRecordingNative.so");
    }

    protected void onCreate(Bundle savedInstanceState)
    {
        //init view
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_silence_audio_recording);
        //start client
        RakClient.start();
    }

    @Override
    protected void onStop()
    {
        //stop client
        RakClient.stop();
        //call stop app
        super.onStop();
    }

    /*
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        //init view
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_silence_audio_recording);
        //instance
        //mAudioEngine = AudioEngine.instance(16000,16);
        mAudioEngine = AudioEngine.instance(8000,16);
        //errors?
        if(mAudioEngine.haveErrors())
        {
            for(String error:mAudioEngine.getErrors())
            {
                Log.e("AudioEngine",error);
            }
        }
        else
        {
            mAudioEngine.startRecording();
        }
    }

    //attribute
    AudioEngine mAudioEngine = null;
    */
}
