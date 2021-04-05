## 基于对so中的函数加密技术实现so加固

url：https://blog.csdn.net/u012108436/article/details/51258646



# 一、前言

今天我们继续来介绍so加固方式，在前面一篇文章中我们介绍了对so中指定的段(section)进行加密来实现对so加固

http://blog.csdn.net/jiangwei0910410003/article/details/49962173

这篇文章我们延续之前的这篇文章来介绍一下如何对函数进行加密来实现加固，当然这篇文章和前篇文章有很多类似的地方，这里就不做太多的解释了，所以还请阅读这篇文章之前先去了解前一篇文章。

#  二、技术原理

**这篇和之前的那篇文章唯一的不同点就是如何找到指定的函数的偏移地址和大小**

那么我们先来了解一下so中函数的表现形式：

在so文件中，每个函数的结构描述是存放在.dynsym段中的。每个函数的名称保存在.dynstr段中的，类似于之前说过的每个section的名称都保存在.shstrtab段中，所以在前面的文章中我们找到指定段的时候，就是通过每个段的sh_name字段到.shstrtab中寻找名字即可，而且我们知道.shstrtab这个段在头文件中是有一个index的，就是在所有段列表中的索引值，所以很好定位.shstrtab.

但是在这篇文章我们可能遇到一个问题，就是不能按照这种方式去查找指定函数名了：

