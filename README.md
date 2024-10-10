# 第一版

### 1. **实验 3: System Call 实验**

- 实验要求
  - **第一部分**：准备并运行 xv6 环境。你需要使用 QEMU 模拟器和编译器工具链来运行 xv6 内核。
  - **第二部分**：向 xv6 添加一个新的系统调用。你将修改和扩展 xv6 源代码以实现此功能。
  - 提交内容包括实验报告以及你修改或创建的文件(Lab 3 System Call (1))。
- 加分项
  - 如果你在 RISC-V 架构上完成该实验，可以获得额外的 10% 分数(Lab 3 System Call (1))。

### 2. **实验 4: 调度器实验**

- **实验要求**：
  - 实现优先级调度算法，将原来的轮转调度替换为优先级调度。你需要为每个进程添加一个优先级值（范围为 0 到 31），并确保总是调度优先级最高的进程。
  - 添加一个系统调用，使进程可以随时改变其优先级。如果某进程的优先级低于当前队列中的其他进程，系统必须切换到优先级更高的进程。
  - 实现的代码需要通过用户测试验证(Lab 4 Scheduler)。
- **加分项**：
  - **加分项 1（5%）**：防止进程饥饿（优先级老化机制）：如果一个进程等待时间过长，提高其优先级；当其运行时，降低优先级。
  - **加分项 2（5%）**：实现优先级捐赠或优先级继承机制。
  - **加分项 3（5%）**：添加用于追踪进程调度性能的字段，允许计算每个进程的周转时间和等待时间，并提供系统调用或自动输出这些值(Lab 4 Scheduler)。

### 3. **实验 5: 内存管理实验**

- **实验要求**：
  - **第一部分（70%）**：修改 xv6 的内存布局，将栈移至地址空间的顶部，并实现栈从顶部向下增长。这部分要求重新设计用户内存布局，使其更接近 Linux 的结构。
  - **第二部分（30%）**：实现栈的自动增长机制。当栈增长超出当前分配的页面时，应触发缺页中断并分配新的页面(help-lab5)(Lab 5 Memory Management)。
- **加分项**：
  - **额外加分（5%）**：实现栈增长到堆的能力。如果无法实现，需详细解释原因并展示相关代码(Lab 5 Memory Management)。

# 第二版

### 1. **实验 3: System Call 实验**

#### 实验要求：

- **准备 xv6 环境**：

  - 你需要先在 Linux 虚拟机中安装 QEMU 模拟器，并下载 xv6 源代码。安装工具链（编译器、调试器等）后，确认环境正常运行。

  - 确认 QEMU 和 GCC 工具链正常工作的命令：

    ```
    bashCopy codegcc -m32 --version  # 检查 GCC 是否支持 32 位编译
    qemu-system-i386 --version  # 检查 QEMU 版本
    ```

- **添加系统调用**：

  - 你需要修改 `syscall.h`、`syscall.c`、`usys.S` 和 `sysproc.c` 文件，添加新的系统调用声明、实现和接口。
  - 在 `syscall.h` 中为系统调用定义一个新的编号，并在 `sysproc.c` 中编写对应的实现函数。
  - 为了测试系统调用，建议编写一个简单的用户程序来调用这个新的系统调用，以验证其功能是否正确。

#### 细节提示：

- **建议 1**：确保每个文件的修改都互相关联，特别是 `syscall.c` 和 `syscall.h` 的修改要与系统调用函数的实现保持一致。
- **建议 2**：在调试过程中，使用 GDB 调试 xv6 内核，可以通过 `make qemu-gdb` 启动内核，随后用 GDB 连接调试。
- **建议 3**：测试程序可以写成一个简单的 C 程序，调用你添加的系统调用，并打印返回值以确认是否正确。

#### 实验加分项：

- 如果你能够在 RISC-V 架构上完成实验，可以额外获得 10% 分数。可以通过以下命令在 RISC-V 模拟器中启动 xv6：

  ```
  bash
  
  
  Copy code
  qemu-system-riscv64 -kernel xv6/kernel  # 启动 RISC-V 模拟器
  ```

### 2. **实验 4: 调度器实验**

#### 实验要求：

- 实现优先级调度
  - 修改 `proc.c` 文件，添加进程的优先级字段（`int priority`），并确保调度器每次调度时选择优先级最高的进程。
  - 实现一个新的系统调用，用于动态调整进程优先级。系统调用可以接受一个进程 ID 和新的优先级作为参数。
  - 当某个进程的优先级降低时，如果存在更高优先级的进程，则调度器需要立即切换到该进程。

#### 细节提示：

- **建议 1**：你可以从修改 `scheduler()` 函数开始，这个函数决定了调度器如何选择进程。在选择进程时，遍历进程列表并选择优先级最高的进程。
- **建议 2**：确保系统调用能够正确修改进程优先级，修改后立即反映到调度器的行为中。
- **建议 3**：编写测试程序，通过创建多个进程并设置不同的优先级，验证调度器的行为。建议程序创建多个进程，让不同优先级的进程竞争 CPU。

#### 实验加分项：

- **加分项 1（5%）**：防止进程饥饿。如果进程等待时间过长，可以通过优先级老化机制增加其优先级，避免进程被长期忽略。
- **加分项 2（5%）**：实现优先级捐赠或优先级继承，避免优先级反转问题。例如，在锁竞争中，较高优先级的进程可以“捐赠”优先级给持有锁的较低优先级进程。
- **加分项 3（5%）**：添加追踪进程调度性能的字段，包括等待时间和周转时间，并在进程退出时输出这些值，或者通过系统调用获取。

#### 细节提示：

- **建议 4**：可以在 `proc.h` 中添加 `wait_time` 和 `turnaround_time` 字段，并在 `scheduler()` 函数中更新这些字段。
- **建议 5**：编写一个简单的脚本，用于生成不同优先级的进程，观察它们的执行顺序，并统计其等待时间和周转时间。

### 3. **实验 5: 内存管理实验**

#### 实验要求：

- **修改内存布局**：
  - 修改 `exec.c` 和 `vm.c`，将栈移到地址空间的顶部，并实现栈从高地址向低地址增长的机制。需要改变栈的初始化位置和增长策略。
  - 重新设计用户地址空间布局，使其符合 Linux 的结构：栈位于地址空间的顶部，并从高地址向下增长，而堆位于代码段之上向上增长。
- **实现栈的自动增长**：
  - 当栈访问超出已分配的页面时，捕获缺页中断，并动态分配新页面以支持栈的增长。你需要修改 `trap.c` 文件中的 `T_PGFLT` 中断处理程序，检测栈的非法访问并分配新的页面。

#### 细节提示：

- **建议 1**：在修改 `exec.c` 中的栈初始化代码时，可以参考 `allocuvm()` 函数如何分配内存页面，并修改其参数以将栈分配到高地址。
- **建议 2**：在 `trap.c` 中添加对 `T_PGFLT` 的处理，确保当栈访问未映射的页面时，能够分配新的页面。
- **建议 3**：为栈的增长设置一个限制条件，防止栈无意中侵占到堆区域。

#### 实验加分项：

- **加分项 1（5%）**：如果你能够实现栈成功增长到堆，并确保栈和堆在内存中相互不冲突，可以获得额外的加分。如果无法实现，需要详细解释为什么，并展示相关代码。

#### 细节提示：

- **建议 4**：在实现栈增长时，可以考虑使用一个缓冲区页（guard page），防止栈和堆直接相邻，从而避免栈无限制地增长到堆中。