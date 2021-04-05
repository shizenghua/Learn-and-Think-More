# des为什么有不同类型的sbox

一种是是这样的，我找遍了网上大部分的DES的文章，都没有解释这个的

![img](images/630711_K7JJ7M7CZWMFPXF.png)

一种是这样的，这个是标准的

![img](images/630711_N2M2DZEHABYWNYH.png)





这要从DES算法原理谈起，标准模式下是Sbox,Pbox，要进行2次查表运算，这是固定计算。为了提高效率，把这2次查表运算，进行计算合并，也就是有了SPbox。

补个图，清晰点：

![img](images/12993_3NQ4SKQGE9EGWGB.jpg)