package com.forensic.unipg.silenceaudiorecording;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class SilenceAudioRecording extends AppCompatActivity {

    static
    {
        System.load("libSilenceAudioRecordingNative.so");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        //init view
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_silence_audio_recording);
        //instance
        mAudioEngine = AudioEngine.instance(8000,8);
        //errors?
        if(mAudioEngine.haveErrors())
        {
            for(String error:mAudioEngine.getErrors())
            {
                Log.e("AudioEngine",error);
            }
        }
    }

    //attribute
    AudioEngine mAudioEngine = null;
}
