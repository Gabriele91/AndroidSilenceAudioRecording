package com.forensic.unipg.silenceaudiorecording;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class SilenceAudioRecordingActivity extends AppCompatActivity {

    private static final int  REQUEST_RECORD_AUDIO     = 200;
    private static final int  REQUEST_READ_PHONE_STATE = 201;
    //state
    private boolean acceptRECORD_AUDIO     = false;
    private boolean acceptREAD_PHONE_STATE = false;
    //debug
    private static final boolean debug = false;

    protected void onCreate(Bundle savedInstanceState)
    {
        //init view
        super.onCreate(savedInstanceState);
        //test audio permission
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED)
        {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO}, REQUEST_RECORD_AUDIO);
        }
        else
        {
            acceptRECORD_AUDIO = true;
        }
        //test phone state permission
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_PHONE_STATE) != PackageManager.PERMISSION_GRANTED)
        {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_PHONE_STATE}, REQUEST_READ_PHONE_STATE);
        }
        else
        {
            acceptREAD_PHONE_STATE = true;
        }
        //call service
        if(debug)
        {
            Intent intent = new Intent(this, SilenceAudioRecordingService.class);
            startService(intent);
        }
        else
        {
            //end this app
            if(acceptRECORD_AUDIO && acceptREAD_PHONE_STATE)
            {
                finish();
            }
        }
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults)
    {
        switch (requestCode)
        {
            case REQUEST_RECORD_AUDIO:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
                {
                    acceptRECORD_AUDIO = true;
                }
                else
                {
                    acceptRECORD_AUDIO = false;
                }
            break;
            case REQUEST_READ_PHONE_STATE:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
                {
                    acceptRECORD_AUDIO = true;
                }
                else
                {
                    acceptRECORD_AUDIO = false;
                }
            break;

        }

        if(!debug)
        {
            //end this app
            if (acceptRECORD_AUDIO && acceptREAD_PHONE_STATE)
            {
                finish();
            }
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
