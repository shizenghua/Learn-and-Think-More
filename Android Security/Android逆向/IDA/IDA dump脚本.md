```
static main(void)
{
	auto fp,dexAddress,end,size;
	dexAddress = 0xD8580000;
	end = 0xD85BA000;
	fp = fopen("D:\\bep.so","wb");
	for(;dexAddress<end; dexAddress++)
	fputc(Byte(dexAddress),fp);
}
```

