# 看雪3W4月第三题frida辅助尝试脱壳

看雪培训脱壳4月第三题，爱加密的方法级别保护，方法的opcode被抽取后，替换成了一个静态方法，执行时调用静态方法修复opcode。
脱下来后发现是方法级别的保护，在ArtMethod::Invoke处方法还是没有修复。可以尝试在执行后或者在解释器中执行前拖opcode，我还没完全搞定。

 

解题过程：

1. 拖进jdax看一下，爱加密
2. 没有下载老师的脱壳机，先用手里的试试看。

```
private void i() {
    H.i(6040);
}
 
/* access modifiers changed from: private */
public void j() {
    H.i(6041);
}
 
/* access modifiers changed from: protected */
public void b() {
    H.i(6031);
}
 
/* access modifiers changed from: protected */
public void c() {
    aju.a = false;
    a((Application) this);
    h();
}
 
/* access modifiers changed from: protected */
public void d() {
    H.i(6035);
}
 
public void e() {
    C1377e.a((Context) this).a((C1376d) new C1376d() {
        public void a(@Nullable String str) {
            H.i(6023);
        }
 
        public void a(@NonNull String str, @NonNull String str2) {
            H.i(6024);
        }
    });
}
```



看一下smali代码

```
.registers 3
.annotation system Ldalvik/annotation/Signature;
    value = {
        "(",
        "Ljava/lang/Class<",
        "*>;)Z"
    }
.end annotation
 
goto :goto_38
 
:goto_1
nop
 
nop
 
nop
 
nop
 
nop
 
nop
 
nop
 
nop
 
nop
```

判断这种应该是将inis拖出来后，替换成一个java方法，在代码执行时候使用`H.i`方法对inis进行修复。

下载fart尝试脱壳。

3. 尝试使用frida对H.i函数进行直接调用。

```
// app直接奔溃，有个大佬告诉我要注意堆栈的连续性，没懂
var JavaH = Java.use("s.h.e.l.l.H");
var JavaClass = Java.use("java.lang.Class");
var JavaObject = Java.use("java.lang.Object");
for (var i = 0; i < 9999; i++) {
    JavaH.i(i);
    console.log("call function H.i(" + i + ")");
}
```

4. 既然直接调用不行，就使用frida主动调用每个函数，来尝试获取修复后的dex。

