# ArtMethod::Invoke

```
//ArtMethod::Invoke开始时dump
void ArtMethod::Invoke(Thread* self, uint32_t* args, uint32_t args_size, JValue* result,
                       const char* shorty) {
  const char* methodName = this->PrettyMethod().c_str();
  if(strstr(methodName, "SplashActivity")){
      dumpArtMethod(this);
  }
  if (UNLIKELY(__builtin_frame_address(0) < self->GetStackEnd())) {
    ThrowStackOverflowError(self);
    return;
  }
  


//ArtMethod::Invoke结束时时dump
void ArtMethod::Invoke(Thread* self, uint32_t* args, uint32_t args_size, JValue* result,
                       const char* shorty) {
 
  if (UNLIKELY(__builtin_frame_address(0) < self->GetStackEnd())) {
    ThrowStackOverflowError(self);
    return;
  }
  ...
 
  self->PopManagedStackFragment(fragment);
 
  const char* methodName = this->PrettyMethod().c_str();
  if(strstr(methodName, "SplashActivity")){
      dumpArtMethod(this);
  }
```







# art_ClassLinker_LoadMethod

