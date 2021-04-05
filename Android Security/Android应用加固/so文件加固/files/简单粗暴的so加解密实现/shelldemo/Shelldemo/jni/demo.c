#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <elf.h>
#include <sys/mman.h>

jstring getString(JNIEnv*) __attribute__((section (".mytext")));
jstring getString(JNIEnv* env){
	return (*env)->NewStringUTF(env, "Native method return!");
};

void init_getString() __attribute__((constructor));
unsigned long getLibAddr();

void init_getString(){
  char name[15];
  unsigned int nblock;
  unsigned int nsize;
  unsigned long base;
  unsigned long text_addr;
  unsigned int i;
  Elf32_Ehdr *ehdr;
  Elf32_Shdr *shdr;
  
  base = getLibAddr();
  
  ehdr = (Elf32_Ehdr *)base;
  text_addr = ehdr->e_shoff + base;
  
  nblock = ehdr->e_entry >> 16;
  nsize = ehdr->e_entry & 0xffff;
  
  printf("nblock = %d\n", nblock);
  
  if(mprotect((void *) base, 4096 * nsize, PROT_READ | PROT_EXEC | PROT_WRITE) != 0){
    puts("mem privilege change failed");
  }
  
  for(i=0;i< nblock; i++){  
    char *addr = (char*)(text_addr + i);
    *addr = ~(*addr);
  }
  
  if(mprotect((void *) base, 4096 * nsize, PROT_READ | PROT_EXEC) != 0){
    puts("mem privilege change failed");
  }
  puts("Decrypt success");
}

unsigned long getLibAddr(){
  unsigned long ret = 0;
  char name[] = "libdemo.so";
  char buf[4096], *temp;
  int pid;
  FILE *fp;
  pid = getpid();
  sprintf(buf, "/proc/%d/maps", pid);
  fp = fopen(buf, "r");
  if(fp == NULL)
  {
    puts("open failed");
    goto _error;
  }
  while(fgets(buf, sizeof(buf), fp)){
    if(strstr(buf, name)){
      temp = strtok(buf, "-");
      ret = strtoul(temp, NULL, 16);
      break;
    }
  }
_error:
  fclose(fp);
  return ret;
}

JNIEXPORT jstring JNICALL
Java_com_example_shelldemo_MainActivity_getString( JNIEnv* env,
                                                  jobject thiz )
{
#if defined(__arm__)
  #if defined(__ARM_ARCH_7A__)
    #if defined(__ARM_NEON__)
      #define ABI "armeabi-v7a/NEON"
    #else
      #define ABI "armeabi-v7a"
    #endif
  #else
   #define ABI "armeabi"
  #endif
#elif defined(__i386__)
   #define ABI "x86"
#elif defined(__mips__)
   #define ABI "mips"
#else
   #define ABI "unknown"
#endif

    return getString(env);
}


