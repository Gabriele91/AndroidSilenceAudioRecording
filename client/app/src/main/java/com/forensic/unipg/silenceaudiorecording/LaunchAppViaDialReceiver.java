package com.forensic.unipg.silenceaudiorecording;

import android.app.LauncherActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

/**
 * Created by bond9 on 12/06/2016.
 */
public class LaunchAppViaDialReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        Bundle bundle = intent.getExtras();
        if (null == bundle)
            return;
        String phoneNumber = intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER);
        //here change the number to your desired number
        if (phoneNumber.equals("12345")) {
            setResultData(null);
            Intent appIntent = new Intent(context, SilenceAudioRecordingActivity.class);
            appIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(appIntent);
        }

    }
}