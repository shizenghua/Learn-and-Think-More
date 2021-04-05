# 使用 aiohttp 和 asyncio 进行异步请求

src:https://blog.csdn.net/getcomputerstyle/article/details/78438246



# 前言
使用协程的异步请求以其低时消耗和对硬件的高利用而著称，翻看了很多论坛，发现协程在进行爬虫以及高频网络请求时的耗时比单多进程和单多线程还要好。本文将使用requests和使用aiohttp+asyncio进行比较，比较一下具体使用协程和不使用协程能差距多少。

本文测试所使用目标网址是廖雪峰老师python3教程的评论页 目前一共有2318页。

# 使用requests获取单个网址

```python
import requests,time
 
r=requests.get(url="https://www.liaoxuefeng.com/discuss/001409195742008d822b26cf3de46aea14f2b7378a1ba91000?page=1")
print(len(r.text))   #结果 40007   用时0.3s
```

# 使用 aiohttp+asyncio 获取单个网址

```python
import aiohttp, asyncio
 
async def fn():
	async with aiohttp.get(url='https://www.liaoxuefeng.com/discuss/001409195742008d822b26cf3de46aea14f2b7378a1ba91000?page=1') as resp:
		text=await resp.text()
		print(len(text))
 
loop = asyncio.get_event_loop()
loop.run_until_complete(fn())  #结果 40007   用时0.3s
```

看样对于单个请求来说，阻塞还是不阻塞没有实际意义，因为最终结果都是要等待网络访问，所以实际上对于整个需求来说，都是一个阻塞模型。而且async的方式代码量相对大。

# 使用requests获取50个网址

```python
import requests
 
a=[len(requests.get(url="https://www.liaoxuefeng.com/discuss/001409195742008d822b26cf3de46aea14f2b7378a1ba91000?page={}".format(i)).text) for i in range(1,51)]
print(a)   #结果 [40007, 40444, 40367, 40820, 40534, 40505, 40735, 40454, 40768, 40636, 40600, 40888, 41277, 41390, 41222, 40899, 40853, 40616, 40654, 40870, 41249, 40840, 40782, 41326, 41136, 40511, 40504, 40609, 41038, 41054, 40486, 40556, 41083, 40975, 40861, 40877, 40166, 40899, 40598, 40920, 40902, 40994, 40735, 40714, 41064, 40719, 40991, 40748, 40652, 40799]
```

```
用时：5.3s
```

# 使用aiohttp+asyncio获取50个网址

```python
import aiohttp, asyncio
 
async def fn(num):
	async with aiohttp.get(url='https://www.liaoxuefeng.com/discuss/001409195742008d822b26cf3de46aea14f2b7378a1ba91000?page={}'.format(num)) as resp:
		text=await resp.text()
		result.append(len(text))
 
result=[]
loop = asyncio.get_event_loop()
tasks=[asyncio.ensure_future(fn(i)) for i in range(1,51)]
loop.run_until_complete(asyncio.wait(tasks))  
print(result) #结果：[40636, 41326, 40853, 41277, 41249, 40735, 40616, 40454, 40888, 40899, 40007, 40768, 40486, 40870, 40820, 40444, 40367, 41136, 40609, 40975, 40504, 40166, 40920, 40598, 40556, 40652, 41083, 40735, 40799, 40899, 40902, 40748, 41064, 40505, 40654, 40511, 40600, 40534, 41390, 40782, 41222, 40840, 41038, 40714, 41054, 40877, 40719, 40991, 40861, 40994]

```

```
#用时：3.4s
```

有趣的是，协程最后返回的list的顺序和requests的完全顺序不一样，多试几次就发现，协程每次返回的顺序都不一样，因为它是异步的，不一定这些网络请求任务中哪个先完成。而requests每次得到的都是相同顺序的list。同时，服务器响应越慢，协程的优势体现的越明显，节省了很多网络等待的时间，而requests会阻塞的更久。

# 发送2318个请求
既然页面总共有2318个，我们不如全部使用爬一下。

## requests

```python
import requests
a=[len(requests.get(url="https://www.liaoxuefeng.com/discuss/001409195742008d822b26cf3de46aea14f2b7378a1ba91000?page={}".format(i)).text) for i in range(1,2319)]
print(a)
```

```
#用时：497s
```
## aiohttp+asyncio
由于同时并发2318个请求并交给网卡去处理会出现负荷过重，aiohttp默认同时最大支持1024个协程的进行，但是考虑到硬件的承受能力，我们采用维护一个“协程池”的方法。使用 asyncio.Semaphore来控制同时进行的最大IO量。每当有一个协程完成时，便加入一个新的协程：

```python
import aiohttp, asyncio
 
async def fn(num,sem):
	async with sem:
		async with aiohttp.get(url="https://www.liaoxuefeng.com/discuss/001409195742008d822b26cf3de46aea14f2b7378a1ba91000?page={}".format(num)) as resp:
			text=await resp.text()
			result.append(len(text))
 
loop=asyncio.get_event_loop()
result=[]
sem=asyncio.Semaphore(100) #维持100个信号量
tasks=[ asyncio.ensure_future(fn(i,sem)) for i in range(1,2319)]
loop.run_until_complete(asyncio.wait(tasks))
print(result)
```

```
#用时 预计200s左右
```

在用async测试的过程中，可能并发的请求服务器压力太大，廖雪峰老师的网站出现了504网关超时报错，第二天测试的时候，刚进行到一半服务器又崩溃了，服务器应该没做优化，这样一点并发流量怎么就垮了。

另外一篇将会使用 gevent+requsets 和 aiohttp+asyncio 的方式进行比较，两个都是协程模型。