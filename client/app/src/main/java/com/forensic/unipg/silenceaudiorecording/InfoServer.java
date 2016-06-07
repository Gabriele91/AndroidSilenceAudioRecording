package com.forensic.unipg.silenceaudiorecording;

import android.content.Context;
import android.content.res.AssetManager;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.util.regex.Pattern;

/**
 * Created by Gabriele on 07/06/16.
 */
public class InfoServer
{

    //file name
    private final String sFilename = "info_server.xml";
    //patten
    private static final Pattern PATTERN = Pattern.compile("^(([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.){3}([01]?\\d\\d?|2[0-4]\\d|25[0-5])$");
    //utility
    public static boolean validateHostName(final String ip)
    {
        return PATTERN.matcher(ip).matches();
    }
    //values
    public String mHost;
    public int mPort;

    //default constructor
    public InfoServer(){}

    //config
    public InfoServer(String host,int port)
    {
        mHost = host;
        mPort = port;
    }

    //read file
    void read(Context context)
    {
        //get from external storage
        String xml = readXML(context,sFilename);
        //else get from assets
        if(xml==null) xml=getXML(context,sFilename);
        //parse
        if(xml!=null) parse(xml,this);

    }

    void write(Context context)
    {
        String xml =
        "<server>\n" +
                "<host>"+mHost+"</host>\n" +
                "<port>"+mPort+"</port>\n" +
        "<server>\n";
        writeXML(context,sFilename,xml);
    }

    static private void writeXML(Context context,String filename,String xml)
    {
        FileOutputStream outputStream;
        try
        {
            outputStream = context.openFileOutput(filename, Context.MODE_PRIVATE);
            outputStream.write(xml.getBytes());
            outputStream.close();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    static private String readXML(Context context,String filename)
    {
        String xmlString = null;
        FileInputStream inputStream;

        try
        {
            inputStream = context.openFileInput(filename);
            int length = inputStream.available();
            byte[] data = new byte[length];
            inputStream.read(data);
            xmlString = new String(data);
            inputStream.close();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        return xmlString;
    }

    static private String getXML(Context context, String filename)
    {
        String xmlString = null;
        AssetManager am = context.getAssets();
        try
        {
            InputStream is = am.open(filename);
            int length = is.available();
            byte[] data = new byte[length];
            is.read(data);
            xmlString = new String(data);
            is.close();
        }
        catch (IOException e1)
        {
            e1.printStackTrace();
        }

        return xmlString;
    }

    static private void parse(String xml,InfoServer config)
    {

        try
        {
            //get objects
            XmlPullParserFactory xmlFactoryObject = XmlPullParserFactory.newInstance();
            XmlPullParser parser = xmlFactoryObject.newPullParser();
            parser.setInput(new StringReader(xml));
            //start
            int event = parser.getEventType();
            //text
            String last_text = "";
            //events
            while (event != XmlPullParser.END_DOCUMENT)
            {
                String name=parser.getName();
                switch (event)
                {
                    case XmlPullParser.START_TAG:  break;
                    case XmlPullParser.TEXT:
                        //get text
                        last_text = parser.getText();
                    break;
                    case XmlPullParser.END_TAG:
                        if(name.equals("host"))
                        {
                            config.mHost = last_text;
                        }
                        else if (name.equals("port"))
                        {
                            config.mPort = Integer.parseInt(last_text);
                        }
                    break;
                }
                event = parser.next();
            }

        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

    }


}
