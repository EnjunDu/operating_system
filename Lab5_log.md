# 配置环境

## 实验环境

虚拟虚拟化平台:Unraid 6.12.13 远程运行

操作系统:Ubuntu 22.04.4 LTS (GNU/Linux 6.8.0-40-generic x86_64)

小型操作系统内核:xv6

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

出现上述输出,gcc环境已配置好

## 安装qemu以及xv6

输入`sudo apt-get install qemu-system`安装qemu

输入`qemu-system-i386 --version`查看qemu版本:
```
user@Lab:~$ qemu-system-i386 --version
QEMU emulator version 6.2.0 (Debian 1:6.2+dfsg-2ubuntu6.22)
Copyright (c) 2003-2021 Fabrice Bellard and the QEMU Project developers
```

出现上述输出,qemu环境已配置好

输入`git clone https://github.com/mit-pdos/xv6-public.git`下载xv6系统

文件最后出现输出,说明操作系统镜像文件已准备好:
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

输入`make qemu`启动qemu,出现如下报错:

```
qemu-system-i386 -serial mon:stdio -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 2 -m 512
gtk initialization failed
make: *** [Makefile:225: qemu] Error 1
```

经查询,其原因通常与 QEMU 的 GTK 版本有关,这可能是由于缺少 GTK 库或没有正确配置显示环境引起的。在此处我们选择采用VNC后端启动QEMU,通过修改其中的`QEMUOPTS`变量如下,修改启动方式:

```QEMUOPTS = -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp $(CPUS) -m 512 -nographic $(QEMUEXTRA)
```

命令行中输入`make qemu`启动qemu,进入qemu界面:

```
SeaBIOS (version 1.15.0-1)


iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap sta8
init: starting sh
$
```

# 修改内存布局

## 阅读exec.c代码:
这段代码是一个典型的操作系统内核中的`exec`函数的实现,用于加载并执行一个新的程序。内存布局的实现主要涉及以下几个步骤: 

1. 查找并锁定可执行文件:
   - 调用`namei(path)`查找文件路径对应的inode。
   - 如果找不到文件,调用`end_op()`结束操作并返回错误。
   - 调用`ilock(ip)`锁定该inode。

2. 检查ELF头:
   - 调用`readi(ip, (char*)&elf, 0, sizeof(elf))`读取文件的ELF头。
   - 如果读取失败或魔数不匹配,跳转到错误处理部分。

3. 设置内核虚拟内存:
   - 调用`setupkvm()`设置内核虚拟内存。
   - 如果设置失败,跳转到错误处理部分。

4. 加载程序到内存:
   - 初始化`sz`为0。
   - 遍历程序头表(Program Header Table),读取每个程序段(Segment)并加载到内存中。

在对栈的分配上,其通过调用`allocuvm`进行分配内存

## 具体步骤:

1. 重新定义用户地址空间布局。
2. 修改 exec.c 以在高地址处分配栈。
3. 调整 proc 结构,以正确跟踪用户栈和堆的边界。
4. 修改与内存管理相关的函数,确保它们能正确处理栈和堆的分配及地址验证。
5. 实现栈增长的处理

## 调整用户地址空间布局

核心文件是 `memlayout.h`,它定义了内核和用户内存空间的布局。在 `memlayout.h` 中,`KERNBASE` 定义了用户地址空间的上限:
```
// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address
```
在这个空间内修改栈的位置。

## 修改exec.c

`exec.c` 负责加载用户程序并初始化用户地址空间。它使用 `allocuvm()` 为程序分配内存,加载代码段和数据段,并为栈分配一页内存。以下是栈分配的步骤

## 修改栈分配逻辑:

在`exec.c` 中,找到分配用户栈的代码,将栈分配到代码和堆的末尾,并分配两页(`2*PGSIZE`)大小的栈空间;
修改栈的位置到 `KERNBASE - 2*PGSIZE` 开始

```
// Allocate user stack at the top of the address space.
  uint stackbase = USERTOP - 2*PGSIZE;
  if((allocuvm(pgdir, stackbase, USERTOP)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(stackbase));
  sp = USERTOP;
```

## 修改 `proc->sz`的管理

当前 `proc->sz` 跟踪整个用户地址空间的大小，包括代码段、堆和栈。在修改后，`proc->sz` 只用于跟踪代码段和堆的大小，需要额外的字段来跟踪栈的起始位置。
故在`proc.h`中对proc结构设置新的结构项:

```
// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  uint stackbase;              // Base of the stack
};
```