```
Java.perform(function () {
            var JavaDexFile = Java.use("dalvik.system.DexFile");
            var JavaH = Java.use("s.h.e.l.l.H");
            var JavaClass = Java.use("java.lang.Class");
            var JavaObject = Java.use("java.lang.Object");
            // for (var i = 0; i < 9999; i++) {
            //     JavaH.i(i);
            //     console.log("call function H.i(" + i + ")");
            // }
 
            // 1. 主动调用自己的脱壳机
            var JavaApkDumper = Java.use("android.app.ApkDumper");
            var JavaModifier = Java.use("java.lang.reflect.Modifier");
            var classLoaders = Java.enumerateClassLoadersSync();
            classLoaders.forEach(function (loader) {
                if (loader.toString().indexOf("assistant") > -1) {
                    console.log("loader info->" + loader);
                    JavaApkDumper.threadDumpDex(loader, "com.xgtl.assistant");
                }
            });
 
            // 2. 添加一个thread
            var JavaRunnable = Java.use("java.lang.Runnable");
            var JavaMethod = Java.use("java.lang.reflect.Method");
            var TestRunnable = Java.registerClass({
                name: "com.ingeek.TestRunnable",
                implements: [JavaRunnable],
                fields: {
                    methodObject: 'java.lang.reflect.Method',
                    methodInvoke: 'java.lang.Object',
                    methodArgs: '[Ljava.lang.Object;'
                },
                methods: {
                    $init: [{
                        returnType: "void",
                        argumentTypes: ["java.lang.reflect.Method", "java.lang.Object", "[Ljava.lang.Object;"],
                        implementation: function (methodObj, methodInv, methodAs) {
                            this.methodObject = methodObj;
                            this.methodInvoke = methodInv;
                            this.methodArgs = methodAs;
                            console.log("TestRunnable cons is called!");
                        }
                    }],
                    run: [{
                        returnType: 'void',
                        implementation: function () {
                            try {
                                Java.cast(this.methodObject, JavaMethod).invoke(this.methodInvoke, this.methodArgs);
                            } catch (e) {
                                console.log("TestRunnable run error!", e);
                            }
 
                            console.log("TestRunnable run is called!");
                        }
                    }]
                }
            });
 
            // console.log("TestRunnable", TestRunnable.$new(null, null, [null]).getClass().getDeclaredMethods(), TestRunnable.$new(null, null, [null]).getClass().getConstructors());
 
            // private void saveDexFilesToSDcard(Object cookie, String pkgname, ClassLoader mClassLoader)
            // JavaDexFile.saveDexFilesToSDcard.implementation = function (cookie, pkgname, loader) {
            //     console.log("test");
            //     var classnameArray = this.getClassNameList(cookie);
            //     classnameArray.forEach(function (classname) {
            //         try {
            //             this.loadClass(classname);
            //         } catch (e) {
            //             console.log("error classname->" + classname + "error info->" + e);
            //         }
            //     });
            //     this.saveDexFilesToSDcard(cookie, pkgname, loader);
            // }
 
            // private static native Class defineClassNative(String name, ClassLoader loader, Object cookie, DexFile dexFile)
            JavaDexFile.defineClassNative.implementation = function (name, loader, cookie, dexFile) {
                var ret = this.defineClassNative(name, loader, cookie, dexFile);
                // console.log("defineClassNative classname->" + name + ", ret is " + ret);
                if (name.indexOf("android") == 0) {
                    // console.log("class name", name);
                } else {
                    if (ret) {
                        console.log("class name", name);
                        var JavaClazzObject = Java.cast(ret, JavaClass);
                        var isAbstract = JavaModifier.isAbstract(JavaClazzObject.getModifiers());
                        var cons = JavaClazzObject.getConstructors();
                        var methods = JavaClazzObject.getDeclaredMethods();
                        var flag = false;
                        javaClassNameArray.forEach(function (classname) {
                            // console.log("filter class name", classname, "target class name", name);
                            if (name.indexOf(classname) == 0) {
                                flag = true;
                            }
                        });
                        console.log("flag info", flag, "isAbstract", isAbstract);
                        if ((!isAbstract) && (!flag)) {
                            for (var i = 0; i < cons.length; i++) {
                                var con = cons[i];
                                var conName = con.getName();
                                con.setAccessible(true);
                                console.log("constructor name", conName, "argsLen", con.getParameterTypes().length);
                                var argsLen = con.getParameterTypes().length;
                                if (argsLen == 0) {
                                    try {
                                        //调用构造器后，构造器修复了。
                                        con.newInstance(null);
                                    } catch (e) {
                                        console.log("call cons error", e);
                                    }
                                }
                            }
                            for (var i = 0; i < methods.length; i++) {
                                var method = methods[i];
                                if (!JavaModifier.isNative(method.getModifiers())) {
                                    var methodName = method.getName();
                                    var argsLen = method.getParameterTypes().length;
                                    var exceptionTypeLen = method.getExceptionTypes().length;
                                    console.log("method name", methodName, "argsLen", argsLen, "exceptionTypeLen", exceptionTypeLen);
                                    method.getParameterTypes().forEach(function (type) {
                                        console.log("method parameter type", type.getName());
                                    });
 
                                    method.setAccessible(true);
                                    // Java.vm.tryGetEnv().fromReflectedMethod(method);
 
                                    // if (JavaModifier.isStatic(method.getModifiers())) {
                                    // if (argsLen == 0) {
                                    // try {
                                    try {
                                        var argsArray = Java.array("java.lang.Object", null);
                                        console.log("call static method " + method.getName(), "method", method.getName(), "null", argsArray);
                                        var testRunnable = TestRunnable.$new(Java.cast(method, JavaMethod), null, argsArray);
                                        Java.use("java.lang.Thread").$new(testRunnable).start();
                                    } catch (e) {
                                        console.log("invoke method error~");
                                    }
                                    // method.invoke(null, Java.array("java.lang.Object", new Array(argsLen)));
                                    // } catch (e) {
                                    //     console.log("call static method error", e);
                                    // }
                                    // }
                                    // } else {
                                    //     try {
                                    //         console.log("call no static method " + method.getName());
                                    //         method.invoke(null, Java.array("java.lang.Object", new Array(argsLen)));
                                    //     } catch (e) {
                                    //         console.log("call no static method error", e);
                                    //     }
                                    //
                                    // }
 
                                }
                            }
                        }
                    }
 
                }
 
                // Java.cast(ret, JavaClass).getConstructors().forEach(function (con) {
                //     console.log("constructor info", con.getName());
                //     try {
                //         var argsLen = con.getParameterTypes().length;
                //         var JavaObjectArray = Java.array("java.lang.Object", new Array(argsLen));
                //         con.newInstance(JavaObjectArray);
                //     } catch (e) {
                //         console.log("invoke constructor error", e);
                //     }
                // });
 
                // Java.cast(ret, JavaClass).getDeclaredMethods().forEach(function (method) {
                //     console.log("method info", method.getName());
                //     try{
                //         var argsLen = method.getParameterTypes().length;
                //         var JavaObjectArray = Java.array("java.lang.Object", new Array(argsLen));
                //         method.invoke(null, JavaObjectArray);
                //
                //     }catch (e) {
                //         console.log("invoke method error", e);
                //     }
                //
                // });
                return ret;
            }
        });
```

5. 调用的时候出现了退出，查看后台报错，怀疑是有些特殊方法导致的退出，尝试添加类过滤。

   

