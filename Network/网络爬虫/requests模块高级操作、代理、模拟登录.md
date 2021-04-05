# requests模块高级操作、代理、模拟登录

src:https://www.cnblogs.com/robertx/p/10944201.html



# 一.代理操作
* **什么是代理?**
a. 就是代理服务器
* **提供代理的网站:**
a. 快代理
b. 西祠代理
c. goubanjia
* **代理的匿名度**
a. 透明代理: 对方服务器可以知道你使用了代理,并且也知道你的真实ip
b. 匿名代理: 对方服务器可以知道你使用了代理,但不知道你的真实ip
c. 高匿代理: 对方服务器不知道你使用了代理, 更不知道那你的真实ip
* **代理的类型:**
a. http: 该类型的代理ip只可以发起http协议头对应的请求
b. https: 该类型的代理ip只可以发起https协议头对应的请求
* **requests的get方法和post方法常用的参数**
a. url
b. headers
c. data/params
d. proxies

示例:用代理访问

```python
import random
import requests

https = [
    {'https':'223.19.212.30:8380'},
    {'https':'221.19.212.30:8380'}
]
http = [
    {'http':'223.19.212.30:8380'},
    {'http':'221.19.212.30:8380'}
]

headers = {
    'User-Agent':'Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36'
}
url = 'https://www.baidu.com/s?wd=ip'

if url.split(':')[0] == 'https':
    page_text = requests.get(url=url,headers=headers,proxies=random.choice(https)).text
else:
     page_text = requests.get(url=url,headers=headers,proxies=random.choice(http)).text

with open('./ip.html','w',encoding='utf-8') as fp:
    fp.write(page_text)
```
# 二.验证码识别
 常用的验证码的打码平台有云打码, 打码兔, 超级鹰,以云打码为例

**云打码使用流程**
* 网址: <http://www.yundama.com/demo.html>

* 注册种类(两种):
1. 普通用户
a. 登录普通用户,查询剩余题分,如果不够需要充值
2. 开发者用户
a. 创建一个软件：我的软件-》创建一个新软件（软件名称，秘钥不可以修改），使用软件的id和秘钥
b. 下载示例代码：开发文档-》点此下载：云打码接口DLL-》PythonHTTP示例下载

下载demo之后,可以看到api的使用示例:

```python
import http.client, mimetypes, urllib, json, time, requests

######################################################################

class YDMHttp:

    apiurl = 'http://api.yundama.com/api.php'
    username = ''
    password = ''
    appid = ''
    appkey = ''

    def __init__(self, username, password, appid, appkey):
        self.username = username  
        self.password = password
        self.appid = str(appid)
        self.appkey = appkey

    def request(self, fields, files=[]):
        response = self.post_url(self.apiurl, fields, files)
        response = json.loads(response)
        return response
    
    def balance(self):
        data = {'method': 'balance', 'username': self.username, 'password': self.password, 'appid': self.appid, 'appkey': self.appkey}
        response = self.request(data)
        if (response):
            if (response['ret'] and response['ret'] < 0):
                return response['ret']
            else:
                return response['balance']
        else:
            return -9001
    
    def login(self):
        data = {'method': 'login', 'username': self.username, 'password': self.password, 'appid': self.appid, 'appkey': self.appkey}
        response = self.request(data)
        if (response):
            if (response['ret'] and response['ret'] < 0):
                return response['ret']
            else:
                return response['uid']
        else:
            return -9001

    def upload(self, filename, codetype, timeout):
        data = {'method': 'upload', 'username': self.username, 'password': self.password, 'appid': self.appid, 'appkey': self.appkey, 'codetype': str(codetype), 'timeout': str(timeout)}
        file = {'file': filename}
        response = self.request(data, file)
        if (response):
            if (response['ret'] and response['ret'] < 0):
                return response['ret']
            else:
                return response['cid']
        else:
            return -9001

    def result(self, cid):
        data = {'method': 'result', 'username': self.username, 'password': self.password, 'appid': self.appid, 'appkey': self.appkey, 'cid': str(cid)}
        response = self.request(data)
        return response and response['text'] or ''

    def decode(self, filename, codetype, timeout):
        cid = self.upload(filename, codetype, timeout)
        if (cid > 0):
            for i in range(0, timeout):
                result = self.result(cid)
                if (result != ''):
                    return cid, result
                else:
                    time.sleep(1)
            return -3003, ''
        else:
            return cid, ''

    def report(self, cid):
        data = {'method': 'report', 'username': self.username, 'password': self.password, 'appid': self.appid, 'appkey': self.appkey, 'cid': str(cid), 'flag': '0'}
        response = self.request(data)
        if (response):
            return response['ret']
        else:
            return -9001

    def post_url(self, url, fields, files=[]):
        for key in files:
            files[key] = open(files[key], 'rb');
        res = requests.post(url, files=files, data=fields)
        return res.text
```
将使用操作封装为函数

