package com.sp.mousegestures;
import android.content.*;

public class StartAtBootReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        MouseGesturesService.toggle(context);
    }
}