```
var javaClassNameArray = ["anet/channel/monitor/fivn",
    "anet/channel/monitor/b$a", "com/alipay/sdk/app/a",
    "com/baidu/mapapi/map/l$a", "com/alibaba/sdk/android/oss/network/ProgressTouchableRequestBody",
    "cn/smssdk/utils/SPHelper", "com/baidu/mapapi/map/Polyline",
    "anet/channel/AccsSessionManager", "com/baidu/mapapi/map/l$a",
    "com/alipay/sdk/app/H5PayActivity", "com/baidu/mapapi/map/offline/MKOfflineMap",
    "com/baidu/platform/comapi/map/s",
    "com/baidu/mapapi/UIMsg$UIOffType",
    "com/alibaba/sdk/android/oss/network/OSSRequestTask",
    "com/baidu/mapapi/search/core/RouteNode",
    "com/alipay/sdk/auth/k",
    "com/baidu/pano/platform/c/j",
    "com/liulishuo/okdownload/i",
    "com/baidu/mapapi/utils/route/IllegalRoutePlanArgumentException",
    "com/bumptech/glide/n",
    "com/baidu/platform/comapi/map/k",
    "com/baidu/vi/c",
    "com/baidu/mapapi/search/share/RouteShareURLOption$RouteShareMode",
    "com/baidu/mapapi/search/poi/PoiBoundSearchOption",
    "com/baidu/mapapi/search/share/LocationShareURLOption",
    "anet/channel/GlobalAppRuntimeInfo", "com/liulishuo/filedownloader/message/e",
    "com/bumptech/glide/R", "com/baidu/mapapi/search/route/MassTransitRoutePlanOption",
    "com/baidu/platform/comapi/commonutils/a", "com/baidu/platform/comapi/NativeLoader",
    "com/baidu/platform/comjni/engine/AppEngine",
    "com/liulishuo/filedownloader/message/SmallMessageSnapshot",
    "com/bumptech/glide/l", "com/baidu/mapframework/open/aidl/a$a$a",
    "com/baidu/mapapi/search/route/MassTransitRouteLine$TransitStep",
    "com/baidu/location/b/h", "com/baidu/mapapi/search/core/VehicleInfo",
    "com/baidu/mapapi/map/MapView", "com/baidu/mapapi/search/busline/BusLineResult",
    "com/baidu/location/c/e", "com/baidu/platform/comjni/util/JNIMD5",
    "com/baidu/location/Poi", "com/baidu/platform/util/a",
    "com/baidu/location/LocationClientOption", "com/baidu/mapapi/search/share/a",
    "com/baidu/mapapi/search/poi/PoiNearbySearchOption",
    "com/alibaba/sdk/android/oss/internal/ResponseParser",
    "com/baidu/mapapi/search", "com/baidu/mapapi/map/BaiduMap$OnMarkerClickListener",
    "com/alibaba/sdk/android/oss/internal/InternalRequestOperation$4",
    "com/baidu/location", "com/baidu/mapapi/search/route/WalkingRouteResult",
    "anetwork/channel/entity/c", "com/lody/virtual/server/content/b",
    "com/baidu/mapapi/cloud/CloudSearchResult",
    "anet/channel/entity/a", "com/baidu/mapapi/CoordType",
    "com/baidu/mapapi/map/MarkerOptions$MarkerAnimateType",
    "com/alibaba/sdk/android/oss/common/OSSLogToFileUtils$1",
    "com/baidu/lbsapi/tools/CoordinateConverter$COOR_TYPE",
    "com/alibaba/sdk/android/oss/common/auth/OSSStsTokenCredentialProvider",
    "com/baidu/location/c/e$c",
    "com/alibaba/sdk/android/oss/model/GetBucketACLResult",
    "com/baidu/lbsapi/auth/LBSAuthManager",
    "com/baidu/location/c/j", "com/baidu/location/c/j$a",
    "com/alibaba/sdk/android/oss/model",
    "com/baidu/location/c/a/d$b",
    "com/baidu/lbsapi/auth/c$a",
    "com/alibaba/sdk/android/oss/internal/ResponseParsers$ListPartsResponseParser",
    "com/alibaba/sdk/android/oss/internal/CheckCRC64DownloadInputStream",
    "com/baidu/location/c/e$b",
    "com/baidu/lbsapi/auth/l",
    "com/baidu/location/f/d$c",
    "com/baidu/pano/platform",
    "com/alibaba/sdk/android/oss/OSSImpl",
    "com/baidu/mapapi/cloud/CloudSearchResult",
    "com/baidu/location/g/a$a",
    "com/baidu/location/e/k$a",
    "com/baidu/location/a/h",
    "com/baidu/mapapi/model/c", "com/lody/virtual/client/stub/ShadowContentProvider$P18",
    "com/baidu/lbsapi/model/BaiduPanoData",
    "com/baidu/mapapi/map/UiSettings",
    "com/baidu/location/a/j",
    "com/alibaba/sdk", "com/baidu/platform/comapi/map/ai",
    "com/baidu/location/c/a/d", "com/lody/virtual/client/stub/ShadowActivity$P78",
    "com/lody/virtual/client/stub/ShadowActivity",
    "com/baidu/location/f/f", "com/baidu/mapapi/map/Dot",
    "cn/smssdk/b/a$1", "com/baidu/mapapi/search/route/TransitRouteLine$TransitStep",
    "anet/channel/strategy/dispatch",
    "com/baidu/location",
    "com/alibaba/sdk/android/oss/internal",
    "com/baidu/location/a/q",
    "com/baidu/mapapi/map/BaiduMap",
    "com/baidu/mapapi",
    "com/baidu/platform",
    "com/alibaba/sdk/android/oss/model",
    "com/lody/virtual/client",
    "com/liulishuo/filedownloader/services",
    "cn/smssdk/contact",
    "com/lody/virtual/client/stub/ShadowContentProvider",
    "com/baidu/platform/comjni/map/basemap/a",
    "com/baidu/mapapi/cloud/CloudManager",
    "com/alibaba/sdk/android/oss/OSSImpl",
    "com/baidu/mapapi/map/MapBaseIndoorMapInfo$SwitchFloorError",
    "com/baidu/mapapi/map/InfoWindow",
    "com/liulishuo/okdownload/g",
    "com/alipay/sdk/auth/AuthActivity",
    "cn/smssdk/net/b$a",
    "com/baidu/mapapi/map/PolylineOptions",
    "com/alibaba/sdk/android/oss/common/auth/OSSAuthCredentialsProvider",
    "com/bumptech/glide/k", "com/baidu/mapapi/search/geocode/GeoCodeResult",
    "cn/smssdk/net/a", "com/mob/MobCommunicator",
    "com/alibaba/sdk/android/oss/model/CompleteMultipartUploadRequest",
    "com/bumptech/glide/g", "com/lody/virtual/client/stub/ShadowDialogActivity$P65",
    "com/baidu/android/bbalbs/common/util/b",
    "anet/channel/util/StringUtils", "com/baidu/mapapi/map/q",
    "com/bumptech/glide/d", "com/lody/virtual/helper/b",
    "com/baidu/mapapi/map", "com/alipay/sdk/app/H5AuthActivity",
    "anet/channel/Config", "com/baidu/platform/comapi/map/j",
    "com/bumptech/glide", "com/baidu/platform/domain",
    "com/baidu/mapapi/search/route", "com/lody/virtual/client/stub/ShadowService$P65",
    "com/baidu/mapapi/map/BaiduMapOptions", "com/lody/virtual/client/stub/ShadowService",
    "com/baidu/pano/platform/c/f", "com/alibaba/sdk/android/oss/model/AbortMultipartUploadResult",
    "com/lody/virtual/remote/vloc/VLocation", "com/baidu/platform/comjni/map/radar/a",
    "com/baidu/mapapi/search/route/IndoorRouteLine", "com/baidu/mapapi/radar/RadarNearbySearchOption",
    "com/lody/virtual/client/stub/ShadowActivity$P44", "com/liulishuo/filedownloader/services/h",
    "com/baidu/location", "com/alipay/sdk/auth/j",
    "com/liulishuo/filedownloader/message/LargeMessageSnapshot$PendingMessageSnapshot", "com/baidu/location/a/q",
    "com/alipay/sdk/app/c", "com/baidu/mapapi/map/WearMapView", "com/liulishuo/filedownloader/model/FileDownloadTaskAtom",
    "com/baidu/platform/comapi/map/e$a", "com/lody/virtual/client/stub/ShadowDialogActivity",
    "com/baidu/mapapi/search/core/TrainInfo", "com/lody/virtual/client/stub", "com/lody/virtual/remote/InstallOptions",
    "com/alibaba/sdk/android/httpdns/g", "com/baidu/lbsapi/auth/e", "com/baidu/mapframework/open/aidl/IComOpenClient",
    "anetwork/channel/aidl/DefaultProgressEvent", "com/baidu/mapapi/utils/poi/IllegalPoiSearchArgumentException",
    "com/baidu/platform/comjni/map/commonmemcache/a", "com/alipay/sdk/app/d$a", "com/baidu/pano/platform/a/a/a",
    "com/baidu/mapapi/navi/NaviParaOption",
    "com/baidu/mapapi/map/n", "com/alibaba/sdk/android/oss/network/ExecutionContext",
    "com/baidu/vi/b", "com/baidu/mapapi/search/busline/OnGetBusLineSearchResultListener",
    "com/baidu/location/e/c", "com/baidu/mapapi/search/core/TransitResultNode", "anetwork/channel/aidl/h$a$a",
    "anet/channel/util/g", "com/baidu/location/c/e$c", "cn/smssdk/utils/a", "anet/channel/Config$Builder",
    "com/baidu/location/e/d$b", "com/baidu/pano/platform/a/a/e$b", "com/baidu/location/BDLocationListener",
    "com/liulishuo/filedownloader/model/FileDownloadModel", "com/alibaba/sdk/android/oss/model",
    "com/baidu/mapapi/map/offline", "com/baidu/mapapi/navi/BaiduMapNavigation",
    "com/baidu/mapapi/map/m", "com/baidu/mapapi/utils/CoordinateConverter", "anet/channel/util/ALog",
    "anet/channel/strategy/dispatch", "com/alibaba/sdk/android/oss/internal/ResumableUploadTask",
    "com/baidu/mapapi/map/WearMapView", "com/baidu/mapapi/search/poi/PoiIndoorResult"];
```

