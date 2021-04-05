#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <elf.h>
#include <sys/mman.h>

#define DEBUG

typedef struct _funcInfo{
  Elf32_Addr st_value;
  Elf32_Word st_size;
}funcInfo;


void init_getString() __attribute__((constructor));

static void print_debug(const char *msg){
#ifdef DEBUG
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "%s", msg);
#endif
}

static unsigned elfhash(const char *_name)
{
    const unsigned char *name = (const unsigned char *) _name;
    unsigned h = 0, g;

    while(*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

static unsigned int getLibAddr(){
  unsigned int ret = 0;
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

static char getTargetFuncInfo(unsigned long base, const char *funcName, funcInfo *info){
	char flag = -1, *dynstr;
	int i;
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;
	Elf32_Off dyn_vaddr;
	Elf32_Word dyn_size, dyn_strsz;
	Elf32_Dyn *dyn;
    Elf32_Addr dyn_symtab, dyn_strtab, dyn_hash;
	Elf32_Sym *funSym;
	unsigned funHash, nbucket;
	unsigned *bucket, *chain;

    ehdr = (Elf32_Ehdr *)base;
	phdr = (Elf32_Phdr *)(base + ehdr->e_phoff);
//    __android_log_print(ANDROID_LOG_INFO, "JNITag", "phdr =  0x%p, size = 0x%x\n", phdr, ehdr->e_phnum);
	for (i = 0; i < ehdr->e_phnum; ++i) {
//		__android_log_print(ANDROID_LOG_INFO, "JNITag", "phdr =  0x%p\n", phdr);
		if(phdr->p_type ==  PT_DYNAMIC){
			flag = 0;
			print_debug("Find .dynamic segment");
			break;
		}
		phdr ++;
	}
	if(flag)
		goto _error;
	dyn_vaddr = phdr->p_vaddr + base;
	dyn_size = phdr->p_filesz;
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "dyn_vadd =  0x%x, dyn_size =  0x%x", dyn_vaddr, dyn_size);
	flag = 0;
	for (i = 0; i < dyn_size / sizeof(Elf32_Dyn); ++i) {
		dyn = (Elf32_Dyn *)(dyn_vaddr + i * sizeof(Elf32_Dyn));
		if(dyn->d_tag == DT_SYMTAB){
			dyn_symtab = (dyn->d_un).d_ptr;
			flag += 1;
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "Find .dynsym section, addr = 0x%x\n", dyn_symtab);
		}
		if(dyn->d_tag == DT_HASH){
			dyn_hash = (dyn->d_un).d_ptr;
			flag += 2;
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "Find .hash section, addr = 0x%x\n", dyn_hash);
		}
		if(dyn->d_tag == DT_STRTAB){
			dyn_strtab = (dyn->d_un).d_ptr;
			flag += 4;
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "Find .dynstr section, addr = 0x%x\n", dyn_strtab);
		}
		if(dyn->d_tag == DT_STRSZ){
			dyn_strsz = (dyn->d_un).d_val;
			flag += 8;
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "Find strsz size = 0x%x\n", dyn_strsz);
		}
	}
	if((flag & 0x0f) != 0x0f){
		print_debug("Find needed .section failed\n");
		goto _error;
	}
	dyn_symtab += base;
	dyn_hash += base;
	dyn_strtab += base;
	dyn_strsz += base;

	funHash = elfhash(funcName);
	funSym = (Elf32_Sym *) dyn_symtab;
	dynstr = (char*) dyn_strtab;
	nbucket = *((int *) dyn_hash);
	bucket = (int *)(dyn_hash + 8);
	chain = (unsigned int *)(dyn_hash + 4 * (2 + nbucket));

	flag = -1;
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "hash = 0x%x, nbucket = 0x%x\n", funHash, nbucket);
	for(i = bucket[funHash % nbucket]; i != 0; i = chain[i]){
		__android_log_print(ANDROID_LOG_INFO, "JNITag", "Find index = %d\n", i);
		if(strcmp(dynstr + (funSym + i)->st_name, funcName) == 0){
			flag = 0;
			__android_log_print(ANDROID_LOG_INFO, "JNITag", "Find %s\n", funcName);
			break;
		}
	}
	if(flag) goto _error;
	info->st_value = (funSym + i)->st_value;
	info->st_size = (funSym + i)->st_size;
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "st_value = %d, st_size = %d", info->st_value, info->st_size);
	return 0;
_error:
	return -1;
}


void init_getString(){
	const char target_fun[] = "Java_com_example_shelldemo2_MainActivity_getString";
	funcInfo info;
	int i;
	unsigned int npage, base = getLibAddr();

	__android_log_print(ANDROID_LOG_INFO, "JNITag", "base addr =  0x%x", base);
	if(getTargetFuncInfo(base, target_fun, &info) == -1){
	  print_debug("Find Java_com_example_shelldemo2_MainActivity_getString failed");
	  return ;
	}
	npage = info.st_size / PAGE_SIZE + ((info.st_size % PAGE_SIZE == 0) ? 0 : 1);
	if(mprotect((void *) ((base + info.st_value) / PAGE_SIZE * PAGE_SIZE), npage, PROT_READ | PROT_EXEC | PROT_WRITE) != 0){
		print_debug("mem privilege change failed");
	}

	for(i=0;i< info.st_size - 1; i++){
		char *addr = (char*)(base + info.st_value -1 + i);
		*addr = ~(*addr);
	}

	if(mprotect((void *) ((base + info.st_value) / PAGE_SIZE * PAGE_SIZE), npage, PROT_READ | PROT_EXEC) != 0){
		print_debug("mem privilege change failed");
	}

}

JNIEXPORT jstring JNICALL
Java_com_example_shelldemo2_MainActivity_getString( JNIEnv* env,
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
	return (*env)->NewStringUTF(env, "Native method return!");
}


