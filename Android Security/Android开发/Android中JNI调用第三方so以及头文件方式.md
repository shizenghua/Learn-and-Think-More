# Android中JNI调用第三方so以及头文件方式

2019-05-19 

- [Android开发](http://www.sorgs.cn/categories/Android开发/)

# 引言

有时候我们在android开发JNI的时候，会涉及到引用第三方的so和头文件引用。现在网上也有相应的资料，但是还是感觉不全和描述不清晰。这里进行整理一些，方便大家参考。

# 准备工作

- NDK，进行JNI开发，Android studio中的NDK肯定是需要配好的。需要注意一点的是，如果上比较新的NDK版本的话，在toolchains目录会少几种，需要去下载比较旧的版本把缺失的放进去。原因大概是Google已经放弃哪几种了。这个主要是针对比较老的工程会遇得到，也会有报错信息，搜一下很容易就知道了，就不展开说了。

- cMake和cpp。一般来说进行了JNI开发了，这些应该是有了，不再细说。只说下目录，cpp可以建一个cpp文件夹放在main文件夹下面，cMake需要放在app目录下面。详情目录结构可以参考Demo。

- build.gradle

  - 首先是在defaultConfig闭包类添加如下内容。我这边是生成了armeabi-v7a的格式，如需要其他格式，自行添加即可。

    ```
    externalNativeBuild {
        cmake {
            cppFlags ""
            abiFilters 'armeabi-v7a'
        }
    }
    ```

  - 在android闭包下面，即最大的闭包下面添加

    ```
    externalNativeBuild {
        cmake {
            path "CMakeLists.txt"
            version "3.10.2"
        }
    }
    
    sourceSets {
        main {
            // let gradle pack the shared library into apk
            jniLibs.srcDirs = ['src/main/jniLibs']
        }
    }
    ```

# 文件放置放置

- so文件：在main目录下面建立jniLibs文件夹。然后在下面在建立armeabi-v7a文件夹，把相应的so文件放到里面。需要注意的是，在自己需要生成什么类型的so，就需要建立什么类的文件夹，然后拷入相应类型第三方so文件。
- 头文件：在cpp目录下面建立include文件夹，放入第三方头文件即可。

# cMake编写

```
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp)

#动态方式加载 STATIC：表示静态的.a的库 SHARED：表示.so的库。
add_library(gmpfprojectorfocusmanager_hidl SHARED IMPORTED)
add_library(utils SHARED IMPORTED)
add_library(hidlbase SHARED IMPORTED)
add_library(hwbinder SHARED IMPORTED)
add_library(hidltransport SHARED IMPORTED)
add_library(hidlmemory SHARED IMPORTED)
#设置要连接的so的相对路径 ${CMAKE_SOURCE_DIR}：表示CMake.txt的当前文件夹路径 ${ANDROID_ABI}：编译时会自动根据CPU架构去选择相应的库
set_target_properties(gmpfprojectorfocusmanager_hidl PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libgmpfprojectorfocusmanager_hidl.so)
set_target_properties(utils PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libutils.so)
set_target_properties(hidlbase PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libhidlbase.so)
set_target_properties(hwbinder PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libhwbinder.so)
set_target_properties(hidltransport PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libhidltransport.so)
set_target_properties(hidlmemory PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libhidlmemory.so)
#添加第三方头文件
target_include_directories(native-lib PRIVATE ${CMAKE_SOURCE_DIR}/src/main/cpp/include)
# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
         gmpfprojectorfocusmanager_hidl
         utils
         hidlbase
         hwbinder
         hidltransport
         hidlmemory
        # included in the NDK.
        ${log-lib})
```

- add_library:这里主要是依赖第三方的so方式，每个so都要写一句。第一个参数是so的文件。例如libutils.so，则需要填写utils；第二个参数为STATIC：表示静态的.a的库或者SHARED：表示.so的库；第三个参数固定IMPORTED
- set_target_properties：链接so的路径。第一个参数依然是so的名字；第二个参数填写PROPERTIES即可；第三个填写IMPORTED_LOCATION即可；第四个则需要填写so的路径，需要注意的是会根据自己的需要生成so的类型去查找相应类型的so。
- target_include_directories：添加第三方头文件。第一个参数填写native-lib；第二个参数PRIVATE；第三个参数即头文件的文件夹路径。
- target_link_libraries：最后需要在这里把第三方so名字加入即可。

# 引用

- 在自己的cpp里面就只直接通过include引用第三的文件夹了，以及调用第三方的so文件

# 结语

1. 建议按照这样的路径来放置，防止出现问题。
2. demo已经放到了github上面，可以进行参考配置。https://github.com/sorgs/NDKTest