6. 增加的太多了，还是没有有效解决退出的问题，思考是否可以像老师教的那样通过阻断函数执行来解决问题。

   通过查看源码。

   

```
// java_lang_reflect_Method.cc
static jobject Method_invoke(JNIEnv* env, jobject javaMethod, jobject javaReceiver,
                             jobject javaArgs) {
  ScopedFastNativeObjectAccess soa(env);
  return InvokeMethod(soa, javaMethod, javaReceiver, javaArgs);
}
 
// reflection.cc
 jobject InvokeMethod(const ScopedObjectAccessAlreadyRunnable& soa, jobject javaMethod,
                     jobject javaReceiver, jobject javaArgs, size_t num_frames) {
......
  // 比较了参数个数
  if (arg_count != classes_size) {
    ThrowIllegalArgumentException(StringPrintf("Wrong number of arguments; expected %d, got %d",
                                               classes_size, arg_count).c_str());
    return nullptr;
  }
......
 
  // 调用了这里
  InvokeWithArgArray(soa, m, &arg_array, &result, shorty);
......
}
 
// reflection.cc
static void InvokeWithArgArray(const ScopedObjectAccessAlreadyRunnable& soa,
                               ArtMethod* method, ArgArray* arg_array, JValue* result,
                               const char* shorty)
    REQUIRES_SHARED(Locks::mutator_lock_) {
  uint32_t* args = arg_array->GetArray();
  if (UNLIKELY(soa.Env()->check_jni)) {
    CheckMethodArguments(soa.Vm(), method->GetInterfaceMethodIfProxy(kRuntimePointerSize), args);
  }
  // 调用了这里
  method->Invoke(soa.Self(), args, arg_array->GetNumBytes(), result, shorty);
}
 
// art_method.cc
 
void ArtMethod::Invoke(Thread* self, uint32_t* args, uint32_t args_size, JValue* result,
                       const char* shorty) {
 
  // 自己觉得这里dump可以了，后来老师指点了一下，还是需要深入的。
  if (UNLIKELY(__builtin_frame_address(0) < self->GetStackEnd())) {
    ThrowStackOverflowError(self);
    return;
  }
......
  if (!IsStatic()) {
          (*art_quick_invoke_stub)(this, args, args_size, self, result, shorty);
        } else {
          (*art_quick_invoke_static_stub)(this, args, args_size, self, result, shorty);
        }
......
}
 
// https://www.jianshu.com/p/2ff1b63f686b
// https://www.cnblogs.com/lanrenxinxin/p/5207174.html
// 加固的app都是用Interpreter解释执行的，所以我们看看这个函数
void EnterInterpreterFromInvoke(Thread* self,
                                  ArtMethod* method,
                                  ObjPtr<mirror::Object> receiver,
                                  uint32_t* args,
                                  JValue* result,
                                  bool stay_in_interpreter) {
    ......
    if (LIKELY(!method->IsNative())) {
      JValue r = Execute(self, code_item, *shadow_frame, JValue(), stay_in_interpreter);
      if (result != nullptr) {
        *result = r;
      }
    ......
}
 
// Execute函数
static inline JValue Execute(
    Thread* self,
    const DexFile::CodeItem* code_item,
    ShadowFrame& shadow_frame,
    JValue result_register,
    bool stay_in_interpreter = false) REQUIRES_SHARED(Locks::mutator_lock_) {
  DCHECK(!shadow_frame.GetMethod()->IsAbstract());
  DCHECK(!shadow_frame.GetMethod()->IsNative());
  if (LIKELY(shadow_frame.GetDexPC() == 0)) {
 
// 看一下android的解释器：
// 在android8.1下面，解释器为两种：Switch解释器与汇编解释器：
// art/runtime/interpreter/interpreter.cc 默认配置的是汇编解释器，我们如果要对方法级别的保护做dump的话，需要在这里修改，然后dump opcode，如果是arm的话，修改起来比较难，我们可以修改为switch解释器，然后再尝试修改dump opcode。
enum InterpreterImplKind {
  kSwitchImplKind,        // Switch-based interpreter implementation.
  kMterpImplKind          // Assembly interpreter
};
- static constexpr InterpreterImplKind kInterpreterImplKind = kMterpImplKind;
+ static constexpr InterpreterImplKind kInterpreterImplKind = kSwitchImplKind;
 
// 然后看/art/runtime/interpreter/interpreter_switch_impl.cc这个文件
// 其中的参数self表示当前线程，shadow_frame类似于java虚拟机中的stack frame,用于存储方法中的本地变量表、操作栈方法出口等信息。
template<bool do_access_check, bool transaction_active>
JValue ExecuteSwitchImpl(Thread* self, const DexFile::CodeItem* code_item,
                         ShadowFrame& shadow_frame, JValue result_register,
                         bool interpret_one_instruction) {
......
  ArtMethod* method = shadow_frame.GetMethod();
  //在这里dump opcode，抄一下老师的dumpDexFileByExecute方法。
  jit::Jit* jit = Runtime::Current()->GetJit();
......
}
```