```python
#将示例代码中的可执行程序封装成函数
def transformCodeImg(imgPath,imgType):
    # 普通用户名
    username    = 'bobo328410948'

    # 密码
    password    = 'bobo328410948'                            

    # 软件ＩＤ，开发者分成必要参数。登录开发者后台【我的软件】获得！
    appid       = 6003                                     

    # 软件密钥，开发者分成必要参数。登录开发者后台【我的软件】获得！
    appkey      = '1f4b564483ae5c907a1d34f8e2f2776c'    

    # 图片文件
    filename    = imgPath                        

    # 验证码类型，# 例：1004表示4位字母数字，不同类型收费不同。请准确填写，否则影响识别率。在此查询所有类型 http://www.yundama.com/price.html
    codetype    = imgType

    # 超时时间，秒
    timeout     = 30                                    
    result = None
    # 检查
    if (username == 'username'):
        print('请设置好相关参数再测试')
    else:
        # 初始化
        yundama = YDMHttp(username, password, appid, appkey)

        # 登陆云打码
        uid = yundama.login();
        print('uid: %s' % uid)

        # 查询余额
        balance = yundama.balance();
        print('balance: %s' % balance)

        # 开始识别，图片路径，验证码类型ID，超时时间（秒），识别结果
        cid, result = yundama.decode(filename, codetype, timeout);
        
    return result
```
# 三.Cookie相关操作
 cookie可以是服务端记录客户端的相关状态, 有些网站get请求数据时,需要携带cookie

```python
#需求：爬取雪球网中的新闻标题和对应的内容简介
url = 'https://xueqiu.com/v4/statuses/public_timeline_by_category.json?since_id=-1&max_id=-1&count=10&category=-1'
headers = {
    'User-Agent':'Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36'
}
json_obj = requests.get(url=url,headers=headers).json()
print(json_obj)
```

# 四.基于multiprocessing.dummy线程池的数据爬取
* 需求：爬取梨视频的视频信息，并计算其爬取数据的耗时

普通爬取

```python
import time
import requests
import random
from lxml import etree
import re
from fake_useragent import UserAgent
#安装fake-useragent库:pip install fake-useragent
url = 'http://www.pearvideo.com/category_1'
#随机产生UA,如果报错则可以添加如下参数：
#ua = UserAgent(verify_ssl=False,use_cache_server=False).random
#禁用服务器缓存：
#ua = UserAgent(use_cache_server=False)
#不缓存数据：
#ua = UserAgent(cache=False)
#忽略ssl验证：
#ua = UserAgent(verify_ssl=False)

ua = UserAgent().random
headers = {
    'User-Agent':ua
}
#获取首页页面数据
page_text = requests.get(url=url,headers=headers).text
#对获取的首页页面数据中的相关视频详情链接进行解析
tree = etree.HTML(page_text)
li_list = tree.xpath('//div[@id="listvideoList"]/ul/li')
detail_urls = []
for li in li_list:
    detail_url = 'http://www.pearvideo.com/'+li.xpath('./div/a/@href')[0]
    title = li.xpath('.//div[@class="vervideo-title"]/text()')[0]
    detail_urls.append(detail_url)
for url in detail_urls:
    page_text = requests.get(url=url,headers=headers).text
    vedio_url = re.findall('srcUrl="(.*?)"',page_text,re.S)[0]
    
    data = requests.get(url=vedio_url,headers=headers).content
    fileName = str(random.randint(1,10000))+'.mp4' #随机生成视频文件名称
    with open(fileName,'wb') as fp:
        fp.write(data)
        print(fileName+' is over')
```
基于线程池的爬取