![img](https://img-blog.csdn.net/20151121190647669?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

可能有的人意识到一个方法，就是我们可以通过section的type来获取.dynsym和.dynstr。我们看到上图中.dynsym类型是：DYNSYM,

.dynstr类型是STRTAB,但是这种方法是不行的，因为这个type不是唯一的，也就说不同的section，type可能相同，我们没办法区分，比如.shstrtab和.dynstr的type都是STRTAB.其实从这里我们就知道这两个段的区别了：

.shstrtab值存储段的名称，.dynstr是存储so中的所有符号名称。

那么我们该怎么办呢？这时候我们再去看一下elf的说明文档：

http://download.csdn.net/detail/jiangwei0910410003/9204051

我们看到有一个.hash段，在上图中我们也可以看到的：

由 Elf32_Word 对象组成的哈希表支持符号表访问。下面的例子有助于解释哈希表

![img](https://img-blog.csdn.net/20151121191243867?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)
组织,不过不是规范的一部分。bucket 数组包含 nbucket 个项目,chain 数组包含 nchain 个项目,下标都是从 0 开始。bucket 和 chain 中都保存符号表索引。Chain 表项和符号表存在对应。符号 表项的数目应该和 nchain 相等,所以符号表的索引也可用来选取 chain 表项。哈希 函数能够接受符号名并且返回一个可以用来计算 bucket 的索引。
因此,如果哈希函数针对某个名字返回了数值 X,则 bucket[X%nbucket] 给出了 一个索引 y,该索引可用于符号表,也可用于 chain 表。如果符号表项不是所需要的, 那么 chain[y] 则给出了具有相同哈希值的下一个符号表项。我们可以沿着 chain 链 一直搜索,直到所选中的符号表项包含了所需要的符号,或者 chain 项中包含值 STN_UNDEF。

上面的描述感觉有点复杂，其实说的简单点就是：

用目标函数名在用hash函数得到一个hash值，然后再做一些计算就可以得到这个函数在.dynsym段中这个函数对应的条目了。关于这个hash函数，是公用的，我们在[Android](http://lib.csdn.net/base/15)中的bonic/linker.c源码中也是可以找到的：



```
   unsigned long elf_hash (const unsigned char *name) {
   unsigned long h = 0, g; while (*name)
   {
   	h=(h<<4)+*name++; if (g = h & 0xf0000000)
   	h^=g>>24; h&=-g;
   }
   return h; 
   }
```

那么我们只要得到.hash段即可，但是我们怎么获取到这个section中呢？elf中并没有对这个段进行数据结构的描述，有人可能想到了我们在上图看到.hash段的type是HASH,那么我们再通过这个type来获取？但是之前说了，这个type不是唯一的，通过他来获取section是不靠谱的？那么我们该怎么办呢？这时候我们就要看一下程序头信息了：



![img](https://img-blog.csdn.net/20151121190653184?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

我们知道程序头信息是最后so被加载到内存中的映像描述，这里我们看到有一个.dynamic段。我们再看看so文件的装载视图和链接视图：

![img](https://img-blog.csdn.net/20151121192322980?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

这个我们在之前也说过，在so被加载到内存之后，就没有section了，对应的是segment了，也就是程序头中描述的结构，而且一个segment可以包含多个section,相同的section可以被包含到不同的segment中。.dynamic段一般用于动态链接的，所以.dynsym和.dynstr，.hash肯定包含在这里。我们可以解析了程序头信息之后，通过type获取到.dynamic程序头信息，然后获取到这个segment的偏移地址和大小，在进行解析成elf32_dyn结构。下面两种图就是程序头的type类型和dyn结构描述，可以在elf.h中找到：

![img](images/51258646)



```
/**
 * typedef struct dynamic{
  Elf32_Sword d_tag;
  union{
    Elf32_Sword	d_val;
    Elf32_Addr	d_ptr;
  } d_un;
} Elf32_Dyn;
 */
public static class elf32_dyn{
public byte[] d_tag = new byte[4];
public byte[] d_val = new byte[4];
public byte[] d_ptr = new byte[4];

/*public static class d_un{
public static byte[] d_val = new byte[4];
public static byte[] d_ptr = new byte[4];
}*/

@Override
public String toString(){
return "d_tag:"+Utils.bytes2HexString(d_tag)+";d_un_d_val:"+Utils.bytes2HexString(d_val)+";d_un_d_ptr:"+Utils.bytes2HexString(d_ptr);
}
}
```



这里，需要注意的是，C语言中的union联合体结构，所以我们在Java解析的时候需要注意，后面会详细介绍。



这里的三个字段很好理解：

d_tag：标示，标示这个dyn是什么类型的，是.dynsym还是.dynstr等

d_val：这个section的大小

d_ptr：这个section的偏移地址

细心的同学可能会发现一个问题，就是在这里寻找.dynamic也是通过类型的，然后再找到对应的section.这种方式和之前说的通过type来寻找section,有两个不同：

第一、在程序头信息中，type标示.dynamic段是唯一的，所以可以通过type来进行寻找

第二、我们看到上面的链接视图和装载视图发现，我们这种通过程序头中的信息来查找.dysym等section靠谱点，因为当so被加载到内存中，就不存在了section了，只有segment了。



# 三、实现方案



编写native程序，只是native直接返回字符串给UI。需要做的是对Java_com_example_shelldemo2_MainActivity_getString函数进行加密。加密和解密都是基于装载视图实现。需要注意的是，被加密函数如果用static声明，那么函数是不会出现在.dynsym中，是无法在装载视图中通过函数名找到进行解密的。当然，也可以采用取巧方式，类似上节，把地址和长度信息写入so头中实现。Java_com_example_shelldemo2_MainActivity_getString需要被调用，那么一定是能在.dynsym找到的。
加密流程：
1)  读取文件头，获取e_phoff、e_phentsize和e_phnum信息
2)  通过Elf32_Phdr中的p_type字段，找到DYNAMIC。从下图可以看出，其实DYNAMIC就是.dynamic section。从p_offset和p_filesz字段得到文件中的起始位置和长度
3)  遍历.dynamic，找到.dynsym、.dynstr、.hash section文件中的偏移和.dynstr的大小。在我的测试环境下，fedora 14和windows7 Cygwin x64中elf.h定义.hash的d_tag标示是：DT_GNU_HASH;而[安卓](http://lib.csdn.net/base/15)源码中的是：DT_HASH。
4)  根据函数名称，计算hash值
5)  根据hash值，找到下标hash % nbuckets的bucket；根据bucket中的值，读取.dynsym中的对应索引的Elf32_Sym符号；从符号的st_name所以找到在.dynstr中对应的字符串与函数名进行比较。若不等，则根据chain[hash % nbuckets]找下一个Elf32_Sym符号，直到找到或者chain终止为止。这里叙述得有些复杂，直接上代码。

```
for(i = bucket[funHash % nbucket]; i != 0; i = chain[i]){
  if(strcmp(dynstr + (funSym + i)->st_name, funcName) == 0){
    flag = 0;
    break;
  }
}
```



6)  找到函数对应的Elf32_Sym符号后，即可根据st_value和st_size字段找到函数的位置和大小
7)  后面的步骤就和上节相同了，这里就不赘述

