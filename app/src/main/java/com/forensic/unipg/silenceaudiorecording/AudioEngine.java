package com.forensic.unipg.silenceaudiorecording;

/**
 * Created by Gabriele on 28/05/16.
 */
public class AudioEngine
{

    native void contextInit();
    native void contextClose();
    native void startRecording();
    native void stopRecording();
    native void pauseRecording();

}
