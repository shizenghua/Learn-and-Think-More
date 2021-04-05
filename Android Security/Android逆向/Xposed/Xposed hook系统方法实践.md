# Xposed hook系统方法实践

[项目构建](http://eternalsakura13.com/2018/02/04/hook2/)和[xposed下载](http://eternalsakura13.com/2018/01/19/nexus51/)参考我之前的文章。

## Hook类的名称进行内部查找方法（findAndHookMethod)

```
private void hook_method(String className, ClassLoader classLoader, String methodName, Object... parameterTypesAndCallback) {
        try {
            XposedHelpers.findAndHookMethod(className, classLoader, methodName, parameterTypesAndCallback);
        } catch (Exception e) {
            XposedBridge.log(e);
        }
    }
```

## 反射找到具体方法

```
private void hook_method2(String className, String methodName, XC_MethodHook xmh) {
        try {
            Class<?> clazz = Class.forName(className);
            for (Method method : clazz.getDeclaredMethods()) {
                if (method.getName().equals(methodName)
                        && !Modifier.isAbstract(method.getModifiers())
                        && Modifier.isPublic(method.getModifiers())) {
                    XposedBridge.hookMethod(method, xmh);
                }
            }
        } catch (Exception e) {
            XposedBridge.log(e);
        }
    }
```

## 案例测试

### 新建一个class，实现xposed

```
package com.example.xposeddemo;
import android.util.Log;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;
/**
 * Created by sakura on 2018/2/5.
 */
public class Main implements IXposedHookLoadPackage {
    //第一个参数是Hook的类的名称，第二个参数是类的classloader，第三个参数是Hook的具体方法，第四个参数是Hook之后的回调，一般有before和after
    private void hook_method(String className, ClassLoader classLoader, String methodName, Object... parameterTypesAndCallback) {
        try {
            XposedHelpers.findAndHookMethod(className, classLoader, methodName, parameterTypesAndCallback);
        } catch (Exception e) {
            XposedBridge.log(e);
        }
    }
    private void hook_method2(String className, String methodName, XC_MethodHook xmh) {
        try {
            Class<?> clazz = Class.forName(className);
            for (Method method : clazz.getDeclaredMethods()) {
                if (method.getName().equals(methodName)
                        && !Modifier.isAbstract(method.getModifiers())
                        && Modifier.isPublic(method.getModifiers())) {
                    XposedBridge.hookMethod(method, xmh);
                }
            }
        } catch (Exception e) {
            XposedBridge.log(e);
        }
    }
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpp) throws Throwable {
        Log.i("sakura", "pkg:" + lpp.packageName);
        hook_method2("android.telephony.TelephonyManager", "getDeviceId", new XC_MethodHook() {
            @Override
            protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                Log.i("sakura", "hook getDeviceId...");
                Object obj = param.getResult();
                Log.i("sakura", "imei args:" + obj);
                param.setResult("sakura");
            }
        });
    }
}
```

MainActivity.java

```
package com.example.xposeddemo;
import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.widget.TextView;
public class MainActivity extends AppCompatActivity {
    private TextView imeiTxt;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imeiTxt = (TextView) findViewById(R.id.imei);
        TelephonyManager telephonyManager = (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.READ_PHONE_STATE) != PackageManager.PERMISSION_GRANTED) {
            // TODO: Consider calling
            // ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            // public void onRequestPermissionsResult(int requestCode, String[] permissions,
            // int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            return;
        }
        String imei = telephonyManager.getDeviceId();
        imeiTxt.setText("imei:" + imei);
    }
}
```

[![img](https://ws1.sinaimg.cn/large/006tNc79ly1fo5zk72n4uj30u01eon0j.jpg)](https://ws1.sinaimg.cn/large/006tNc79ly1fo5zk72n4uj30u01eon0j.jpg)
[![img](https://ws1.sinaimg.cn/large/006tNc79ly1fo5zmy36l6j30uy1gejv4.jpg)](https://ws1.sinaimg.cn/large/006tNc79ly1fo5zmy36l6j30uy1gejv4.jpg)

[# xposed](https://eternalsakura13.com/tags/xposed/)