## 修改`exec.c`中的地址分配，以在高地址处分配栈。
并在`exec.c`中设置跟踪栈的基地址
```
// Allocate user stack at the top of the address space.
  uint stackbase = USERTOP - 2*PGSIZE;
```
在`exec.c`中修改栈的分配方式
```
// Allocate user stack at the top of the address space.
  stackbase = USERTOP - 2*PGSIZE;
  if((allocuvm(pgdir, stackbase, USERTOP)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(stackbase));
  sp = USERTOP;
```

在`vm.c`中修改 `copyuvm` 函数,以确保在 fork 时正确地拷贝栈。:
```
copyuvm(pde_t *pgdir, uint sz, uint stackbase)
{
   ...
   // Copy the stack
  for(i = stackbase; i < USERTOP; i += PGSIZE){
    if((pte = walkpgdir(pgdir, (void *) i, 0)) == 0)
      panic("copyuvm: pte should exist");
    if(!(*pte & PTE_P))
      panic("copyuvm: page not present");
    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);
    if((mem = kalloc()) == 0)
      goto bad;
    memmove(mem, (char*)P2V(pa), PGSIZE);
    if(mappages(d, (void*)i, PGSIZE, V2P(mem), flags) < 0) {
      kfree(mem);
      goto bad;
    }
  }

  return d;
  ...
}
```

## 进行其他关于sz修改后的相关修改

在`proc.c`中，修改`fork`函数以传递 `stackbase` 参数给 `copyuvm` 函数。

```
if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz, curproc->stackbase)) == 0)
```

在`trap.c` 中,中处理页面错误，检测是否是栈溢出，并分配新的栈页:
```
// 新增触发页错误（Page Fault）的情况
case T_PGFLT:
  
  if (rcr2() < USERTOP)// 条件检查:确保页面错误地址在用户栈范围内。
  {
    cprintf("page error %x ",rcr2());
    cprintf("stack pos : %x\n", myproc()->stackbase);
    // 为用户栈分配新的页面
    if ((myproc()->stackbase = allocuvm(myproc()->pgdir, myproc()->stackbase - 1 * PGSIZE,
    myproc()->stackbase)) == 0)
    {
      myproc()->killed = 1;
    }
    myproc()->stackbase-=PGSIZE; // 更新用户栈的栈顶位置
    cprintf("create a new page %x\n", myproc()->stackbase);
      //clearpteu(myproc()->pgdir, (char *) (myproc()->stackbase - PGSIZE));
    return;
  }
  else
  {
     myproc()->killed = 1;
    break;
  }
```

## 编写测试程序`testcase.c`

```
#include "types.h"
#include "stat.h"
#include "user.h"
#include "memlayout.h"

// 获取当前栈指针
uint get_stack_pointer() {
    uint sp;
    asm volatile("movl %%esp, %0" : "=r" (sp));
    return sp;
}

// 测试栈是否位于堆之上
void test_stack_position() {
    printf(1, "\n=== Test 1: Stack Position ===\n");
    uint sp = get_stack_pointer();
    uint heap_end = (uint)sbrk(0);

    printf(1, "Current stack pointer: 0x%x\n", sp);
    printf(1, "Heap end: 0x%x\n", heap_end);

    if (sp > heap_end) {
        printf(1, "Test passed: Stack is above heap.\n");
    } else {
        printf(1, "Test failed: Stack is not correctly positioned.\n");
    }
}

// 测试堆和栈是否分离
void test_heap_stack_separation() {
    printf(1, "\n=== Test 2: Heap and Stack Separation ===\n");

    uint heap_start = (uint)sbrk(0);
    sbrk(4096 * 10);  // 增加10页堆
    uint heap_end = (uint)sbrk(0);
    uint sp = get_stack_pointer();

    printf(1, "Heap range: 0x%x - 0x%x\n", heap_start, heap_end);
    printf(1, "Stack pointer: 0x%x\n", sp);

    if (sp > heap_end) {
        printf(1, "Test passed: Stack is above heap after sbrk.\n");
    } else {
        printf(1, "Test failed: Stack and heap overlap.\n");
    }
}

// 递归调用以强制栈增长
void recursive_call(int depth) {
    char buffer[1024];  // 在栈上分配空间，逐步增长栈
    printf(1, "Recursion depth: %d, buffer address: 0x%x\n", depth, buffer);

    // 递归调用，强制栈增长
    if (depth < 100) {
        recursive_call(depth + 1);
    }
}

// 测试栈增长
void test_stack_growth() {
    printf(1, "\n=== Test 3: Stack Growth ===\n");
    printf(1, "Testing stack growth with recursion...\n");
    recursive_call(1);  // 初始调用

    printf(1, "Test completed.\n");
}

// 深度递归调用以测试更大规模的栈增长
void deep_recursion(int depth) {
    char buffer[4096];  // 分配4KB的空间，确保每次递归占用整页
    printf(1, "Recursion depth: %d, buffer address: 0x%x\n", depth, buffer);

    // 在深度达到一定值之前递归
    if (depth < 200) {
        deep_recursion(depth + 1);
    }
}

// 测试深度递归引发栈增长
void test_deep_stack_growth() {
    printf(1, "\n=== Test 4: Deep Stack Growth ===\n");
    printf(1, "Starting deep recursion test...\n");

    deep_recursion(1);  // 初始递归调用，测试栈增长

    printf(1, "Deep recursion test completed.\n");
}

// 强制堆增长以接近栈区域
void force_stack_heap_collision() {
    // 通过增加堆的大小来接近栈区域
    sbrk((KERNBASE - (uint)sbrk(0)) / 2);  // 增加到接近栈的区域

    char buffer[1024];  // 在栈上分配空间
    printf(1, "Allocated stack buffer at: 0x%x\n", buffer);
}

// 测试堆与栈的冲突
void test_stack_heap_collision() {
    printf(1, "\n=== Test 5: Stack-Heap Collision ===\n");

    force_stack_heap_collision();  // 强制堆增长，接近栈

    printf(1, "Test completed.\n");
}

int main() {
    // 执行所有测试
    test_stack_position();         // 测试栈位置
    test_heap_stack_separation();  // 测试堆与栈的分离
    test_stack_growth();           // 测试栈增长
    test_deep_stack_growth();      // 测试深度递归栈增长
    test_stack_heap_collision();   // 测试堆与栈的冲突

    exit();
}
```

