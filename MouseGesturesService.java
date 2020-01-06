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
            instance.execute("su -c /data/shutdown");
            //context.stopService(new Intent(context, MouseGesturesService.class));
    }

    void execute(String command) {
        try
        {
            Runtime.getRuntime().exec(command);
        }
        catch (Exception e)
        {
            Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_LONG).show();
            toggle(getApplicationContext());
        }
    }

    @Override
	public void onCreate() {
		super.onCreate();
		instance = this;
        execute("su -c /data/mousegest /dev/input/event4");
	}

    @Override
    public void onDestroy() {
        instance = null;
        Toast.makeText(getApplicationContext(), "stopped Mouse Gestures", Toast.LENGTH_SHORT).show();
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}