```python
import requests
import random
from lxml import etree
import re
from fake_useragent import UserAgent
#安装fake-useragent库:pip install fake-useragent
#导入线程池模块
from multiprocessing.dummy import Pool
#实例化线程池对象
pool = Pool()
url = 'http://www.pearvideo.com/category_1'
#随机产生UA
ua = UserAgent().random
headers = {
    'User-Agent':ua
}
#获取首页页面数据
page_text = requests.get(url=url,headers=headers).text
#对获取的首页页面数据中的相关视频详情链接进行解析
tree = etree.HTML(page_text)
li_list = tree.xpath('//div[@id="listvideoList"]/ul/li')

detail_urls = []#存储二级页面的url
for li in li_list:
    detail_url = 'http://www.pearvideo.com/'+li.xpath('./div/a/@href')[0]
    title = li.xpath('.//div[@class="vervideo-title"]/text()')[0]
    detail_urls.append(detail_url)
    
vedio_urls = []#存储视频的url
for url in detail_urls:
    page_text = requests.get(url=url,headers=headers).text
    vedio_url = re.findall('srcUrl="(.*?)"',page_text,re.S)[0]
    vedio_urls.append(vedio_url) 
    
#使用线程池进行视频数据下载    
func_request = lambda link:requests.get(url=link,headers=headers).content
video_data_list = pool.map(func_request,vedio_urls)

#使用线程池进行视频数据保存
func_saveData = lambda data:save(data)
pool.map(func_saveData,video_data_list)
def save(data):
    fileName = str(random.randint(1,10000))+'.mp4'
    with open(fileName,'wb') as fp:
        fp.write(data)
        print(fileName+'已存储')
        
pool.close()
pool.join()
```
# 五.模拟古诗文登录
* 爬取古诗文网,模拟登陆

```python
import requests
from lxml import etree

s = requests.Session()

headers = {
    "User-Agent":'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36'
}

# 获取验证码图片并且让打码平台
url = 'https://so.gushiwen.org/user/login.aspx'
page_text = s.get(url=url,headers=headers).text
tree = etree.HTML(page_text)
img_src = 'https://so.gushiwen.org' + tree.xpath('//*[@id="imgCode"]/@src')[0]

img_data = s.get(url=img_src,headers=headers).content
with open('./gushiwen.jpg','wb') as f:
    f.write(img_data)
result = transformCodeImg('./gushiwen.jpg',1004)

__VIEWSTATE = tree.xpath('//*[@id="__VIEWSTATE"]/@value')[0]
__VIEWSTATEGENERATOR = tree.xpath('//*[@id="__VIEWSTATEGENERATOR"]/@value')[0]


# 模拟登陆
post_url = 'https://so.gushiwen.org/user/login.aspx?from=http%3a%2f%2fso.gushiwen.org%2fuser%2fcollect.aspx'
data = {
    "__VIEWSTATE": __VIEWSTATE,
    "__VIEWSTATEGENERATOR": __VIEWSTATEGENERATOR,
    "from": "http://so.gushiwen.org/user/collect.aspx",
    "email":"hey2380@163.com",
    "pwd": "123456",
    "code": result,
    "denglu": "登录",
}

response = s.post(url=post_url,headers=headers,data=data)
page_text = response.text
print(response.status_code)
# print(page_text)
print(result)
page_text = response.text
with open('./gushi.html','w',encoding='utf-8') as f:
    f.write(page_text)
```