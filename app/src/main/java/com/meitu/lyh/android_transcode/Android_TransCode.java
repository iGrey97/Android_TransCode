package com.meitu.lyh.android_transcode;

import android.os.Handler;
import android.os.Message;

import java.security.PublicKey;

public class Android_TransCode {

    private String i_file;
    private String o_file;
    private int width;
    private int height;
    private int channels;
    private int audio_samp_rate;
    private Handler transHandler;

    public Android_TransCode(Handler transHandler,String i_file,String o_file,int width,int height,int channels,int audio_samp_rate){
        this.transHandler=transHandler;
        this.i_file=i_file;
        this.o_file=o_file;
        this.width=width;
        this.height=height;
        this.channels=channels;
        this.audio_samp_rate=audio_samp_rate;
    }
    public int Transcode(){
        return jni_trancode( i_file, o_file, width, height, channels, audio_samp_rate);
    }
    static {
        System.loadLibrary("android_transcode");
    }
    public native int jni_trancode(String i_file,String o_file,int width,int height,int channels,int audio_samp_rate);

    public void sendInfo(int what, String info){

        Message message = Message.obtain();
        message.what = what;
        message.obj = info;
        transHandler.sendMessage(message);

    }

}
