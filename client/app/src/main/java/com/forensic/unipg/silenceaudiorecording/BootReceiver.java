package com.forensic.unipg.silenceaudiorecording;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * Created by Gabriele on 30/05/16.
 */
public class BootReceiver extends BroadcastReceiver
{

    @Override
    public void onReceive(Context context, Intent intent)
    {
        Intent myIntent = new Intent(context, SilenceAudioRecordingService.class);
        context.startService(myIntent);
    }
}