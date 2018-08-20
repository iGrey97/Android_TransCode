package com.meitu.lyh.android_transcode;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;

import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Date;


public class MainActivity extends AppCompatActivity {
    private static final int FILE_SELECT_CODE = 0;
    private Button btnOPen;
    private Button btnStart;
    public String strIn;
    private EditText editIn;
    private EditText editout;
    private EditText editWidth;
    private EditText editHeight;
    private EditText editChannels;
    private EditText editSamples;

    private TextView progressTv;
    private TextView infoTv;
    private ProgressBar progressBarHorizontal;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btnOPen=(Button)findViewById(R.id.btnOpen);
        btnStart=(Button)findViewById(R.id.btnStart);
        editIn=(EditText)findViewById(R.id.editIn);
        editout=(EditText)findViewById(R.id.editOut);
        editWidth=(EditText)findViewById(R.id.editWidth);
        editHeight=(EditText)findViewById(R.id.editHeight);
        editChannels=(EditText)findViewById(R.id.editChannels);
        editSamples=(EditText)findViewById(R.id.editSamples);

        progressTv=(TextView)findViewById(R.id.progressTv);
        infoTv=(TextView)findViewById(R.id.infoTv);
        progressBarHorizontal=(ProgressBar)findViewById(R.id.progressBarHorizontal);

        btnOPen.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.setType("*/*");//设置类型，这里设的是任意类型，任意后缀的可以这样写。
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                try {
                    startActivityForResult( Intent.createChooser(intent, "Select a File to Transcode"), FILE_SELECT_CODE);
                } catch (android.content.ActivityNotFoundException ex) {
                    Toast.makeText(MainActivity.this,"Please install files manager",Toast.LENGTH_SHORT).show();
                    return;
                }

            }
        });
        btnStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {


                if(editIn.getText().toString().isEmpty()||editout.getText().toString().isEmpty()){
                    //抛异常
                    Log.e("Android_TransCode","Edit In/Out is Empty");
                    return;
                }
                Log.i("Android_TransCode",editIn.getText().toString());
////                   public Android_TransCode(String i_file,String o_file,int width,int height,int channels,int audio_samp_rate)
//
//                Android_TransCode transcode = new Android_TransCode("/storage/sdcard0/11/num10.mp4", "/storage/sdcard0/11/lyh.mp4",
//                        200, 200,
//                        1, 44100);
                new Thread(){
                    @Override
                    public void run() {
                        super.run();
                        //进行自己的操作
                        int intWidth=-1;
                        int intHeight=-1;
                        int intChannels=-1;
                        int intSamples=-1;

                        //类内定义线程可以访问类内对象
                        String strIn= editIn.getText().toString();
                        String strOut=editout.getText().toString();
                        String strWidth=editWidth.getText().toString();
                        String strHeight=editHeight.getText().toString();
                        String strChannels=editChannels.getText().toString();
                        String strSamples=editSamples.getText().toString();

//                        if(strIn.isEmpty()||strOut.isEmpty()){
//                            //抛异常
//                            Log.e("Android_TransCode","Edit In/Out is Empty");
//                            return;
//                        }
                        Log.i("Android_TransCode",strIn);
                        Log.i("Android_TransCode",strOut);

                        if(!strWidth.isEmpty()){
                            intWidth=Integer.parseInt(strIn);
                        }
                        if(!strHeight.isEmpty()){
                            intHeight=Integer.parseInt(strHeight);
                        }
                        if(!strChannels.isEmpty()){
                            intChannels=Integer.parseInt(strChannels);
                        }
                        if(!strSamples.isEmpty()){
                            intSamples=Integer.parseInt(strSamples);
                        }
                        Android_TransCode transcode = new Android_TransCode(strIn, strOut,
                                intWidth,intHeight, intChannels, intSamples);
                        Log.i("Android_TransCode","Begin");

                        Date curDate = new Date(System.currentTimeMillis());




                        transcode.Transcode();
                        Date endDate = new Date(System.currentTimeMillis());
                        double diff = endDate.getTime() - curDate.getTime();
                        Log.i("Android_TransCode","spend time"+diff/1000.0+"s");
//                        Toast.makeText(MainActivity.this,"spend time"+diff,Toast.LENGTH_SHORT).show();//会崩掉，MainActivity在新线程中不能用

                        Log.i("Android_TransCode","end");
                    }
                }.start();

            }
        });



