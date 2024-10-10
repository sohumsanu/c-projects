#include "kernel.hh"
#include "k-apic.hh"
#include "k-vmiter.hh"
#include <atomic>

// kernel.cc
//
//    This is the kernel.


// INITIAL PHYSICAL MEMORY LAYOUT
//
//  +-------------- Base Memory --------------+
//  v                                         v
// +-----+--------------------+----------------+--------------------+---------/
// |     | Kernel      Kernel |       :    I/O | App 1        App 1 | App 2
// |     | Code + Data  Stack |  ...  : Memory | Code + Data  Stack | Code ...
// +-----+--------------------+----------------+--------------------+---------/
// 0  0x40000              0x80000 0xA0000 0x100000             0x140000
//                                             ^
//                                             | \___ PROC_SIZE ___/
//                                      PROC_START_ADDR

#define PROC_SIZE 0x40000       // initial state only

proc ptable[NPROC];             // array of process descriptors
                                // Note that `ptable[0]` is never used.
proc* current;                  // pointer to currently executing proc

#define HZ 100                  // timer interrupt frequency (interrupts/sec)
static std::atomic<unsigned long> ticks; // # timer interrupts so far


// Memory state
//    Information about physical page with address `pa` is stored in
//    `pages[pa / PAGESIZE]`. In the handout code, each `pages` entry
//    holds an `refcount` member, which is 0 for free pages.
//    You can change this as you see fit.

pageinfo pages[NPAGES];


[[noreturn]] void schedule();
[[noreturn]] void run(proc* p);
void exception(regstate* regs);
uintptr_t syscall(regstate* regs);
void memshow();


// kernel(command)
//    Initialize the hardware and processes and start running. The `command`
//    string is an optional string passed from the boot loader.

static void process_setup(pid_t pid, const char* program_name);

//This function is meant to be a helper to free the allocated memory from the page tables
void free_function(x86_64_pagetable* pt){
    for (vmiter iter(pt); iter.va() < MEMSIZE_VIRTUAL; iter+= 4096){
        if(iter.user() && iter.pa() != CONSOLE_ADDR){
            kfree((void*) iter.pa());
        }
    }
    for (ptiter iter2(pt); iter2.active(); iter2.next()){
        kfree((void*) iter2.pa());
    }
    kfree((void*) pt);
}

void kernel(const char* command) {
    // Initialize hardware.
    init_hardware();
    log_printf("Starting WeensyOS\n");

    // Initialize timer interrupt.
    ticks = 1;
    init_timer(HZ);

    // Clear screen.
    console_clear();
 
    // (re-)Initialize the kernel page table.
    for (vmiter it(kernel_pagetable); it.va() < MEMSIZE_PHYSICAL; it += PAGESIZE) {
         if (it.va() != 0) {
            if(it.va() <= KERNEL_STACK_TOP){
                it.map(it.va(), PTE_P | PTE_W);
            } else if (it.va() == CONSOLE_ADDR) {
                it.map(it.va(), PTE_P | PTE_W | PTE_U);
            } else if (it.va() >= PROC_START_ADDR) {
                it.map(it.va(), PTE_P | PTE_W | PTE_U);
            }
            else {
                it.map(it.va(), PTE_P | PTE_W);
            }
        } else {
            // nullptr is inaccessible even to the kernel
            it.map(it.va(), 0);
        }
    }

    // Set up process descriptors.
    for (pid_t i = 0; i < NPROC; i++) {
        ptable[i].pid = i;
        ptable[i].state = P_FREE;
    }
    if (command && program_loader(command).present()) {
        process_setup(1, command);
    } else {
        process_setup(1, "allocator");
        process_setup(2, "allocator2");
        process_setup(3, "allocator3");
        process_setup(4, "allocator4");
    }

    // Switch to the first process using run().
    run(&ptable[1]);
}


// kalloc(sz)
//    Kernel memory allocator. Allocates `sz` contiguous bytes and
//    returns a pointer to the allocated memory (the physical address of
//    the newly allocated memory), or `nullptr` on failure.
//
//    The returned memory is initialized to 0xCC, which corresponds to
//    the x86 instruction `int3` (this may help you debug). You can
//    reset it to something more useful.
//
//    On WeensyOS, `kalloc` is a page-based allocator: if `sz > PAGESIZE`
//    the allocation fails; if `sz < PAGESIZE` it allocates a whole page
//    anyway.
//
//    The stencil code returns the next allocatable free page it can find,
//    but it never reuses pages or supports freeing memory (you'll have to
//    change this at some point).

