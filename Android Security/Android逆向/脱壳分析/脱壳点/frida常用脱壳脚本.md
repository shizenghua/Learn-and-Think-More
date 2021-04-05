```


var processName = "com.xgtl.assistant";
function Dex_Dump_file(DumpClassName, Base, size) {
    var dex_begin = ptr(Base);
    var dex_size = size;
    //if (SavaSize.indexOf(dex_size) != -1) {return;}
    var Dex_magic = Memory.readUtf8String(dex_begin);
    if (Dex_magic.indexOf("035") > -1) {
        var Dex_fileName = DumpClassName + "_" + dex_size.toString() + ".dex";
        var dex_path = "/data/data/" + processName + "/" + Dex_fileName;
        console.log("Dex_DUMP FileName :", dex_path);
        var dex_file = new File(dex_path, "wb");
        var buffw = Memory.readByteArray(dex_begin, dex_size);
        //console.log(buffw);
        dex_file.write(buffw);
        dex_file.flush();
        dex_file.close();
        SavaSize.push(dex_size);
    }
}
function  hook_art_Base_addr() {
    var symbols = Module.enumerateSymbolsSync("libart.so");
 
    for (var i = 0; i < symbols.length; i++) {
        var symbol = symbols[i];
        if (symbol.name == "_ZN3art7DexFile10OpenMemoryEPKhjRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPNS_6MemMapEPKNS_10OatDexFileEPS9_") {
            art_DexFile_OpenMemory = symbol.address;
            console.log("art_DexFile_OpenMemory ", symbol.address, symbol.name);
        } else if (symbol.name == "_ZN3art11ClassLinker10LoadMethodERKNS_7DexFileERKNS_21ClassDataItemIteratorENS_6HandleINS_6mirror5ClassEEEPNS_9ArtMethodE") {
            art_ClassLinker_LoadMethod = symbol.address;
            console.log("art_ClassLinker_LoadMethod ", symbol.address, symbol.name);
        } else if (symbol.name == "_ZN3art7DexFileC2EPKhjRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileE") {
            art_DexFile_DexFile = symbol.address;
            console.log("art_DexFile_DexFile ", symbol.address, symbol.name);
        }else if (symbol.name == "_ZN3art9ArtMethod6InvokeEPNS_6ThreadEPjjPNS_6JValueEPKc") {
            art_ArtMethod_Invoke = symbol.address;
            console.log("art_ArtMethod_Invoke ", symbol.address, symbol.name);
        }
    }
}
function Hook_art_DexFile_DexFile() {
    if (art_DexFile_DexFile != null) {
        Interceptor.attach(art_DexFile_DexFile, {
            onEnter: function (args) {
                this.dex_Base = args[1];
                this.dex_Size = args[2];
              console.log("dex_file Info:", this.dex_Base , this.dex_Size );
            },
            onLeave: function (retval) {
                console.log("= art_DexFile_DexFile DUMP =");
                Dex_Dump_file("DexFile", this.dex_Base ,this.dex_Size.toInt32());
            }
        });
    }
function Hook_art_ClassLinker_LoadMethod(){
    if (art_ClassLinker_LoadMethod != null){
        Interceptor.attach(art_ClassLinker_LoadMethod, {
            onEnter: function (args){
                this.dex_file =  args[1];
                this.dex_begin = ptr (this.dex_file.add(4).readUInt());
                this.dex_size = this.dex_file.add(8).readInt();
                console.log("art_ClassLinker_LoadMethod Info:", this.dex_begin, this.dex_size);
                console.log("art_ClassLinker_LoadMethod ", hexdump( this.dex_begin, ))
            },
            onLeave: function (retval){
                console.log("art_ClassLinker_LoadMethod retval :", retval);
                //console.log("= art_ClassLinker_LoadMethod DUMP =");
                Dex_Dump_file("LoadMethod", this.dex_begin, this.dex_size);
            }
        })
    }
}

//com.xgtl.aggregate.utils.f
function runJava(clx) {
    Java.perform(function (){
        var hookgoal = Java.use(clx);
        console.log(hookgoal);
        var funcs = hookgoal.class.getDeclaredMethods();
        for(var i = 0; i < funcs.length; i++){
            var Method = funcs[i];
            console.log(Method);
            Method.setAccessible(true);
            var constructor = funcs[0].class.getDeclaredConstructors();
            console.log("constructor",constructor);
            try{
                Method.invoke(null, null);
            }catch(err){
                console.log(err);
            }
        }
    })
}
}
```