//        Android_TransCode("/storage/sdcard0/11/num10.mp4","/storage/sdcard0/11/lyh.mp4");


    }



    //startActivityForResult的回调函数，另一个界面finish后回调
    protected void onActivityResult(int requestCode, int resultCode, Intent data){
        if (resultCode == Activity.RESULT_OK) {
            Uri uri = data.getData();
//            PathUtils path=new PathUtils();
            strIn=getPath(uri,this);
//            String temp=strIn;
//            editIn.setText(temp);

            editIn.setText(strIn);
//            editIn.setText(1);
            Toast.makeText(MainActivity.this,strIn,Toast.LENGTH_SHORT).show();

        }
    }
    public  String getPath( Uri uri,final Context context ){
        String path;
//         Uri uri = data.getData();
        if ("file".equalsIgnoreCase(uri.getScheme())){//使用第三方应用打开、equalsIgnoreCase忽略大小写比较
            path = uri.getPath();
        }
        if ( Build.VERSION.SDK_INT > Build.VERSION_CODES.KITKAT) {//4.4以后
            path =  getRealPathFromUriAboveApi19(context, uri);
//        Toast.makeText(this,pathin,Toast.LENGTH_SHORT).show();
        } else {//4.4以下的系统调用方法
            path = getRealPathFromUriBelowAPI19(uri);
//                Toast.makeText(MainActivity.this, pathin, Toast.LENGTH_SHORT).show();
        }
        return path;
    }



    public String getRealPathFromUriBelowAPI19(Uri contentUri) {
        String res = null;
        String[] proj = { MediaStore.Images.Media.DATA };
        Cursor cursor = getContentResolver().query(contentUri, proj, null, null, null);
        if(null!=cursor&&cursor.moveToFirst()){;
            int column_index = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
            res = cursor.getString(column_index);
            cursor.close();
        }
        return res;
    }


    /**
     * 专为Android4.4设计的从Uri获取文件绝对路径，以前的方法已不好使
     */
    @SuppressLint("NewApi")
    public String getRealPathFromUriAboveApi19(final Context context, final Uri uri) {


        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;
        // DocumentProvider
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            // ExternalStorageProvider
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];
                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }
            }
            // DownloadsProvider
            else if (isDownloadsDocument(uri)) {
                final String id = DocumentsContract.getDocumentId(uri);
                final Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));


                return getDataColumn(context, contentUri, null, null);
            }
            // MediaProvider
            else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];


                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }
                final String selection = "_id=?";
                final String[] selectionArgs = new String[]{split[1]};
                return getDataColumn(context, contentUri, selection, selectionArgs);
            }

        }
        // MediaStore (and general)
        else if ("content".equalsIgnoreCase(uri.getScheme())) {
            return getDataColumn(context, uri, null, null);
        }
        // File
        else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }
        return null;
    }


    /**
     * Get the value of the data column for this Uri. This is useful for
     * MediaStore Uris, and other file-based ContentProviders.
     *
     * @param context       The context.
     * @param uri           The Uri to query.
     * @param selection     (Optional) Filter used in the query.
     * @param selectionArgs (Optional) Selection arguments used in the query.
     * @return The value of the _data column, which is typically a file path.
     */
    public String getDataColumn(Context context, Uri uri, String selection, String[] selectionArgs) {


        Cursor cursor = null;
        final String column = "_data";
        final String[] projection = {column};


        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,null);
            if (cursor != null && cursor.moveToFirst()) {
                final int column_index = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(column_index);
            }
        } finally {
            if (cursor != null)
                cursor.close();
        }
        return null;
    }


    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    public boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }


    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    public boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    public boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

}