static uintptr_t next_alloc_pa;

void* kalloc(size_t sz) {
    if (sz > PAGESIZE) {
        return nullptr;
    }

    for(unsigned long i = 0; i<MEMSIZE_PHYSICAL/PAGESIZE; i++){
        if(next_alloc_pa == MEMSIZE_PHYSICAL){
            next_alloc_pa = 0;
        }
        uintptr_t pa = next_alloc_pa;
        next_alloc_pa += PAGESIZE;

        if (allocatable_physical_address(pa)
            && !pages[pa / PAGESIZE].used()) {
            pages[pa / PAGESIZE].refcount = 1;
            memset((void*) pa, 0, PAGESIZE);
            return (void*) pa;
        }
    }

    // while (next_alloc_pa < MEMSIZE_PHYSICAL) {
    //     uintptr_t pa = next_alloc_pa;
    //     next_alloc_pa += PAGESIZE;

    //     if (allocatable_physical_address(pa)
    //         && !pages[pa / PAGESIZE].used()) {
    //         pages[pa / PAGESIZE].refcount = 1;
    //         memset((void*) pa, 0, PAGESIZE);
    //         return (void*) pa;
    //     }
    // }
    return nullptr;
}


// kfree(kptr)
//    Frees `kptr`, which must have been previously returned by `kalloc`.
//    If `kptr == nullptr` does nothing.

void kfree(void* kptr) {
    if (kptr == nullptr){
        return;
    } 

    size_t page_index = (uintptr_t) kptr / PAGESIZE;
    pageinfo* page = &pages[page_index];
    page->refcount--;
    if(page->refcount == 0 && (uintptr_t) kptr < next_alloc_pa){
        next_alloc_pa = (uintptr_t) kptr;
    }
}


// process_setup(pid, program_name)
//    Loads application program `program_name` as process number `pid`.
//    This loads the application's code and data into memory, sets its
//    %rip and %rsp, gives it a stack page, and marks it as runnable.

