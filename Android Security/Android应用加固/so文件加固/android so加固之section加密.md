# android so加固之section加密

url：https://xz.aliyun.com/t/5328

# 引言

如何对so文件中的核心代码进行保护？

通过将核心代码写到自定义节中，并且对该节使用加密工具进行加密，在so文件执行时，利用***\*attribute\**((constructor));**属性，先于main执行解密函数，作用类似于java中的构造函数

# 实现流程

1. 确定好自定义节的名称
2. 开始加密流程
   - 遍历所有节头，根据节头名来定位需要加密的节
   - 获取节头中节的起始位置和大小，对节头指向的数据进行加密
3. 编写解密代码
   - 用属性： **attribute**((constructor));声明解密函数
   - 在native层编写解密函数

# 代码实现

## 加密流程

```
#include <stdio.h>
#include <elf.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>



int main(int argc, char** argv)
{
    int fd;
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;
    char * section_name_table;
    int i;
    unsigned int base, length;
    char *content;


    //参数验证
    if(argc != 3)
    {
        printf("Encrypt section of elf file\n\nUsage:\n\t%s <elf_file> <section_name>\n", *argv);
        goto _error;
    }

    if((fd = open(argv[1], O_RDWR, 0777)) == -1)
    {
        perror("open");
        goto _error;
    }

    if(read(fd, &ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        perror("read elf header");
        goto _error;
    }

    //读取节头字符串表
    printf("[+] Begining find section %s\n", argv[2]);
    lseek(fd, ehdr.e_shoff+sizeof(Elf32_Shdr)*ehdr.e_shstrndx, SEEK_SET);
    if(read(fd, &shdr, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr))
    {
        perror("read elf section header which contain string table");
        goto _error;
    }

    if((section_name_table = (char*) malloc(shdr.sh_size)) == NULL)
    {
        perror("malloc for SHT_STRTAB");
        goto _error;
    }

    lseek(fd, shdr.sh_offset, SEEK_SET);
    if(read(fd, section_name_table, shdr.sh_size) != shdr.sh_size)
    {
        perror("read string table");
        goto _error;
    }
    lseek(fd, ehdr.e_shoff, SEEK_SET);
    //根据节头名来定位需要加密的节头
    for(i=0; i<ehdr.e_shnum; i++)
    {
        if(read(fd, &shdr, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr))
        {
            perror("read section");
            goto _error;
        }
        if(strcmp(section_name_table+shdr.sh_name, argv[2]) == 0)
        {
            base = shdr.sh_offset;
            length = shdr.sh_size;
            printf("[+] Find section %s\n", argv[2]);
            printf("[+] %s section offset is %X\n", argv[2], base);
            printf("[+] %s section size is %d\n", argv[2], length);
            break;
        }
    }

    //开始加密逻辑
    lseek(fd, base, SEEK_SET);
    content = (char *)malloc(length);
    if(content == NULL)
    {
        perror("malloc space for section");
        goto _error;
    }
    if(read(fd, content, length) != length)
    {
        perror("read section in encrpt");
        goto _error;
    }
    //取反加密
    for(i=0; i<length; i++)
    {
        content[i] = ~content[i];
    }

    lseek(fd, 0, SEEK_SET);
    if(write(fd, &ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        perror("write ELF header to file");
        goto _error;
    }
    lseek(fd, base, SEEK_SET);
    if(write(fd, content, length) != length)
    {
        perror("write encrypted section to file");
        goto _error;
    }

    printf("[+] Encrypt section %s completed!\n", argv[2]);
_error:
    free(section_name_table);
    free(content);
    close(fd);
    return 0;
}
```

加密前后的对比图：

