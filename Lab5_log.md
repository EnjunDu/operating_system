# 配置环境

## 实验环境

虚拟机架构：Unraid 6.12.13 远程运行

操作系统：Ubuntu 22.04.4 LTS (GNU/Linux 6.8.0-40-generic x86_64)


## gcc配置

输入`sudo apt update`更新软件包列表

输入`sudo apt-get install -y build-essential git gcc-multilib `配置gcc

输入`objdump -i`检查是否支持64位:
```
user@Lab:~$ objdump -i
BFD header file version (GNU Binutils for Ubuntu) 2.38
elf64-x86-64
```

输入`gcc -m32 -print-libgcc-file-name `查看32位gcc库文件路径:
```
user:~$ gcc -m32 -print-libgcc-file-name 
/usr/lib/gcc/x86_64-linux-gnu/11/32/libgcc.a
```

出现上述输出，gcc环境已配置好

## 安装qemu以及xv6

输入`sudo apt-get install qemu-system`安装qemu

输入`qemu-system-i386 --version`查看qemu版本:
```
user@Lab:~$ qemu-system-i386 --version
QEMU emulator version 6.2.0 (Debian 1:6.2+dfsg-2ubuntu6.22)
Copyright (c) 2003-2021 Fabrice Bellard and the QEMU Project developers
```

出现上述输出，qemu环境已配置好

输入`git clone https://github.com/mit-pdos/xv6-public.git`下载xv6系统

文件最后出现输出，说明操作系统镜像文件已准备好：
``` 
+0 records in
+0 records out
512 bytes (512 B) copied, 0.000543844 s, 938 kB/s
dd if=kernal of=xf6.img seek=1 conv=notrunc
393+1 records in
393+1 records out
```

## 启动qemu

输入`~/xv6$ echo "add-auto-load-safe-path $HOME/xv6/.gdbinit" > ~/.gdbinit` 配置gdb

输入`make qemu`启动qemu,出现如下报错：

```
qemu-system-i386 -serial mon:stdio -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 2 -m 512
gtk initialization failed
make: *** [Makefile:225: qemu] Error 1
```

经查询，其原因通常与 QEMU 的 GTK 版本有关，这可能是由于缺少 GTK 库或没有正确配置显示环境引起的。在此处我们选择采用VNC后端启动QEMU，通过修改其中的`QEMUOPTS`变量如下，修改启动方式：

```QEMUOPTS = -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp $(CPUS) -m 512 -nographic $(QEMUEXTRA)
```

命令行中输入`make qemu`启动qemu，进入qemu界面：

```
SeaBIOS (version 1.15.0-1)


iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap sta8
init: starting sh
$
```