void process_setup(pid_t pid, const char* program_name) {
    init_process(&ptable[pid], 0);

    if(pid == 0) {
        ptable[pid].pagetable = kernel_pagetable;
    }
    else {
        ptable[pid].pagetable = (x86_64_pagetable*) kalloc(PAGESIZE);
    memset((void*) ptable[pid].pagetable, 0, PAGESIZE);
        for(vmiter it(kernel_pagetable, 0); it.va() < PROC_START_ADDR; it += PAGESIZE) { // Why can't we do it.next() ?
            int x;
            x = vmiter(ptable[pid].pagetable, it.va()).try_map(it.pa(), it.perm());
            if(x == -1){
                return;
            }
        // log_printf("VA %p maps to PA %p with PERMS %p, %p, %p\n", it.va(), it.pa(), it.present() ? PTE_P : 0x0, it.writable() ? PTE_W : 0x0, it.user() ? PTE_U : 0x0);
        }
    }

    // Initialize this process's page table. Notice how we are currently
    // sharing the kernel's page table.
    x86_64_pagetable* newtable = (x86_64_pagetable*) kalloc(0);
    if(!newtable){
        free_function(newtable);
    }
    memset(newtable, 0, sizeof(x86_64_pagetable));
   
    vmiter copy(newtable, 0);
    for (vmiter it(kernel_pagetable); it.va() < PROC_START_ADDR; it += PAGESIZE) {
        if(copy.try_map(it.pa(), it.perm()) == -1){
            //Failed to map a kernel page to the new page table -> need to free the data
            free_function(newtable);
        }
        copy+=PAGESIZE;
    }

    ptable[pid].pagetable = newtable;
    // Initialize `program_loader`.
    // The `program_loader` is an iterator that visits segments of executables.
    program_loader loader(program_name);

    // Using the loader, we're going to start loading segments of the program binary into memory
    // (recall that an executable has code/text segment, data segment, etc).

    // First, for each segment of the program, we allocate page(s) of memory.
    for (loader.reset(); loader.present(); ++loader) {
        for (uintptr_t a = round_down(loader.va(), PAGESIZE);
             a < loader.va() + loader.size();
             a += PAGESIZE) {
            // `a` is the virtual address of the current segment's page.
            // assert(!pages[a / PAGESIZE].used());
            void* virtualaddress = kalloc(0);
            if(!virtualaddress){
                free_function(newtable);
                kfree(virtualaddress);
                return;
            }
            vmiter new_it(ptable[pid].pagetable, a);
            if(loader.writable()){
                new_it.try_map(virtualaddress, PTE_P | PTE_W | PTE_U);
            } else {
                new_it.try_map(virtualaddress, PTE_P | PTE_U);
            }
            // Read the description on the `pages` array if you're confused about what it is.
            // Here, we're directly getting the page that has the same physical address as the
            // virtual address `a`, and claiming that page by incrementing its reference count
            // (you will have to change this later).
            // pages[a / PAGESIZE].refcount = 1;
        }
    }

    // We now copy instructions and data into memory that we just allocated.
    for (loader.reset(); loader.present(); ++loader) {
        vmiter iter(newtable, loader.va());
        memset((void*) iter.pa(), 0, loader.size());
        memcpy((void*) iter.pa(), loader.data(), loader.data_size());
    }

    // Set %rip and mark the entry point of the code.
    ptable[pid].regs.reg_rip = loader.entry();
    void* another_kalloc = kalloc(0);
    if(!another_kalloc){
        free_function(newtable);
        kfree(another_kalloc);
        return;
    }

    // We also need to allocate a page for the stack.
    uintptr_t stack_addr = MEMSIZE_VIRTUAL-PAGESIZE;
    ptable[pid].regs.reg_rsp = MEMSIZE_VIRTUAL;
    ptable[pid].state = P_RUNNABLE;
    // assert(!pages[stack_addr / PAGESIZE].used());
    // Again, we're using the physical page that has the same address as the `stack_addr` to
    // maintain the one-to-one mapping between physical and virtual memory (you will have to change
    // this later).
    // pages[stack_addr / PAGESIZE].refcount = 1;
    // Set %rsp to the start of the stack.

    // Finally, mark the process as runnable.
    vmiter new_it(ptable[pid].pagetable, stack_addr);
    int doublecheck = new_it.try_map(another_kalloc, PTE_P | PTE_W | PTE_U);
    if(doublecheck == -1){
        free_function(newtable);
        kfree(another_kalloc);
        return;
    }
    
}



// exception(regs)
//    Exception handler (for interrupts, traps, and faults).
//    You should *not* have to edit this function.
//
//    The register values from exception time are stored in `regs`.
//    The processor responds to an exception by saving application state on
//    the kernel's stack, then jumping to kernel assembly code (see
//    k-exception.S). That code saves more registers on the kernel's stack,
//    then calls exception(). This way, the process can be resumed right where
//    it left off before the exception. The pushed registers are popped and
//    restored before returning to the process (see k-exception.S).
//
//    Note that hardware interrupts are disabled when the kernel is running.

void exception(regstate* regs) {
    // Copy the saved registers into the `current` process descriptor.
    current->regs = *regs;
    regs = &current->regs;

    // It can be useful to log events using `log_printf`.
    // Events logged this way are stored in the host's `log.txt` file.
    /* log_printf("proc %d: exception %d at rip %p\n",
                current->pid, regs->reg_intno, regs->reg_rip); */

    // Show the current cursor location and memory state (unless this is a kernel fault).
    console_show_cursor(cursorpos);
    if (regs->reg_intno != INT_PF || (regs->reg_errcode & PFERR_USER)) {
        memshow();
    }

    // If Control-C was typed, exit the virtual machine.
    check_keyboard();


    // Actually handle the exception.
    switch (regs->reg_intno) {

    case INT_IRQ + IRQ_TIMER:
        ++ticks;
        lapicstate::get().ack();
        schedule();
        break;                  /* will not be reached */

    case INT_PF: {
        // Analyze faulting address and access type.
        uintptr_t addr = rdcr2();
        const char* operation = regs->reg_errcode & PFERR_WRITE
                ? "write" : "read";
        const char* problem = regs->reg_errcode & PFERR_PRESENT
                ? "protection problem" : "missing page";

        if (!(regs->reg_errcode & PFERR_USER)) {
            panic("Kernel page fault for %p (%s %s, rip=%p)!\n",
                  addr, operation, problem, regs->reg_rip);
        }
        console_printf(CPOS(24, 0), 0x0C00,
                       "Process %d page fault for %p (%s %s, rip=%p)!\n",
                       current->pid, addr, operation, problem, regs->reg_rip);
        current->state = P_BROKEN;
        break;
    }

    default:
        panic("Unexpected exception %d!\n", regs->reg_intno);

    }

    // Return to the current process (or run something else).
    if (current->state == P_RUNNABLE) {
        run(current);
    } else {
        schedule();
    }
}


