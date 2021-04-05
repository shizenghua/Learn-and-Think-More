# android热修复

url：https://blog.csdn.net/jkkjklmn/article/details/52336713



 APP上线之后，出现了严重bug怎么办？热修复是一个很好的解决方式。

  怎么去做热修复参考的系列课程1讲的很清楚。需要注意到的是，参考的系列课程1的工程中本来就有两个dex文件，出现问题之后加上一个修复的dex文件总共有三个dex文件，所以他能正常修复。

  一般情况下，我们的工程当中就只有一个dex文件，如果完全按照系列课程1去做的话就会有问题，就是CLASS_IS_PREVERIFIED标志的问题。所以在这里的话我们要按照参考2中的去做：去引用别的dex中的一个类（就如参考2中提到的hack.dex中的Hack类，哪怕这个类啥都不做，这么做的目的就是为了防止MainActivity被打上CLASS_IS_PREVERIFIED标志）
  我们的工程，哪怕是初步发布的APP也要有两个dex文件，而且在调用后来发现有问题的类文件的Java文件中，要去调用第2个dex文件，这样的话，我们的主调用类就不会被打上CLASS_IS_PREVERIFIED标志。这样才可以正常的打补丁。



下面是打补丁的工具类，添加了一些注释便于理解。



```java
/**
 * 动态加载补丁包
 * @author alex_mahao
 *
 */
public class FixDexUtils {
 
    private static HashSet<File> loadedDex = new HashSet<File>();
 
    static {
        loadedDex.clear();
    }
 
    //把从服务器下载下来的已经修复好的没有bug的文件全部添加<span style="font-family: Arial, Helvetica, sans-serif;">loadedDex</span>中
    public static void loadFixDex(Context context) {
        // 获取到系统的odex 目录
        File fileDir = context.getDir("odex", Context.MODE_PRIVATE);
        File[] listFiles = fileDir.listFiles();
 
        for (File file : listFiles) {
            if (file.getName().endsWith(".dex")) {
                // 存储该目录下的.dex文件（补丁）
                loadedDex.add(file);
            }
        }
 
        doDexInject(context, fileDir);
 
    }
    //
    private static void doDexInject(Context context, File fileDir) {
        // .dex 的加载需要一个临时目录
        String optimizeDir = fileDir.getAbsolutePath() + File.separator + "opt_dex";
        File fopt = new File(optimizeDir);
        if (!fopt.exists())
            fopt.mkdirs();
        // 根据.dex 文件创建对应的DexClassLoader 类
        for (File file : loadedDex) { <span style="font-family: Arial, Helvetica, sans-serif;">//修复好的dex文件可能有多个，根据每个修复好的dex文件生成一个</span><span style="font-family: Arial, Helvetica, sans-serif;">classLoader </span>
            DexClassLoader classLoader = new DexClassLoader(file.getAbsolutePath(), fopt.getAbsolutePath(), null,
                    context.getClassLoader());
 
            //注入：逐个的把修复好的dex文件注入到系统dex（也就是PathClassLoader）前面
            inject(classLoader, context);
 
        }
    }
 
    private static void inject(DexClassLoader classLoader, Context context) {
 
        // 获取到系统的DexClassLoader 类：也就是包含MainActivity以及Application的主dex文件
        PathClassLoader pathLoader = (PathClassLoader) context.getClassLoader();
        try {
            // 分别获取到补丁的dexElements和系统的dexElements，并且合并他们，注意：补丁dex放在系统dex的前面
            Object dexElements = combineArray(getDexElements(getPathList(classLoader)),
                    getDexElements(getPathList(pathLoader)));
            // 获取到系统的pathList 对象
            Object pathList = getPathList(pathLoader);
            // 设置系统的dexElements 的值（反射方式）
            setField(pathList, pathList.getClass(), "dexElements", dexElements);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
 
    /**
     * 通过反射设置字段值
     */
    private static void setField(Object obj, Class<?> cl, String field, Object value)
            throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
 
        Field localField = cl.getDeclaredField(field);
        localField.setAccessible(true);
        localField.set(obj, value);
    }
 
    /**
     * 通过反射获取 BaseDexClassLoader中的PathList对象
     */
    private static Object getPathList(Object baseDexClassLoader)
            throws IllegalArgumentException, NoSuchFieldException, IllegalAccessException, ClassNotFoundException {
        return getField(baseDexClassLoader, Class.forName("dalvik.system.BaseDexClassLoader"), "pathList");
    }
 
    /**
     * 通过反射获取指定字段的值
     */
    private static Object getField(Object obj, Class<?> cl, String field)
            throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
        Field localField = cl.getDeclaredField(field);
        localField.setAccessible(true);
        return localField.get(obj);
    }
 
    /**
     * 通过反射获取DexPathList中dexElements
     */
    private static Object getDexElements(Object paramObject)
            throws IllegalArgumentException, NoSuchFieldException, IllegalAccessException {
        return getField(paramObject, paramObject.getClass(), "dexElements");
    }
 
    /**
     * 合并两个数组
     * @param arrayLhs
     * @param arrayRhs
     * @return
     */
    private static Object combineArray(Object arrayLhs, Object arrayRhs) {
        Class<?> localClass = arrayLhs.getClass().getComponentType();
        int i = Array.getLength(arrayLhs);
        int j = i + Array.getLength(arrayRhs);
        Object result = Array.newInstance(localClass, j);
        for (int k = 0; k < j; ++k) {
            if (k < i) {
                Array.set(result, k, Array.get(arrayLhs, k));
            } else {
                Array.set(result, k, Array.get(arrayRhs, k - i));
            }
        }
        return result;
    }
}
```



参考1：http://blog.csdn.net/lisdye2/article/details/52119602 系列课程。



参考2：http://www.tuicool.com/articles/ZNZ7rmi
http://blog.csdn.net/lmj623565791/article/details/49883661

