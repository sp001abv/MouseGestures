package com.sp.mousegestures;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

public class MainActivity extends Activity {
	@Override
    protected void onCreate(Bundle savedInstanceState) { 
		super.onCreate(savedInstanceState);
		MouseGesturesService.toggle(getApplicationContext());
		finish(); // Just close the Activity after the toggle
	}
}