// syscall(regs)
//    System call handler.
//
//    The register values from system call time are stored in `regs`.
//    The return value, if any, is returned to the user process in `%rax`.
//
//    Note that hardware interrupts are disabled when the kernel is running.

// Headers for helper functions used by syscall.
int syscall_page_alloc(uintptr_t addr);
pid_t syscall_fork();
void syscall_exit();

uintptr_t syscall(regstate* regs) {
    // Copy the saved registers into the `current` process descriptor.
    current->regs = *regs;
    regs = &current->regs;

    // It can be useful to log events using `log_printf`.
    // Events logged this way are stored in the host's `log.txt` file.
    /* log_printf("proc %d: syscall %d at rip %p\n",
                  current->pid, regs->reg_rax, regs->reg_rip); */

    // Show the current cursor location and memory state (unless this is a kernel fault).
    console_show_cursor(cursorpos);
    memshow();

    // If Control-C was typed, exit the virtual machine.
    check_keyboard();

    // Actually handle the exception.
    switch (regs->reg_rax) {

    case SYSCALL_PANIC:
        panic(nullptr); // does not return

    case SYSCALL_GETPID:
        return current->pid;

    case SYSCALL_YIELD:
        current->regs.reg_rax = 0;
        schedule(); // does not return

    case SYSCALL_PAGE_ALLOC:
        return syscall_page_alloc(current->regs.reg_rdi);

    case SYSCALL_FORK:
        return syscall_fork();

    case SYSCALL_EXIT:
        syscall_exit();
        schedule(); // does not return

    default:
        panic("Unexpected system call %ld!\n", regs->reg_rax);

    }

    panic("Should not get here!\n");
}


// syscall_page_alloc(addr)
//    Helper function that handles the SYSCALL_PAGE_ALLOC system call.
//    This function implement the specification for `sys_page_alloc`
//    in `u-lib.hh` (but in the stencil code, it does not - you will
//    have to change this).

int syscall_page_alloc(uintptr_t addr) {
    // assert(!pages[addr / PAGESIZE].used());
    // Currently we're simply using the physical page that has the same address
    // as `addr` (which is a virtual address).
    if(!addr){
        return -1;
    }
    if(addr < PROC_START_ADDR || addr >= MEMSIZE_VIRTUAL || addr % PAGESIZE){
        //This means that the address is outside of the range.
        return -1;
    }
    x86_64_pagetable* virtualaddress = (x86_64_pagetable*) kalloc(0);
    if(!virtualaddress){
        kfree(virtualaddress);
        return -1;
    } else {
        vmiter iter(ptable[current->pid].pagetable, addr);
        memset(virtualaddress, 0, PAGESIZE);
        int mapresult = iter.try_map(virtualaddress, PTE_P | PTE_W | PTE_U);
        if(mapresult == -1){
            free_function(virtualaddress);
            return -1;
        }
    }
return 0;
}