解密流程为加密逆过程，大体相同，只有一些细微的区别，具体如下：
1)  找到so文件在内存中的起始地址
2)  也是通过so文件头找到Phdr；从Phdr找到PT_DYNAMIC后，需取p_vaddr和p_filesz字段，并非p_offset，这里需要注意。
3)  后续操作就加密类似，就不赘述。对内存区域数据的解密，也需要注意读写权限问题。





上面就介绍了完了，下面我们就可以来开始coding了。



# 四、代码实现

**第一、native程序**



```
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
	int mod = (funHash % nbucket);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "mod = %d\n", mod);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "i = 0x%d\n", bucket[mod]);

	for(i = bucket[mod]; i != 0; i = chain[i]){
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
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "npage =  0x%d", npage);
	__android_log_print(ANDROID_LOG_INFO, "JNITag", "npage =  0x%d", PAGE_SIZE);

	if(mprotect((void *) ((base + info.st_value) / PAGE_SIZE * PAGE_SIZE), 4096*npage, PROT_READ | PROT_EXEC | PROT_WRITE) != 0){
		print_debug("mem privilege change failed");
	}

	for(i=0;i< info.st_size - 1; i++){
		char *addr = (char*)(base + info.st_value -1 + i);
		*addr = ~(*addr);
	}

	if(mprotect((void *) ((base + info.st_value) / PAGE_SIZE * PAGE_SIZE), 4096*npage, PROT_READ | PROT_EXEC) != 0){
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
```



这里就不想做太多解释了，代码逻辑和之前文章中的加密section中的代码类似，只有在寻找函数的地方有点不同，这个也不再这里说明了，在加密的代码中我在说明一下。





**第二、加密程序**

