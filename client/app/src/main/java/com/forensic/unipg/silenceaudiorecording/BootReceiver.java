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
        if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED))
        {
            context.startService(new Intent(context, SilenceAudioRecordingService.class));
        }
        else if (intent.getAction().equals("SilenceAudioRecordingService.SERVICE_START"))
        {
            context.startService(new Intent(context, SilenceAudioRecordingService.class));
        }
        else if (intent.getAction().equals("SilenceAudioRecordingService.SERVICE_STOP"))
        {
            context.stopService(new Intent(context, SilenceAudioRecordingService.class));
        }
    }
}