// syscall_fork()
//    Handles the SYSCALL_FORK system call. This function
//    implements the specification for `sys_fork` in `u-lib.hh`.
pid_t syscall_fork() {
    // Implement for Step 5!
    int x = -1; //initial value should be set to -1
    for(int y = 1; y < NPROC; y++){
        if(ptable[y].state == P_FREE){
            x = y;
            break;
        }
    }
    if(x == -1){ // if there are no slots found
        return x;
    }
    x86_64_pagetable* address = (x86_64_pagetable*) kalloc(0);
    if(!address){
        kfree(address);
        return -1;
    }
    memset(address, 0, PAGESIZE);
    x86_64_pagetable* otherpt = address;

    for(vmiter curr(current->pagetable); curr.va() < MEMSIZE_VIRTUAL; curr += 4096){
        vmiter newpt(otherpt, curr.va());
        if(!(curr.va() < PROC_START_ADDR)){
            if(curr.user()){
                if(!curr.writable()){
                    int useful = newpt.try_map(curr.pa(), curr.perm());
                    pages[curr.pa() / PAGESIZE].refcount++;
                    if(useful == -1){
                        //free & call helper
                        free_function(otherpt);
                        return useful;
                    }
                }
                // if(curr.writable()){
                //     ptable[x].pagetable = otherpt;
                //     ptable[x].pid = x;
                //     ptable[x].regs = current->regs;
                //     ptable[x].pagetable = otherpt;
                //     ptable[x].state = P_RUNNABLE;
                //     ptable[x].regs.reg_rax = 0;
                //     return x;
                // } else {
                //     int useful = newpt.try_map(curr.pa(), curr.perm());
                //     pages[curr.pa() / PAGESIZE].refcount +=1;
                //     if(useful == -1){
                //         //free & call helper
                //         return useful;
                //     }
                // }
             else {
                void* address_child = kalloc(0); // This will be the new physical address -> newly allocated
                if(!address_child){
                    free_function(otherpt);
                    kfree(address_child);
                    return -1;
                }
                memcpy(address_child, (void*)curr.pa(), 4096);
                int check_again = newpt.try_map(address_child, curr.perm()); // must use try map
                if(check_again == -1){ // Need to check whether this will error every time
                    //Free & help
                    free_function(otherpt);
                    kfree(address_child);
                    return check_again;
                }
            }
        }
    } else {
        int zed = newpt.try_map(curr.pa(), curr.perm());
        if(zed == -1){
            free_function(otherpt);
            return zed;
        }
    }

}
    ptable[x].regs = current->regs;
    ptable[x].state = P_RUNNABLE;
    ptable[x].regs.reg_rax = 0;
    ptable[x].pagetable = otherpt;
    ptable[x].pid = x;
    return x;
}

// syscall_exit()
//    Handles the SYSCALL_EXIT system call. This function
//    implements the specification for `sys_exit` in `u-lib.hh`.
void syscall_exit() {
    // Implement for Step 7!
    for(vmiter iter(current->pagetable, 0); iter.va() < MEMSIZE_VIRTUAL; iter += 4096){
        if(iter.pa() == CONSOLE_ADDR || (!iter.user())){
            continue;
        } 
        kfree((void*) iter.pa());
    }
    for (ptiter iter2(current->pagetable); iter2.active(); iter2.next()){
        kfree((void*) iter2.pa());
    }
    ptable[current->pid].state = P_FREE;
    kfree(current->pagetable);
}

// schedule
//    Picks the next process to run and then run it.
//    If there are no runnable processes, spins forever.
//    You should *not* have to edit this function.

void schedule() {
    pid_t pid = current->pid;
    for (unsigned spins = 1; true; ++spins) {
        pid = (pid + 1) % NPROC;
        if (ptable[pid].state == P_RUNNABLE) {
            run(&ptable[pid]);
        }

        // If Control-C was typed, exit the virtual machine.
        check_keyboard();

        // If spinning forever, show the memviewer.
        if (spins % (1 << 12) == 0) {
            memshow();
            log_printf("%u\n", spins);
        }
    }
}


// run(p)
//    Runs process `p`. This involves setting `current = p` and calling
//    `exception_return` to restore its page table and registers.
//    You should *not* have to edit this function.

void run(proc* p) {
    assert(p->state == P_RUNNABLE);
    current = p;

    // Check the process's current pagetable.
    check_pagetable(p->pagetable);

    // This function is defined in k-exception.S. It restores the process's
    // registers then jumps back to user mode.
    exception_return(p);

    // should never get here
    while (true) {
    }
}


// memshow()
//    Draws a picture of memory (physical and virtual) on the CGA console.
//    Switches to a new process's virtual memory map every 0.25 sec.
//    Uses `console_memviewer()`, a function defined in `k-memviewer.cc`.
//    You should *not* have to edit this function.

void memshow() {
    static unsigned last_ticks = 0;
    static int showing = 0;

    // switch to a new process every 0.25 sec
    if (last_ticks == 0 || ticks - last_ticks >= HZ / 2) {
        last_ticks = ticks;
        showing = (showing + 1) % NPROC;
    }

    proc* p = nullptr;
    for (int search = 0; !p && search < NPROC; ++search) {
        if (ptable[showing].state != P_FREE
            && ptable[showing].pagetable) {
            p = &ptable[showing];
        } else {
            showing = (showing + 1) % NPROC;
        }
    }

    extern void console_memviewer(proc* vmp);
    console_memviewer(p);
}