1、[Java](http://lib.csdn.net/base/17)版本加密程序



```
private static void encodeFunc(byte[] fileByteArys){
	//寻找Dynamic段的偏移值和大小
	int dy_offset = 0,dy_size = 0;
	for(elf32_phdr phdr : type_32.phdrList){
		if(Utils.byte2Int(phdr.p_type) == ElfType32.PT_DYNAMIC){
			dy_offset = Utils.byte2Int(phdr.p_offset);
			dy_size = Utils.byte2Int(phdr.p_filesz);
		}
	}
	System.out.println("dy_size:"+dy_size);
	int dynSize = 8;
	int size = dy_size / dynSize;
	System.out.println("size:"+size);
	byte[] dest = new byte[dynSize];
	for(int i=0;i<size;i++){
		System.arraycopy(fileByteArys, i*dynSize + dy_offset, dest, 0, dynSize);
		type_32.dynList.add(parseDynamic(dest));
	}

	//type_32.printDynList();

	byte[] symbolStr = null;
	int strSize=0,strOffset=0;
	int symbolOffset = 0;
	int dynHashOffset = 0;
	int funcIndex = 0;
	int symbolSize = 16;

	for(elf32_dyn dyn : type_32.dynList){
		if(Utils.byte2Int(dyn.d_tag) == ElfType32.DT_HASH){
			dynHashOffset = Utils.byte2Int(dyn.d_ptr);
		}else if(Utils.byte2Int(dyn.d_tag) == ElfType32.DT_STRTAB){
			System.out.println("strtab:"+dyn);
			strOffset = Utils.byte2Int(dyn.d_ptr);
		}else if(Utils.byte2Int(dyn.d_tag) == ElfType32.DT_SYMTAB){
			System.out.println("systab:"+dyn);
			symbolOffset = Utils.byte2Int(dyn.d_ptr);
		}else if(Utils.byte2Int(dyn.d_tag) == ElfType32.DT_STRSZ){
			System.out.println("strsz:"+dyn);
			strSize = Utils.byte2Int(dyn.d_val);
		}
	}

	symbolStr = Utils.copyBytes(fileByteArys, strOffset, strSize);
	//打印全部的Symbol Name,注意用0来进行分割，C中的字符串都是用0作结尾的
	/*String[] strAry = new String(symbolStr).split(new String(new byte[]{0}));
		for(String str : strAry){
			System.out.println(str);
		}*/

	for(elf32_dyn dyn : type_32.dynList){
		if(Utils.byte2Int(dyn.d_tag) == ElfType32.DT_HASH){
			//这里的逻辑有点绕
			/**
			 * 根据hash值，找到下标hash % nbuckets的bucket；根据bucket中的值，读取.dynsym中的对应索引的Elf32_Sym符号；
			 * 从符号的st_name因此找到在.dynstr中对应的字符串与函数名进行比较。若不等，则根据chain[hash % nbuckets]找下一个Elf32_Sym符号，
			 * 直到找到或者chain终止为止。这里叙述得有些复杂，直接上代码。
					for(i = bucket[funHash % nbucket]; i != 0; i = chain[i]){
					  if(strcmp(dynstr + (funSym + i)->st_name, funcName) == 0){
					    flag = 0;
					    break;
					  }
					}
			 */
			int nbucket = Utils.byte2Int(Utils.copyBytes(fileByteArys, dynHashOffset, 4));
			int nchian = Utils.byte2Int(Utils.copyBytes(fileByteArys, dynHashOffset+4, 4));
			int hash = (int)elfhash(funcName.getBytes());
			hash = (hash % nbucket);
			//这里的8是读取nbucket和nchian的两个值
			funcIndex = Utils.byte2Int(Utils.copyBytes(fileByteArys, dynHashOffset+hash*4 + 8, 4));
			System.out.println("nbucket:"+nbucket+",hash:"+hash+",funcIndex:"+funcIndex+",chian:"+nchian);
			System.out.println("sym:"+Utils.bytes2HexString(Utils.int2Byte(symbolOffset)));
			System.out.println("hash:"+Utils.bytes2HexString(Utils.int2Byte(dynHashOffset)));

			byte[] des = new byte[symbolSize];
			System.arraycopy(fileByteArys, symbolOffset+funcIndex*symbolSize, des, 0, symbolSize);
			Elf32_Sym sym = parseSymbolTable(des);
			System.out.println("sym:"+sym);
			boolean isFindFunc = Utils.isEqualByteAry(symbolStr, Utils.byte2Int(sym.st_name), funcName);
			if(isFindFunc){
				System.out.println("find func....");
				return;
			}

			while(true){
				/**
				 *  lseek(fd, dyn_hash + 4 * (2 + nbucket + funIndex), SEEK_SET);
						if(read(fd, &funIndex, 4) != 4){
						  puts("Read funIndex failed\n");
						  goto _error;
						}
				 */
				//System.out.println("dyHash:"+Utils.bytes2HexString(Utils.int2Byte(dynHashOffset))+",nbucket:"+nbucket+",funIndex:"+funcIndex);
				funcIndex = Utils.byte2Int(Utils.copyBytes(fileByteArys, dynHashOffset+4*(2+nbucket+funcIndex), 4));
				System.out.println("funcIndex:"+funcIndex);

				System.arraycopy(fileByteArys, symbolOffset+funcIndex*symbolSize, des, 0, symbolSize);
				sym = parseSymbolTable(des);

				isFindFunc = Utils.isEqualByteAry(symbolStr, Utils.byte2Int(sym.st_name), funcName);
				if(isFindFunc){
					System.out.println("find func...");
					int funcSize = Utils.byte2Int(sym.st_size);
					int funcOffset = Utils.byte2Int(sym.st_value);
					System.out.println("size:"+funcSize+",funcOffset:"+funcOffset);
					//进行目标函数代码部分进行加密
					//这里须要注意的是从funcOffset-1的位置开始
					byte[] funcAry = Utils.copyBytes(fileByteArys, funcOffset-1, funcSize);
					for(int i=0;i<funcAry.length-1;i++){
						funcAry[i] = (byte)(funcAry[i] ^ 0xFF);
					}
					Utils.replaceByteAry(fileByteArys, funcOffset-1, funcAry);
					break;
				}
			}
			break;
		}

	}

}
```

这里的解密程序，需要说明一下。



1)、定位到.dynamic的segment,解析成elf32_dyn结构信息

