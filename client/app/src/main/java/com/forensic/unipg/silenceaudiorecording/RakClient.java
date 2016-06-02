package com.forensic.unipg.silenceaudiorecording;

/**
 * Created by Gabriele on 29/05/16.
 */
public class RakClient
{

    public static final int C_S_START           = 0;
    public static final int C_S_CONNECTED       = 1;
    public static final int C_S_DISCONECTED     = 2;
    public static final int C_S_FAIL_TO_CONNECT = 3;
    public static final int C_S_FAIL_TO_START   = 4;

    static public native boolean start(String host, int port);
    static public native void    setIMEI(String imei);
    static public native void    setAndroidID(String androidID);
    static public native boolean stop();
    static public native int     state();
}
