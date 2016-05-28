package com.forensic.unipg.silenceaudiorecording;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class SilenceAudioRecording extends AppCompatActivity {

    static
    {
        System.load("SilenceAudioRecordingLib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_silence_audio_recording);
    }
}
