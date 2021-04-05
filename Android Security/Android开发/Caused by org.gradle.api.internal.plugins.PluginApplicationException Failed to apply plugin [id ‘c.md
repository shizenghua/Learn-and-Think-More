# Caused by: org.gradle.api.internal.plugins.PluginApplicationException: Failed to apply plugin [id ‘c

Caused by: org.gradle.api.internal.plugins.PluginApplicationException: Failed to apply plugin [id ‘com.android.internal.application’]
在 Gradle.Scripts中的gradle.properties
添加

```cpp
android.overridePathCheck=true
```

就行了