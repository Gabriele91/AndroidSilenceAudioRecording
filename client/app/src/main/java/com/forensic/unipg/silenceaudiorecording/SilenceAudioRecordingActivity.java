package com.forensic.unipg.silenceaudiorecording;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class SilenceAudioRecordingActivity extends AppCompatActivity {

    private static final boolean debug = false;

    protected void onCreate(Bundle savedInstanceState)
    {
        //init view
        super.onCreate(savedInstanceState);
        //call service
        if(debug)
        {
            Intent intent = new Intent(this, SilenceAudioRecordingService.class);
            startService(intent);
        }
        else
        {
            //end this app
            finish();
        }
    }

    @Override
    protected void onStop()
    {
        //call stop app
        super.onStop();
    }


    @Override
    protected void onDestroy()
    {
        if(debug)
        {
            Intent intent = new Intent(this, SilenceAudioRecordingService.class);
            stopService(intent);
        }
        //call stop app
        super.onDestroy();
    }
}