![img](https://xzfile.aliyuncs.com/media/upload/picture/20190531205600-70936b2e-83a3-1.png)

上面的加密代码只对节数据进行加密，下面我们增加几行代码，把被加密节的长度、用到的内存页数替换到文件头中的入口点和节头表偏移中去，进一步防止反汇编并且简化后面解密流程。

那么有人会问，入口点都被填充了文件怎么执行？这里我们需要知道，对于动态链接库，e_entry 入口地址是无意义的，因为程序被加载时，设定的跳转地址是动态连接器的地址，这个字段是可以被作为数据填充的

```
#include <stdio.h>
#include <elf.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>



int main(int argc, char** argv)
{
    int fd;
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;
    char * section_name_table;
    int i;
    unsigned int base, length;
    char *content;
    unsigned short nsize;


    //参数验证
    if(argc != 3)
    {
        printf("Encrypt section of elf file\n\nUsage:\n\t%s <elf_file> <section_name>\n", *argv);
        goto _error;
    }

    if((fd = open(argv[1], O_RDWR, 0777)) == -1)
    {
        perror("open");
        goto _error;
    }

    if(read(fd, &ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        perror("read elf header");
        goto _error;
    }

    //读取节头字符串表
    printf("[+] Begining find section %s\n", argv[2]);
    lseek(fd, ehdr.e_shoff+sizeof(Elf32_Shdr)*ehdr.e_shstrndx, SEEK_SET);
    if(read(fd, &shdr, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr))
    {
        perror("read elf section header which contain string table");
        goto _error;
    }

    if((section_name_table = (char*) malloc(shdr.sh_size)) == NULL)
    {
        perror("malloc for SHT_STRTAB");
        goto _error;
    }

    lseek(fd, shdr.sh_offset, SEEK_SET);
    if(read(fd, section_name_table, shdr.sh_size) != shdr.sh_size)
    {
        perror("read string table");
        goto _error;
    }
    lseek(fd, ehdr.e_shoff, SEEK_SET);
    //根据节头名来定位需要加密的节头
    for(i=0; i<ehdr.e_shnum; i++)
    {
        if(read(fd, &shdr, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr))
        {
            perror("read section");
            goto _error;
        }
        if(strcmp(section_name_table+shdr.sh_name, argv[2]) == 0)
        {
            base = shdr.sh_offset;
            length = shdr.sh_size;
            printf("[+] Find section %s\n", argv[2]);
            printf("[+] %s section offset is %X\n", argv[2], base);
            printf("[+] %s section size is %d\n", argv[2], length);
            break;
        }
    }

    //开始加密逻辑
    lseek(fd, base, SEEK_SET);
    content = (char *)malloc(length);
    if(content == NULL)
    {
        perror("malloc space for section");
        goto _error;
    }
    if(read(fd, content, length) != length)
    {
        perror("read section in encrpt");
        goto _error;
    }
    //将该节具体长度值替换到入口点去
    //将该节的偏移地址替换到文件头中的节头表偏移值中去
    nsize = length/4096 + (length%4096 == 0 ? 0 : 1);
    ehdr.e_entry = (length << 16) + nsize;
    ehdr.e_shoff = base;
    printf("[+] %s section use %d memory page!\n", argv[2], nsize);

    //取反加密
    for(i=0; i<length; i++)
    {
        content[i] = ~content[i];
    }

    lseek(fd, 0, SEEK_SET);
    if(write(fd, &ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        perror("write ELF header to file");
        goto _error;
    }
    lseek(fd, base, SEEK_SET);
    if(write(fd, content, length) != length)
    {
        perror("write encrypted section to file");
        goto _error;
    }

    printf("[+] Encrypt section %s completed!\n", argv[2]);
_error:
    free(section_name_table);
    free(content);
    close(fd);
    return 0;
}
```

利用上面代码加密后32位ELF文件用IDA打开就会出现以下错误，不能进入程序正确入口并且不能从节头、函数符号也收到影响

![img](https://xzfile.aliyuncs.com/media/upload/picture/20190531205645-8bbb084e-83a3-1.png)

![img](https://xzfile.aliyuncs.com/media/upload/picture/20190531205747-b08aedc4-83a3-1.png)

节头完全识别不出来只能用段表来显示

![img](https://xzfile.aliyuncs.com/media/upload/picture/20190531205720-a087a958-83a3-1.png)

## Native解密

**解密原理**：我们从加密后so文件头的入口点右移16的值中获取加密的自定义节的长度，从文件头中的节头表偏移值中获取加密的节的内存偏移。然后我们用mprotect把这个节修改成可写属性接着逐个字符的解密，解密完成后修改会原始权限

```
#include <jni.h>
#include <string>
#include <asm/fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sstream>
#include <fcntl.h>
#include <android/log.h>
#include <elf.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

jstring getString(JNIEnv*) __attribute__((section (".mytext")));
//声明为构造函数，在init_array节执行
void decryte_section() __attribute__((constructor));
unsigned long getLibAddr();

void decryte_section() {
    unsigned long base;
    Elf32_Ehdr *ehdr;
    Elf32_Shdr *shdr;
    unsigned long my_text_addr;
    unsigned int nblock;
    unsigned int nsize;
    unsigned int i;


    base = getLibAddr();
    ehdr = (Elf32_Ehdr *)base;
    //自定义节的位置
    my_text_addr = base + ehdr->e_shoff;
    nblock = ehdr->e_entry >> 16;
    nsize = (nblock / PAGE_SIZE) + (nblock%PAGE_SIZE == 0 ? 0 : 1);
    __android_log_print(ANDROID_LOG_INFO, "JNITag", "size of encrypted section is %d", nblock);
    if (mprotect((void *)(my_text_addr / PAGE_SIZE * PAGE_SIZE), nsize*PAGE_SIZE, PROT_READ | PROT_EXEC | PROT_WRITE) == -1){
        __android_log_print(ANDROID_LOG_ERROR, "JNITag", "Memory privilege change failed before encrypt");
    }
    //解密
    for(i=0; i<nblock; i++){ 
        char * addr = (char *)(my_text_addr + i);
        * addr = ~(*addr);
    }
    //改回该节的权限
    if (mprotect((void *)(my_text_addr / PAGE_SIZE * PAGE_SIZE), nsize*PAGE_SIZE, PROT_READ | PROT_EXEC) == -1){
        __android_log_print(ANDROID_LOG_ERROR, "JNITag", "Memory privilege change failed after encrypt");
    }
    __android_log_print(ANDROID_LOG_INFO, "JNITag", "Decrypt completed!");
}

/** 获取当前进程内部指定共享库文件的内存映射地址*/
unsigned long getLibAddr(){

    int pid;
    char buffer[4096];
    FILE *fd;
    char *tmp;

    unsigned long ret = 0;
    char so_name[] = "libnative-lib.so";

    pid = getpid();
    sprintf(buffer, "/proc/%d/maps", pid);
    if((fd = fopen(buffer, "r")) == NULL){
        __android_log_print(ANDROID_LOG_DEBUG, "JNITag", "open /proc/%d/maps failed!", pid);
        goto _error;
    }
    __android_log_print(ANDROID_LOG_INFO, "JNITag", "cat /proc/%d/maps succussful!", pid);
    while(fgets(buffer, sizeof(buffer), fd)){
        if(strstr(buffer, so_name)){
            tmp = strtok(buffer, "-");
            ret = strtoul(tmp, 0, 16);
            __android_log_print(ANDROID_LOG_INFO, "JNITag", "Find file %s", so_name);
            __android_log_print(ANDROID_LOG_INFO, "JNITag", "The memory address of %s is 0x%X", so_name, ret);
            break;
        }
    }

_error:
    fclose(fd);
    return ret;
}

//后面是java层调用的Native函数
extern "C"
JNIEXPORT jstring JNICALL
Java_com_testjni_MainActivity_isTraceMe(JNIEnv *env, jobject instance){
    return getString(env);
}

//需要加密的核心代码主要存放在这个自定义节中
jstring getString(JNIEnv* env){
    return env->NewStringUTF("Text from JNI");
};
```

从下面可以运行结果显示，实现了自定义节的动态解密过程

![img](https://xzfile.aliyuncs.com/media/upload/picture/20190531205659-93d44ae0-83a3-1.png)

# 小结

本篇文章主要写了如何对section的加密、以及在`.init_array`节中进行动态解密的详细过程。

想要绕过也是可以的，通过动态调试在解密的.init_array节处下断点，然后dump出解密后的so文件进行反编译即可

# 参考

> [0] [Android so库加固加壳方案](http://szuwest.github.io/android-soku-jia-gu-jia-ke-fang-an.html)
>
> [1] [Android逆向之旅---基于对so中的section加密技术实现so加固](https://blog.csdn.net/jiangwei0910410003/article/details/49962173)
>
> [3] [[原创\]简单粗暴的so加解密实现](https://bbs.pediy.com/thread-191649.htm)