​      分析到这里，基本就可以修改函数，炒一炒老师的代码，完成脱壳了，这部分就需要我修改源码了，慢慢改。

7. 附上我写的完整的frida脚本，没有拖出来完整的dex。

```
/**
 * frida -H 127.0.0.1:1337 -f com.xgtl.assistant -l hook_kanxue03.js --no-pause
 *
 */
function hook() {
    hook_java();
    hook_native();
}
 
var javaClassNameArray = ["anet/channel/monitor/fivn",
    "anet/channel/monitor/b$a", "com/alipay/sdk/app/a",
    "com/baidu/mapapi/map/l$a", "com/alibaba/sdk/android/oss/network/ProgressTouchableRequestBody",
    "cn/smssdk/utils/SPHelper", "com/baidu/mapapi/map/Polyline",
    "anet/channel/AccsSessionManager", "com/baidu/mapapi/map/l$a",
    "com/alipay/sdk/app/H5PayActivity", "com/baidu/mapapi/map/offline/MKOfflineMap",
    "com/baidu/platform/comapi/map/s",
    "com/baidu/mapapi/UIMsg$UIOffType",
    "com/alibaba/sdk/android/oss/network/OSSRequestTask",
    "com/baidu/mapapi/search/core/RouteNode",
    "com/alipay/sdk/auth/k",
    "com/baidu/pano/platform/c/j",
    "com/liulishuo/okdownload/i",
    "com/baidu/mapapi/utils/route/IllegalRoutePlanArgumentException",
    "com/bumptech/glide/n",
    "com/baidu/platform/comapi/map/k",
    "com/baidu/vi/c",
    "com/baidu/mapapi/search/share/RouteShareURLOption$RouteShareMode",
    "com/baidu/mapapi/search/poi/PoiBoundSearchOption",
    "com/baidu/mapapi/search/share/LocationShareURLOption",
    "anet/channel/GlobalAppRuntimeInfo", "com/liulishuo/filedownloader/message/e",
    "com/bumptech/glide/R", "com/baidu/mapapi/search/route/MassTransitRoutePlanOption",
    "com/baidu/platform/comapi/commonutils/a", "com/baidu/platform/comapi/NativeLoader",
    "com/baidu/platform/comjni/engine/AppEngine",
    "com/liulishuo/filedownloader/message/SmallMessageSnapshot",
    "com/bumptech/glide/l", "com/baidu/mapframework/open/aidl/a$a$a",
    "com/baidu/mapapi/search/route/MassTransitRouteLine$TransitStep",
    "com/baidu/location/b/h", "com/baidu/mapapi/search/core/VehicleInfo",
    "com/baidu/mapapi/map/MapView", "com/baidu/mapapi/search/busline/BusLineResult",
    "com/baidu/location/c/e", "com/baidu/platform/comjni/util/JNIMD5",
    "com/baidu/location/Poi", "com/baidu/platform/util/a",
    "com/baidu/location/LocationClientOption", "com/baidu/mapapi/search/share/a",
    "com/baidu/mapapi/search/poi/PoiNearbySearchOption",
    "com/alibaba/sdk/android/oss/internal/ResponseParser",
    "com/baidu/mapapi/search", "com/baidu/mapapi/map/BaiduMap$OnMarkerClickListener",
    "com/alibaba/sdk/android/oss/internal/InternalRequestOperation$4",
    "com/baidu/location", "com/baidu/mapapi/search/route/WalkingRouteResult",
    "anetwork/channel/entity/c", "com/lody/virtual/server/content/b",
    "com/baidu/mapapi/cloud/CloudSearchResult",
    "anet/channel/entity/a", "com/baidu/mapapi/CoordType",
    "com/baidu/mapapi/map/MarkerOptions$MarkerAnimateType",
    "com/alibaba/sdk/android/oss/common/OSSLogToFileUtils$1",
    "com/baidu/lbsapi/tools/CoordinateConverter$COOR_TYPE",
    "com/alibaba/sdk/android/oss/common/auth/OSSStsTokenCredentialProvider",
    "com/baidu/location/c/e$c",
    "com/alibaba/sdk/android/oss/model/GetBucketACLResult",
    "com/baidu/lbsapi/auth/LBSAuthManager",
    "com/baidu/location/c/j", "com/baidu/location/c/j$a",
    "com/alibaba/sdk/android/oss/model",
    "com/baidu/location/c/a/d$b",
    "com/baidu/lbsapi/auth/c$a",
    "com/alibaba/sdk/android/oss/internal/ResponseParsers$ListPartsResponseParser",
    "com/alibaba/sdk/android/oss/internal/CheckCRC64DownloadInputStream",
    "com/baidu/location/c/e$b",
    "com/baidu/lbsapi/auth/l",
    "com/baidu/location/f/d$c",
    "com/baidu/pano/platform",
    "com/alibaba/sdk/android/oss/OSSImpl",
    "com/baidu/mapapi/cloud/CloudSearchResult",
    "com/baidu/location/g/a$a",
    "com/baidu/location/e/k$a",
    "com/baidu/location/a/h",
    "com/baidu/mapapi/model/c", "com/lody/virtual/client/stub/ShadowContentProvider$P18",
    "com/baidu/lbsapi/model/BaiduPanoData",
    "com/baidu/mapapi/map/UiSettings",
    "com/baidu/location/a/j",
    "com/alibaba/sdk", "com/baidu/platform/comapi/map/ai",
    "com/baidu/location/c/a/d", "com/lody/virtual/client/stub/ShadowActivity$P78",
    "com/lody/virtual/client/stub/ShadowActivity",
    "com/baidu/location/f/f", "com/baidu/mapapi/map/Dot",
    "cn/smssdk/b/a$1", "com/baidu/mapapi/search/route/TransitRouteLine$TransitStep",
    "anet/channel/strategy/dispatch",
    "com/baidu/location",
    "com/alibaba/sdk/android/oss/internal",
    "com/baidu/location/a/q",
    "com/baidu/mapapi/map/BaiduMap",
    "com/baidu/mapapi",
    "com/baidu/platform",
    "com/alibaba/sdk/android/oss/model",
    "com/lody/virtual/client",
    "com/liulishuo/filedownloader/services",
    "cn/smssdk/contact",
    "com/lody/virtual/client/stub/ShadowContentProvider",
    "com/baidu/platform/comjni/map/basemap/a",
    "com/baidu/mapapi/cloud/CloudManager",
    "com/alibaba/sdk/android/oss/OSSImpl",
    "com/baidu/mapapi/map/MapBaseIndoorMapInfo$SwitchFloorError",
    "com/baidu/mapapi/map/InfoWindow",
    "com/liulishuo/okdownload/g",
    "com/alipay/sdk/auth/AuthActivity",
    "cn/smssdk/net/b$a",
    "com/baidu/mapapi/map/PolylineOptions",
    "com/alibaba/sdk/android/oss/common/auth/OSSAuthCredentialsProvider",
    "com/bumptech/glide/k", "com/baidu/mapapi/search/geocode/GeoCodeResult",
    "cn/smssdk/net/a", "com/mob/MobCommunicator",
    "com/alibaba/sdk/android/oss/model/CompleteMultipartUploadRequest",
    "com/bumptech/glide/g", "com/lody/virtual/client/stub/ShadowDialogActivity$P65",
    "com/baidu/android/bbalbs/common/util/b",
    "anet/channel/util/StringUtils", "com/baidu/mapapi/map/q",
    "com/bumptech/glide/d", "com/lody/virtual/helper/b",
    "com/baidu/mapapi/map", "com/alipay/sdk/app/H5AuthActivity",
    "anet/channel/Config", "com/baidu/platform/comapi/map/j",
    "com/bumptech/glide", "com/baidu/platform/domain",
    "com/baidu/mapapi/search/route", "com/lody/virtual/client/stub/ShadowService$P65",
    "com/baidu/mapapi/map/BaiduMapOptions", "com/lody/virtual/client/stub/ShadowService",
    "com/baidu/pano/platform/c/f", "com/alibaba/sdk/android/oss/model/AbortMultipartUploadResult",
    "com/lody/virtual/remote/vloc/VLocation", "com/baidu/platform/comjni/map/radar/a",
    "com/baidu/mapapi/search/route/IndoorRouteLine", "com/baidu/mapapi/radar/RadarNearbySearchOption",
    "com/lody/virtual/client/stub/ShadowActivity$P44", "com/liulishuo/filedownloader/services/h",
    "com/baidu/location", "com/alipay/sdk/auth/j",
    "com/liulishuo/filedownloader/message/LargeMessageSnapshot$PendingMessageSnapshot", "com/baidu/location/a/q",
    "com/alipay/sdk/app/c", "com/baidu/mapapi/map/WearMapView", "com/liulishuo/filedownloader/model/FileDownloadTaskAtom",
    "com/baidu/platform/comapi/map/e$a", "com/lody/virtual/client/stub/ShadowDialogActivity",
    "com/baidu/mapapi/search/core/TrainInfo", "com/lody/virtual/client/stub", "com/lody/virtual/remote/InstallOptions",
    "com/alibaba/sdk/android/httpdns/g", "com/baidu/lbsapi/auth/e", "com/baidu/mapframework/open/aidl/IComOpenClient",
    "anetwork/channel/aidl/DefaultProgressEvent", "com/baidu/mapapi/utils/poi/IllegalPoiSearchArgumentException",
    "com/baidu/platform/comjni/map/commonmemcache/a", "com/alipay/sdk/app/d$a", "com/baidu/pano/platform/a/a/a",
    "com/baidu/mapapi/navi/NaviParaOption",
    "com/baidu/mapapi/map/n", "com/alibaba/sdk/android/oss/network/ExecutionContext",
    "com/baidu/vi/b", "com/baidu/mapapi/search/busline/OnGetBusLineSearchResultListener",
    "com/baidu/location/e/c", "com/baidu/mapapi/search/core/TransitResultNode", "anetwork/channel/aidl/h$a$a",
    "anet/channel/util/g", "com/baidu/location/c/e$c", "cn/smssdk/utils/a", "anet/channel/Config$Builder",
    "com/baidu/location/e/d$b", "com/baidu/pano/platform/a/a/e$b", "com/baidu/location/BDLocationListener",
    "com/liulishuo/filedownloader/model/FileDownloadModel", "com/alibaba/sdk/android/oss/model",
    "com/baidu/mapapi/map/offline", "com/baidu/mapapi/navi/BaiduMapNavigation",
    "com/baidu/mapapi/map/m", "com/baidu/mapapi/utils/CoordinateConverter", "anet/channel/util/ALog",
    "anet/channel/strategy/dispatch", "com/alibaba/sdk/android/oss/internal/ResumableUploadTask",
    "com/baidu/mapapi/map/WearMapView", "com/baidu/mapapi/search/poi/PoiIndoorResult"];
 
function hook_java() {
    if (Java.available) {
        Java.perform(function () {
            // var JavaDexFile = Java.use("dalvik.system.DexFile");
            // JavaDexFile.loadClass.implementation = function (classname, loader) {
            //     var ret = this.loadClass(classname, loader);
            //     console.log("loadClass classname->" + classname);
            //     return ret;
            // }
            console.log("yes");
        });
    }
}
 
function register_class() {
    if (Java.available) {
        Java.perform(function () {
            // 2. 添加一个thread
            var JavaRunnable = Java.use("java.lang.Runnable");
            var TestRunnable = Java.registerClass({
                name: 'com.ingeek.TestRunnable',
                implements: [JavaRunnable],
                fields: {
                    methodObject: 'java.lang.reflect.Method',
                    methodInvoke: 'java.lang.Object',
                    methodArgs: '[Ljava.lang.Object;'
                },
                methods: {
                    $init: function () {
                        console.log("TestRunnable new args 0 ");
                    },
                    $init: function (methodObject, methodInvoke, methodArgs) {
                        this.methodObject = methodObject;
                        this.methodInvoke = methodInvoke;
                        this.methodArgs = methodArgs;
                        console.log("methodObject", this.methodObject.getName(), "methodInvoke", this.methodInvoke.getName(), "methodArgs", this.methodArgs.length)
                    },
                    run: function () {
                        invokeMethod();
                    },
                    invokeMethod: function () {
                        methodObject.invoke(methodInvoke, methodArgs);
                    }
                }
            });
        });
    }
}
 
// function hook_test() {
//     if (Java.available) {
//         Java.perform(function () {
//             // var JavaTestRunnable = Java.use("com.ingeek.TestRunnable");
//             var JavaClass = Java.use("java.lang.Class");
//             var JavaTestRunnable = JavaClass.forName("com.ingeek.TestRunnable");
//             var methods = JavaTestRunnable.getDeclaredMethods();
//             methods.forEach(function (method) {
//                 console.log("method " + method.getName());
//             });
//             JavaTestRunnable.$init.implementation = function (a1, a2) {
//                 return this.$init(a1, a2);
//             }
//         });
//     }
// }
 
 
function hook_call() {
    if (Java.available) {
        Java.perform(function () {
            var JavaDexFile = Java.use("dalvik.system.DexFile");
            var JavaH = Java.use("s.h.e.l.l.H");
            var JavaClass = Java.use("java.lang.Class");
            var JavaObject = Java.use("java.lang.Object");
            // for (var i = 0; i < 9999; i++) {
            //     JavaH.i(i);
            //     console.log("call function H.i(" + i + ")");
            // }
 
            // 1. 主动调用自己的脱壳机
            var JavaApkDumper = Java.use("android.app.ApkDumper");
            var JavaModifier = Java.use("java.lang.reflect.Modifier");
            var classLoaders = Java.enumerateClassLoadersSync();
            classLoaders.forEach(function (loader) {
                if (loader.toString().indexOf("assistant") > -1) {
                    console.log("loader info->" + loader);
                    JavaApkDumper.threadDumpDex(loader, "com.xgtl.assistant");
                }
            });
 
            // 2. 添加一个thread
            var JavaRunnable = Java.use("java.lang.Runnable");
            var JavaMethod = Java.use("java.lang.reflect.Method");
            var TestRunnable = Java.registerClass({
                name: "com.ingeek.TestRunnable",
                implements: [JavaRunnable],
                fields: {
                    methodObject: 'java.lang.reflect.Method',
                    methodInvoke: 'java.lang.Object',
                    methodArgs: '[Ljava.lang.Object;'
                },
                methods: {
                    $init: [{
                        returnType: "void",
                        argumentTypes: ["java.lang.reflect.Method", "java.lang.Object", "[Ljava.lang.Object;"],
                        implementation: function (methodObj, methodInv, methodAs) {
                            this.methodObject = methodObj;
                            this.methodInvoke = methodInv;
                            this.methodArgs = methodAs;
                            console.log("TestRunnable cons is called!");
                        }
                    }],
                    run: [{
                        returnType: 'void',
                        implementation: function () {
                            try {
                                Java.cast(this.methodObject, JavaMethod).invoke(this.methodInvoke, this.methodArgs);
                            } catch (e) {
                                console.log("TestRunnable run error!", e);
                            }
 
                            console.log("TestRunnable run is called!");
                        }
                    }]
                }
            });
 
            // console.log("TestRunnable", TestRunnable.$new(null, null, [null]).getClass().getDeclaredMethods(), TestRunnable.$new(null, null, [null]).getClass().getConstructors());
 
 
            // private void saveDexFilesToSDcard(Object cookie, String pkgname, ClassLoader mClassLoader)
            // JavaDexFile.saveDexFilesToSDcard.implementation = function (cookie, pkgname, loader) {
            //     console.log("test");
            //     var classnameArray = this.getClassNameList(cookie);
            //     classnameArray.forEach(function (classname) {
            //         try {
            //             this.loadClass(classname);
            //         } catch (e) {
            //             console.log("error classname->" + classname + "error info->" + e);
            //         }
            //     });
            //     this.saveDexFilesToSDcard(cookie, pkgname, loader);
            // }
 
            // private static native Class defineClassNative(String name, ClassLoader loader, Object cookie, DexFile dexFile)
            JavaDexFile.defineClassNative.implementation = function (name, loader, cookie, dexFile) {
                var ret = this.defineClassNative(name, loader, cookie, dexFile);
                // console.log("defineClassNative classname->" + name + ", ret is " + ret);
                if (name.indexOf("android") == 0) {
                    // console.log("class name", name);
                } else {
                    if (ret) {
                        console.log("class name", name);
                        var JavaClazzObject = Java.cast(ret, JavaClass);
                        var isAbstract = JavaModifier.isAbstract(JavaClazzObject.getModifiers());
                        var cons = JavaClazzObject.getConstructors();
                        var methods = JavaClazzObject.getDeclaredMethods();
                        var flag = false;
                        javaClassNameArray.forEach(function (classname) {
                            // console.log("filter class name", classname, "target class name", name);
                            if (name.indexOf(classname) == 0) {
                                flag = true;
                            }
                        });
                        console.log("flag info", flag, "isAbstract", isAbstract);
                        if ((!isAbstract) && (!flag)) {
                            for (var i = 0; i < cons.length; i++) {
                                var con = cons[i];
                                var conName = con.getName();
                                con.setAccessible(true);
                                console.log("constructor name", conName, "argsLen", con.getParameterTypes().length);
                                var argsLen = con.getParameterTypes().length;
                                if (argsLen == 0) {
                                    try {
                                        con.newInstance(null);
                                    } catch (e) {
                                        console.log("call cons error", e);
                                    }
                                }
                            }
                            for (var i = 0; i < methods.length; i++) {
                                var method = methods[i];
                                if (!JavaModifier.isNative(method.getModifiers())) {
                                    var methodName = method.getName();
                                    var argsLen = method.getParameterTypes().length;
                                    var exceptionTypeLen = method.getExceptionTypes().length;
                                    console.log("method name", methodName, "argsLen", argsLen, "exceptionTypeLen", exceptionTypeLen);
                                    method.getParameterTypes().forEach(function (type) {
                                        console.log("method parameter type", type.getName());
                                    });
 
                                    method.setAccessible(true);
                                    // 从frida这里fromReflectedMethod看一下是不是修复了。
                                    // Java.vm.tryGetEnv().fromReflectedMethod(method);
 
                                    // if (JavaModifier.isStatic(method.getModifiers())) {
                                    // if (argsLen == 0) {
                                    // try {
 
 
                                    try {
                                        var argsArray = Java.array("java.lang.Object", null);
                                        console.log("call static method " + method.getName(), "method", method.getName(), "null", argsArray);
                                        var testRunnable = TestRunnable.$new(Java.cast(method, JavaMethod), null, argsArray);
                                        Java.use("java.lang.Thread").$new(testRunnable).start();
                                    } catch (e) {
                                        console.log("invoke method error~");
                                    }
                                    // method.invoke(null, Java.array("java.lang.Object", new Array(argsLen)));
                                    // } catch (e) {
                                    //     console.log("call static method error", e);
                                    // }
                                    // }
 
 
                                    // } else {
                                    //     try {
                                    //         console.log("call no static method " + method.getName());
                                    //         method.invoke(null, Java.array("java.lang.Object", new Array(argsLen)));
                                    //     } catch (e) {
                                    //         console.log("call no static method error", e);
                                    //     }
                                    //
                                    // }
 
                                }
                            }
                        }
                    }
 
                }
 
                // Java.cast(ret, JavaClass).getConstructors().forEach(function (con) {
                //     console.log("constructor info", con.getName());
                //     try {
                //         var argsLen = con.getParameterTypes().length;
                //         var JavaObjectArray = Java.array("java.lang.Object", new Array(argsLen));
                //         con.newInstance(JavaObjectArray);
                //     } catch (e) {
                //         console.log("invoke constructor error", e);
                //     }
                // });
 
                // Java.cast(ret, JavaClass).getDeclaredMethods().forEach(function (method) {
                //     console.log("method info", method.getName());
                //     try{
                //         var argsLen = method.getParameterTypes().length;
                //         var JavaObjectArray = Java.array("java.lang.Object", new Array(argsLen));
                //         method.invoke(null, JavaObjectArray);
                //
                //     }catch (e) {
                //         console.log("invoke method error", e);
                //     }
                //
                // });
                return ret;
            }
        });
    }
}
 
function hook_native() {
    console.log("hook native");
    var artmethodInvoke = Module.findExportByName('libart.so', "_ZN3art9ArtMethod6InvokeEPNS_6ThreadEPjjPNS_6JValueEPKc");
    console.log("artmethodInvoke addr " + artmethodInvoke);
    Interceptor.attach(artmethodInvoke, {
        onEnter: function (args) {
            console.log("artmethod name", ptr(this.context.r0).GetName());
        },
        onLeave: function (retval) {
 
        }
    });
}
 
setTimeout(hook, 0);
```

寒冰大佬太强了， 提醒我在ArtMethod::Invoke中artmethod执行完之后，把opcode脱下来，可惜自己代码能力太弱了，一时半会也写不出来，慢慢来吧。