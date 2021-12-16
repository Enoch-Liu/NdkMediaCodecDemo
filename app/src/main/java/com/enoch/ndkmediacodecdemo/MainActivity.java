package com.enoch.ndkmediacodecdemo;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.enoch.ndkmediacodecdemo.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private ActivityMainBinding binding;
    Button btnPlay;
    Integer succeedTimes = 0;
    Integer failedTimes = 0;
    TextView sv;
    public void setSv() {
        String s = "Succeed:" + succeedTimes.toString() + "---Failed:" + failedTimes.toString();
        sv.setText(s);
    };
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView cv = findViewById(R.id.cur_state);

        sv = findViewById(R.id.statistics_state);


        btnPlay = findViewById(R.id.aplay);
        btnPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                checkAndRequirePermission();
                if(autoplay()) {
                    succeedTimes = succeedTimes + 1;
                    cv.setText("Auto play succeed.");
                } else {
                    failedTimes = failedTimes + 1;
                    cv.setText("Auto play failed.");
                }
                setSv();
            }
        });

        Button btnInit = findViewById(R.id.init);
        btnInit.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                checkAndRequirePermission();
                if (init()) {
                    cv.setText("Init succeed.");
                } else {
                    cv.setText("Init failed.");
                }
            }
        });

        Button btnSeek = findViewById(R.id.seek);
        btnSeek.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (seek(0)) {
                    cv.setText("Seek succeed.");
                } else {
                    cv.setText("Seek failed.");
                }
            }
        });

        Button btnPlay = findViewById(R.id.play);
        btnPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (play(999)) {
                    release();
                    cv.setText("Play succeed.");
                    succeedTimes = succeedTimes + 1;
                } else {
                    release();
                    cv.setText("Play failed.");
                    failedTimes = failedTimes + 1;
                }
                setSv();
            }
        });

        Button btnFlag = findViewById(R.id.flag);
        btnFlag.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.e("DECCODE_DEMO", "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
                cv.setText("Flag succeed.");
            }
        });

        Button btnPlayMid = findViewById(R.id.playmid);
        btnPlayMid.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (play(2)) {
                    cv.setText("Play to 2 sec succeed.");
                    succeedTimes += 1;
                } else {
                    cv.setText("Play to 2 sec failed.");
                    failedTimes += 1;
                }
                setSv();
            }
        });

        Button btnSeekBegin = findViewById(R.id.seekbegin);
        btnSeekBegin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (seek(0)) {
                    cv.setText("Seek to 0 succeed.");
                } else {
                    cv.setText("Seek to 0 failed.");
                }
            }
        });

        Button btnPlayEnd = findViewById(R.id.playtoend);
        btnPlayEnd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (play(4)) {
                    cv.setText("Play to 4 sec succeed.");
                    succeedTimes += 1;
                } else {
                    cv.setText("Play to 4 sec failed.");
                    failedTimes += 1;
                }
                setSv();
            }
        });

        Button btnSeekMid = findViewById(R.id.seekmid);
        btnSeekMid.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (seek(2)) {
                    cv.setText("Seek to 2 sec succeed.");
                } else {
                    cv.setText("Seek to 2 sec failed.");
                }
            }
        });
    }

    private void checkAndRequirePermission() {
        final String[] PERMISSIONS_STORAGE = {
                "android.permission.READ_EXTERNAL_STORAGE",
                "android.permission.WRITE_EXTERNAL_STORAGE"
        };

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                //进入到这里代表没有权限.
                if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                    //已经禁止提示了
                    AlertDialog.Builder builder = new AlertDialog.Builder(this);
                    builder.setCancelable(false)
                            .setMessage("应用需要存储权限来让您选择手机中的相片！")
                            .setNegativeButton("取消", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    //Toast.makeText(MainActivity.this, "点击了取消按钮", Toast.LENGTH_LONG).show();
                                    Toast.makeText(MainActivity.this, "没有存储器的读写权限", Toast.LENGTH_LONG).show();
                                }
                            })
                            .setPositiveButton("确定", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    ActivityCompat.requestPermissions(MainActivity.this, PERMISSIONS_STORAGE, 1);
                                }
                            }).show();
                } else {
                    ActivityCompat.requestPermissions(this, PERMISSIONS_STORAGE, 1);
                }
            } else {
                return ;
            }
        }
        return ;
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native boolean autoplay();
    public native boolean init();
    public native boolean seek(int sec);
    public native boolean play(int sec);
    public native boolean release();
}