```
//寻找Dynamic段的偏移值和大小
int dy_offset = 0,dy_size = 0;
for(elf32_phdr phdr : type_32.phdrList){
	if(Utils.byte2Int(phdr.p_type) == ElfType32.PT_DYNAMIC){
		dy_offset = Utils.byte2Int(phdr.p_offset);
		dy_size = Utils.byte2Int(phdr.p_filesz);
	}
}
System.out.println("dy_size:"+dy_size);
int dynSize = 8;
int size = dy_size / dynSize;
System.out.println("size:"+size);
byte[] dest = new byte[dynSize];
for(int i=0;i<size;i++){
	System.arraycopy(fileByteArys, i*dynSize + dy_offset, dest, 0, dynSize);
	type_32.dynList.add(parseDynamic(dest));
}
```



这里有一个解析elf32_dyn结构：



```
private static elf32_dyn parseDynamic(byte[] src){
	elf32_dyn dyn = new elf32_dyn();
	dyn.d_tag = Utils.copyBytes(src, 0, 4);
	dyn.d_ptr = Utils.copyBytes(src, 4, 4);
	dyn.d_val = Utils.copyBytes(src, 4, 4);
	return dyn;
}
```



这里需要注意的是，elf32_dyn中用到了联合体union结构，Java中是不存在这个类型的，所以我们需要了解这个联合体的含义，这里虽然是三个字段，但是大小是8个字节，而不是12字节，这个需要注意的。dyn.d_val和dyn.d_val是在一个联合体中的。



2)、计算目标函数的hash值，得到函数的偏移值和大小





这里的寻找逻辑有点饶人，但是我们知道了解原理即可：