在`Makefile`中添加编译`testcase.c`的命令:
```
  _testcase\
```

# 进行实验

运行`make qemu`启动QEMU，并在QEMU中运行测试程序:
```
$ testcase
```
一共有五个测试，具体输出如下:

## 测试1 测试栈位置
输出当前栈指针，堆结束地址，判断栈是否位于堆之上，可以发现栈指针位于堆之上
```
=== Test 1: Stack Position ===
Current stack pointer: 0x7FFFEFB0
Heap end: 0x2000
Test passed: Stack is above heap.
```

## 测试2 测试堆和栈分离
输出堆范围，栈指针，判断栈是否位于堆之上，可以发现栈指针位于堆之上
```
=== Test 2: Heap and Stack Separation ===
Heap range: 0x2000 - 0xC000
Stack pointer: 0x7FFFEFA0
Test passed: Stack is above heap after sbrk.
```

## 测试3 测试栈增长
递归调用，输出递归深度和栈地址，可以发现栈地址逐渐增长
```
=== Test 3: Stack Growth ===
Testing stack growth with recursion...
Recursion depth: 1, buffer address: 0x7FFFE3A0
Recursion depth: 2, buffer address: 0x7FFFE7A0
Recursion depth: 3, buffer address: 0x7FFFEBA0
```
## 测试4 深度递归调用以测试更大规模的栈增长
递归调用，输出递归深度和栈地址，可以发现栈地址逐渐增长，并且当深度达到一定值时，会触发页错误，创建新的页，由此不断重复，直到到达设定的递归深度
```
=== Test 4: Deep Stack Growth ===
Starting deep recursion test...
Recursion depth: 1, buffer address: 0x7FFFBFA0
Recursion depth: 2, buffer address: 0x7FFFCFA0
Recursion depth: 3, buffer address: 0x7FFFDFA0
...
Recursion depth: 48, buffer address: 0x7FFD0DC0
page error 7ffcbda0 stack pos : 7ffcc000
create a new page 7ffcb000
Recursion depth: 49, buffer address: 0x7FFCBDA0
Recursion depth: 50, buffer address: 0x7FFCCDA
ecursion depth: 51, buffer address: 0x7FFCDDA0
page error 7ffcad80 stack pos : 7ffcb000
create a new page 7ffca000
page error 7ffc9d80 stack pos : 7ffca000
create a new page 7ffc9000
page error 7ffc8d80 stack pos : 7ffc9000
create a new page 7ffc8000
Recursion depth: 52, buffer address: 0x7FFC8D80
...
...
page error 7ff37760 stack pos : 7ff38000
create a new page 7ff37000
page error 7ff36760 stack pos : 7ff37000
create a new page 7ff36000
page error 7ff35760 stack pos : 7ff36000
create a new page 7ff35000
Recursion depth: 199, buffer address: 0x7FF35760
Recursion depth: 200, buffer address: 0x7FF36760
```
## 测试5 测试堆与栈的冲突
强制堆增长，接近栈区域，输出栈地址，可以发现栈地址逐渐增长，直到触发页错误，创建新的页，从而实现堆与栈的冲突
```
=== Test 5: Stack-Heap Collision ===
allocuvm out of memory
Allocated stack buffer at: 0x7FFFEBA0
Test completed.
```