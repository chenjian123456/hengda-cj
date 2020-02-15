# 基于深度学习的目标检测及周界系统

## Env
[环境配置](ENV.md)

## 模型准备
[模型准备](MODELS.md)

## Usage

## 编译
```bash
./build.sh
```
注意查看输出，必要的输出我都添加了，
编译结果输出在`out`文件夹，自动打包输出到`$HOME`，方便上传到`Atlas 500`

## 上传
使用`scp`上传。虽然无法从本机上传服务器，但是可以从服务器反向从本机下载(本机做SSH服务器)

0. 安装`SSH`服务端并启动
```
sudo apt install openssh-server
service sshd start
```
1. 使用ssh登录到服务器
2. 执行命令，请将命令中的用户名、IP地址和文件路径改成自己的
```
scp ffiirree@192.168.2.222:/home/ffiirree/out.tar ./
```
3. 解压
```
tar -zxvf out.tar
```
## 运行

```bash
./out/main
```

## 查看日志
日志文件保存在`/var/dlog`，查看日志文件
```
ll /var/dlog/
```
查看日志内容
```
cat /var/dlog/<filename>
```

## RPC

下载[co库](https://github.com/idealvin/co)，使用`xmake`交叉编译
```sh
$ cd co
$ xmake f -p linux --sdk=${DDDK_HOME}toolchains/Euler_compile_env_cross/arm/cross_compile/install
$ xmake
```

工程目录结构如下：

```
|-- server
    |-- hengda_server
    |-- include
    |-- lib
        |-- euler
```

其中`include`为`co库`的头文件，`euler`下放交叉编译后的`libbase.a`