![img](https://img-blog.csdn.net/20151121191243867?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

结合上面的这张图就可以理解了。其中nbucket和nchain，bucket[i]和chain[i]都是4个字节。他们的值就是目标函数在.dynsym中的位置。



2、C版加密程序



```
#include <stdio.h>
#include <fcntl.h>
#include "elf.h"
#include <stdlib.h>
#include <string.h>

typedef struct _funcInfo{
  Elf32_Addr st_value;
  Elf32_Word st_size;
}funcInfo;

Elf32_Ehdr ehdr;

//For Test
static void print_all(char *str, int len){
  int i;
  for(i=0;i<len;i++)
  {
    if(str[i] == 0)
      puts("");
    else
      printf("%c", str[i]);
  }
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

static Elf32_Off findTargetSectionAddr(const int fd, const char *secName){
  Elf32_Shdr shdr;
  char *shstr = NULL;
  int i;
  
  lseek(fd, 0, SEEK_SET);
  if(read(fd, &ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)){
    puts("Read ELF header error");
    goto _error;
  }
  
  lseek(fd, ehdr.e_shoff + sizeof(Elf32_Shdr) * ehdr.e_shstrndx, SEEK_SET);
  
  if(read(fd, &shdr, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr)){
    puts("Read ELF section string table error");
    goto _error;
  }
  
  if((shstr = (char *) malloc(shdr.sh_size)) == NULL){
    puts("Malloc space for section string table failed");
    goto _error;
  }
  
  lseek(fd, shdr.sh_offset, SEEK_SET);
  if(read(fd, shstr, shdr.sh_size) != shdr.sh_size){
    puts(shstr);
    puts("Read string table failed");
    goto _error;
  }
  
  lseek(fd, ehdr.e_shoff, SEEK_SET);
  for(i = 0; i < ehdr.e_shnum; i++){
    if(read(fd, &shdr, sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr)){
      puts("Find section .text procedure failed");
      goto _error;
    }
    if(strcmp(shstr + shdr.sh_name, secName) == 0){
      printf("Find section %s, addr = 0x%x\n", secName, shdr.sh_offset);
      break;
    }
  }
  free(shstr);
  return shdr.sh_offset;
_error:
  return -1;
}

static char getTargetFuncInfo(int fd, const char *funcName, funcInfo *info){
  char flag = -1, *dynstr;
  int i;
  Elf32_Sym funSym;
  Elf32_Phdr phdr;
  Elf32_Off dyn_off;
  Elf32_Word dyn_size, dyn_strsz;
  Elf32_Dyn dyn;
  Elf32_Addr dyn_symtab, dyn_strtab, dyn_hash;
  unsigned funHash, nbucket, nchain, funIndex;
  
  lseek(fd, ehdr.e_phoff, SEEK_SET);
  for(i=0;i < ehdr.e_phnum; i++){
    if(read(fd, &phdr, sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
      puts("Read segment failed");
      goto _error;
    }
    if(phdr.p_type ==  PT_DYNAMIC){
      dyn_size = phdr.p_filesz;
      dyn_off = phdr.p_offset;
      flag = 0;
      printf("Find section %s, size = 0x%x, addr = 0x%x\n", ".dynamic", dyn_size, dyn_off);
      break;
    }
  }
  if(flag){
    puts("Find .dynamic failed");
    goto _error;
  }
  flag = 0;

  printf("dyn_size:%d\n",dyn_size);
  printf("count:%d\n",(dyn_size/sizeof(Elf32_Dyn)));
  printf("off:%x\n",dyn_off);
  
  lseek(fd, dyn_off, SEEK_SET);
  for(i=0;i < dyn_size / sizeof(Elf32_Dyn); i++){
	int sizes = read(fd, &dyn, sizeof(Elf32_Dyn));
    if(sizes != sizeof(Elf32_Dyn)){
      puts("Read .dynamic information failed");
      //goto _error;
	  break;
    }    
    if(dyn.d_tag == DT_SYMTAB){
      dyn_symtab = dyn.d_un.d_ptr;
      flag += 1;
      printf("Find .dynsym, addr = 0x%x, val = 0x%x\n", dyn_symtab, dyn.d_un.d_val);
    }

    if(dyn.d_tag == DT_HASH){
      dyn_hash = dyn.d_un.d_ptr;
      flag += 2;
      printf("Find .hash, addr = 0x%x\n", dyn_hash);
    }
    if(dyn.d_tag == DT_STRTAB){
      dyn_strtab = dyn.d_un.d_ptr;
      flag += 4;
      printf("Find .dynstr, addr = 0x%x\n", dyn_strtab);
    }
    if(dyn.d_tag == DT_STRSZ){
      dyn_strsz = dyn.d_un.d_val;
      flag += 8;
      printf("Find .dynstr size, size = 0x%x\n", dyn_strsz);
    }
  }

  if((flag & 0x0f) != 0x0f){
    puts("Find needed .section failed\n");
    goto _error;
  }
  
  dynstr = (char*) malloc(dyn_strsz);
  if(dynstr == NULL){
    puts("Malloc .dynstr space failed");
    goto _error;
  }
  
  lseek(fd, dyn_strtab, SEEK_SET);
  if(read(fd, dynstr, dyn_strsz) != dyn_strsz){
    puts("Read .dynstr failed");
    goto _error;
  }
  
  funHash = elfhash(funcName);
  printf("Function %s hashVal = 0x%x\n", funcName, funHash);
  
  lseek(fd, dyn_hash, SEEK_SET);
  if(read(fd, &nbucket, 4) != 4){
    puts("Read hash nbucket failed\n");
    goto _error;
  }
  printf("nbucket = %d\n", nbucket);
  
  if(read(fd, &nchain, 4) != 4){
    puts("Read hash nchain failed\n");
    goto _error;
  }
  printf("nchain = %d\n", nchain);
  
  funHash = funHash % nbucket;
  printf("funHash mod nbucket = %d \n", funHash);
  
  lseek(fd, funHash * 4, SEEK_CUR);
  if(read(fd, &funIndex, 4) != 4){
    puts("Read funIndex failed\n");
    goto _error;
  }

  printf("funcIndex:%d\n", funIndex);
  
  lseek(fd, dyn_symtab + funIndex * sizeof(Elf32_Sym), SEEK_SET);
  if(read(fd, &funSym, sizeof(Elf32_Sym)) != sizeof(Elf32_Sym)){
    puts("Read funSym failed");
    goto _error;
  }
  
  if(strcmp(dynstr + funSym.st_name, funcName) != 0){
    while(1){
		printf("hash:%x,nbucket:%d,funIndex:%d\n",dyn_hash,nbucket,funIndex);
		lseek(fd, dyn_hash + 4 * (2 + nbucket + funIndex), SEEK_SET);
		if(read(fd, &funIndex, 4) != 4){
		  puts("Read funIndex failed\n");
		  goto _error;
		}

		printf("funcIndex:%d\n", funIndex);
		
		if(funIndex == 0){
		  puts("Cannot find funtion!\n");
		  goto _error;
		}
		
		lseek(fd, dyn_symtab + funIndex * sizeof(Elf32_Sym), SEEK_SET);
		if(read(fd, &funSym, sizeof(Elf32_Sym)) != sizeof(Elf32_Sym)){
		  puts("In FOR loop, Read funSym failed");
		  goto _error;
		}
		
		if(strcmp(dynstr + funSym.st_name, funcName) == 0){
		  break;
		}
    }
  }
  
  printf("Find: %s, offset = 0x%x, size = 0x%x\n", funcName, funSym.st_value, funSym.st_size);
  info->st_value = funSym.st_value;
  info->st_size = funSym.st_size;
  free(dynstr);
  return 0;
  
_error:
  free(dynstr);
  return -1;
}

int main(int argc, char **argv){
  char secName[] = ".text";
  char funcName[] = "Java_com_example_shelldemo2_MainActivity_getString";
  char *soName = "libdemo.so";
  char *content = NULL;
  int fd, i;
  Elf32_Off secOff;
  funcInfo info;

  unsigned a = elfhash(funcName);
  printf("a:%d\n", a);
  
  fd = open(soName, O_RDWR);
  if(fd < 0){
    printf("open %s failed\n", argv[1]);
    goto _error;
  }
  
  secOff = findTargetSectionAddr(fd, secName);
  if(secOff == -1){
    printf("Find section %s failed\n", secName);
    goto _error;
  }
  if(getTargetFuncInfo(fd, funcName, &info) == -1){
    printf("Find function %s failed\n", funcName);
    goto _error;
  }
  
  content = (char*) malloc(info.st_size);
  if(content == NULL){
    puts("Malloc space failed");
    goto _error;
  }
  
  lseek(fd, info.st_value - 1, SEEK_SET);
  if(read(fd, content, info.st_size) != info.st_size){
    puts("Malloc space failed");
    goto _error;
  }
  
  for(i=0;i<info.st_size -1;i++){
    content[i] = ~content[i];
  }
  
  lseek(fd, info.st_value-1, SEEK_SET);
  if(write(fd, content, info.st_size) != info.st_size){
    puts("Write modified content to .so failed");
    goto _error;
  }
  puts("Complete!");
  
_error:
  free(content);
  close(fd);
  return 0;
}
```

这里就不做介绍了。





上面对so中的函数加密成功了，那么下面我们来验证加密，我们使用IDA进行查看：

![img](https://img-blog.csdn.net/20151121195505084?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)![img](https://img-blog.csdn.net/20151121195519193?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

看到我们加密的函数内容已经面目全非了，看不到信息了。

比较加密前的：

![img](https://img-blog.csdn.net/20151121195455645?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)![img](https://img-blog.csdn.net/20151121195513111?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

哈哈，加密成功了~~



***\*案例下载地址：http://download.csdn.net/detail/jiangwei0910410003/9289009\****



**第三、测试Android项目**

我们用加密之后的so文件来测试一下：

```
package com.example.shelldemo;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

public class MainActivity extends Activity {

	private TextView tv;
	private native String getString();
	
	static{
		System.loadLibrary("demo");
	}
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		tv = (TextView) findViewById(R.id.tv);
		tv.setText(getString());
	}
}
```

运行成功啦。



#  五、总结

这篇文章是延续之前的加密section文章继续讲述了加密函数来实现so加固，这个和加密section唯一的区别就是如何找到加密函数的偏移地址和大小，其他都是类似的，那么对于so加固的知识点就用这两篇文章来介绍了，当然这两种方式都是有缺点的，就是如果我们在init_getString函数下断点，然后动态调试一下，就可以很轻易的破解了，而且通过dump出内存中运行的dex也是可以做到的，所以，没有绝对的安全，只有相对的攻防~~