package com.sp.mousegestures;

import android.app.*;
import android.os.*;
import android.content.*;

import android.widget.Toast;

public class MouseGesturesService extends Service
{
    static MouseGesturesService instance;

    public static void toggle(Context context){
        if (instance == null)
            context.startService(new Intent(context, MouseGesturesService.class));
        else
            context.stopService(new Intent(context, MouseGesturesService.class));
    }

    static void execute(Context context, String command) {
        try
        {
            Runtime.getRuntime().exec(command);
        }
        catch (Exception e)
        {
            Toast.makeText(context, e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    @Override
	public void onCreate() {
        super.onCreate();
        instance = this;
        execute(getApplicationContext(), "/data/startmousegest.sh");
	}

    @Override
    public void onDestroy() {
        instance = null;
        execute(getApplicationContext(), "/data/stopmousegest.sh");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}

