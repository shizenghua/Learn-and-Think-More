# Debugging Krita on Android

url：https://www.sh-zam.com/2019/05/debugging-krita-on-android.html

Well, the easiest way is to use [Android Studio](https://developer.android.com/studio).

Import the project in Android Studio as a `gradle` project and build the project. Krita build *will* fail when run from Android Studio. Now, to run it successfully, we’ll have to manually provide the path to [`installPrefix`](https://invent.kde.org/kde/krita/blob/660972c3835b022d8e0816ee65e60e75568ef0af/packaging/android/apk/build.gradle#L25) or comment out `copyLibs` [dependency](https://invent.kde.org/kde/krita/blob/660972c3835b022d8e0816ee65e60e75568ef0af/packaging/android/apk/build.gradle#L104). Now, the project *should* build properly.

You might want to [change the debug type](https://developer.android.com/studio/debug#debug-types) to **`Native`** or **`Dual`**, as their **`Auto`** mode did not work for me. Open the C++ file in Android Studio and set a breakpoint. Click the bug icon, sit back and watch while Android Studio does all the magic for you.

And then it is usual `lldb` (in Android Studio) or GUI if that’s what you prefer.

### Using command line:

Starting Android studio takes a lot of time and memory. Then, it builds which takes an additional few minutes, so it really isn’t a good idea to use it for debugging every time the app crashes. So, the less time consuming one and a bit complex method – here we go!

Assuming the app has been installed with the debug key. The first step is to launch it in debug mode, to do so:

```shell
# domain/(launcher activity or exported activity's class-path)
$ adb shell am start -D -n "org.kde.krita/org.krita.android.MainActivity"
```

Now the app on phone should launch and show `Waiting for Debugger` message. While it waits – open a terminal and enter `$ adb shell`, and then look for `lldb-server` in `/data/local/tmp/`. If you ever debugged app through Android Studio, then it *should* exist. If it does not, then launch Android Studio and run it in debug mode 😂…hahahaha.

Just kidding, push the file to that location.

```shell
$ adb push $ANDROID_SDK_ROOT/lldb/<version>/android/<abi>/lldb-server /data/local/tmp
$ adb shell chmod +x /data/local/tmp/lldb-server
```

(No `lldb` directory? See notes)

Then for us to access the libraries, we’ll have to copy it to `/data/data/org.kde.krita`, for that:

```shell
$ adb shell run-as org.kde.krita cp /data/local/tmp/lldb-server /data/data/org.kde.krita
```

(Why `run-as`? It is a `setuid` program and gives us the necessary permission to access [the sandbox](https://source.android.com/security/app-sandbox)).

Now, enter the app sandbox by first entering the `$ adb shell` and then `$ run-as org.kde.krita`.

Run the `lldb-server` like you would if you were remote debugging.

```shell
$ ./lldb-server platform --server --listen "<incoming-ip>:<port>"
$ # Example: allow any ip on port 9999
$ ./lldb-server platform --server --listen "*:9999"
```

Now on the host machine, run `lldb` and then

```shell
(lldb) platform select remote-android
(lldb) platform connect connect://<ip>:<port>
```

On my machine:

```lldb
(lldb) platform select remote-android
  Platform: remote-android
 Connected: no
(lldb) platform connect connect://localhost:9999
  Platform: remote-android
    Triple: arm-*-linux-android
OS Version: 28.0.0 (4.4.153-15659493)
    Kernel: #2 SMP PREEMPT Thu Apr 4 18:31:57 KST 2019
  Hostname: localhost
 Connected: yes
WorkingDir: /data/data/org.kde.krita
```

You can read more about what they do on [LLVM’s website](https://lldb.llvm.org/use/remote.html).
(This is one a time setup you can keep the server and client connected)

Remember, that our process is still `Waiting for Debugger`? :(
Let’s give it what it wants. Attach the debugger to the running process’s pid, which can be known by `$ adb shell ps | grep "krita"` or `$ pgrep "krita"`

To attach:

```shell
(lldb) attach <pid>
(lldb) # on my machine
(lldb) attach 1818
Process 1818 stopped
* thread #1, name = 'org.kde.krita', stop reason = signal SIGSTOP
    frame #0: 0xe8d35f7c libc.so`syscall + 28
<and much more>
```

Still didn’t continue? :-<
So, let’s **finally** resume it!

We’ll have to resume it over Java Debug Wire Protocol (JDWP), we’ll use `jdb`

```shell
$ adb forward tcp:12345 jdwp:<pid> # the same pid which we attached in lldb
$ jdb -attach localhost:12345
```

Now `continue` the process in `lldb` and we are done!

(This might seem like a lot, but it really isn’t. Every time the app crashes, I run in debug mode `$ attach pid` and I get the backtrace immediately!)

PS: When I was looking for it on the internet, I didn’t find much about it and had to spend a lot of time on this.
This method *should* work with debugging any android app with `lldb`, obv!
(I am really new to blogging. If it’s hard to understand or my formatting is bad, I am really sorry.)

### Notes

- I hate the extra `jdb` thing, and if the function which you want to debug is not going to be called during the early start up, you can use `-N` flag instead of `-D` with `am`.
- Can’t find `lldb` directory in your SDK? Use platform tools to install it.
- `jdb` doesn’t attach? `$ killall android-studio && adb kill-server #_#`

#### Resources

https://source.android.com/devices/tech/debug/gdb
https://android.googlesource.com/platform/ndk/+/master/ndk-gdb.py
https://lldb.llvm.org/use/remote.html