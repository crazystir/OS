#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "spinlock.h"

extern char data[];  // defined in data.S

static pde_t *kpgdir;  // for use in scheduler()

struct {
  uint shmem_address[SHMEM_SIZE];
  uint shmem_count[SHMEM_SIZE];
  struct spinlock lock;
} shmems;


// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
void
kvmalloc(void)
{
  kpgdir = setupkvm();
}

// Set up CPU's kernel segment descriptors.
// Run once at boot time on each CPU.
void
seginit(void)
{
  struct cpu *c;

  // Map virtual addresses to linear addresses using identity map.
  // Cannot share a CODE descriptor for both kernel and user
  // because it would have to have DPL_USR, but the CPU forbids
  // an interrupt from CPL=0 to DPL=3.
  c = &cpus[cpunum()];
  c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
  c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
  c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
  c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);

  // Map cpu, and curproc
  c->gdt[SEG_KCPU] = SEG(STA_W, &c->cpu, 8, 0);

  lgdt(c->gdt, sizeof(c->gdt));
  loadgs(SEG_KCPU << 3);
  
  // Initialize cpu-local storage.
  cpu = c;
  proc = 0;
}

// Return the address of the PTE in page table pgdir
// that corresponds to linear address va.  If create!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int create)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)PTE_ADDR(*pde);
  } else {
    if(!create || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table 
    // entries, if necessary.
    *pde = PADDR(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// Create PTEs for linear addresses starting at la that refer to
// physical addresses starting at pa. la and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *la, uint size, uint pa, int perm)
{
  char *a, *last;
  pte_t *pte;
  
  a = PGROUNDDOWN(la);
  last = PGROUNDDOWN(la + size - 1);
  for(;;){
    pte = walkpgdir(pgdir, a, 1);
    if(pte == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// The mappings from logical to linear are one to one (i.e.,
// segmentation doesn't do anything).
// There is one page table per process, plus one that's used
// when a CPU is not running any process (kpgdir).
// A user process uses the same page table as the kernel; the
// page protection bits prevent it from using anything other
// than its memory.
// 
// setupkvm() and exec() set up every page table like this:
//   0..640K          : user memory (text, data, stack, heap)
//   640K..1M         : mapped direct (for IO space)
//   1M..end          : mapped direct (for the kernel's text and data)
//   end..PHYSTOP     : mapped direct (kernel heap and user pages)
//   0xfe000000..0    : mapped direct (devices such as ioapic)
//
// The kernel allocates memory for its heap and for user memory
// between kernend and the end of physical memory (PHYSTOP).
// The virtual address space of each user program includes the kernel
// (which is inaccessible in user mode).  The user program addresses
// range from 0 till 640KB (USERTOP), which where the I/O hole starts
// (both in physical memory and in the kernel's virtual address
// space).
static struct kmap {
  void *p;
  void *e;
  int perm;
} kmap[] = {
  {(void*)USERTOP,    (void*)0x100000, PTE_W},  // I/O space
  {(void*)0x100000,   data,            0    },  // kernel text, rodata
  {data,              (void*)PHYSTOP,  PTE_W},  // kernel data, memory
  {(void*)0xFE000000, 0,               PTE_W},  // device mappings
};

// Set up kernel part of a page table.
pde_t*
setupkvm(void)
{
  pde_t *pgdir;
  struct kmap *k;

  if((pgdir = (pde_t*)kalloc()) == 0)
    return 0;
  memset(pgdir, 0, PGSIZE);
  k = kmap;
  for(k = kmap; k < &kmap[NELEM(kmap)]; k++)
    if(mappages(pgdir, k->p, k->e - k->p, (uint)k->p, k->perm) < 0)
      return 0;

  return pgdir;
}

// Turn on paging.
void
vmenable(void)
{
  uint cr0;

  switchkvm(); // load kpgdir into cr3
  cr0 = rcr0();
  cr0 |= CR0_PG;
  lcr0(cr0);
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void
switchkvm(void)
{
  lcr3(PADDR(kpgdir));   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void
switchuvm(struct proc *p)
{
  pushcli();
  cpu->gdt[SEG_TSS] = SEG16(STS_T32A, &cpu->ts, sizeof(cpu->ts)-1, 0);
  cpu->gdt[SEG_TSS].s = 0;
  cpu->ts.ss0 = SEG_KDATA << 3;
  cpu->ts.esp0 = (uint)proc->kstack + KSTACKSIZE;
  ltr(SEG_TSS << 3);
  if(p->pgdir == 0)
    panic("switchuvm: no pgdir");
  lcr3(PADDR(p->pgdir));  // switch to new address space
  popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void
inituvm(pde_t *pgdir, char *init, uint sz)
{
  char *mem;
  
  if(sz >= PGSIZE)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pgdir, 0, PGSIZE, PADDR(mem), PTE_W|PTE_U);
  memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int
loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
  uint i, pa, n;
  pte_t *pte;

  if((uint)addr % PGSIZE != 0)
    panic("loaduvm: addr must be page aligned");
  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, addr+i, 0)) == 0)
      panic("loaduvm: address should exist");
    pa = PTE_ADDR(*pte);
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if(readi(ip, (char*)pa, offset+i, n) != n)
      return -1;
  }
  return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *mem;
  uint a;

  if(newsz > USERTOP)
    return 0;
  if(newsz < oldsz)
    return oldsz;

  a = PGROUNDUP(oldsz);
  for(; a < newsz; a += PGSIZE){
    mem = kalloc();
    if(mem == 0){
      cprintf("allocuvm out of memory\n");
      deallocuvm(pgdir, newsz, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    mappages(pgdir, (char*)a, PGSIZE, PADDR(mem), PTE_W|PTE_U);
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int
deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  pte_t *pte;
  uint a, pa;

  if(newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP(newsz);
  for(; a  < oldsz; a += PGSIZE){
    pte = walkpgdir(pgdir, (char*)a, 0);
    if(pte && (*pte & PTE_P) != 0){
      pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
      kfree((char*)pa);
      *pte = 0;
    }
  }
  return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pde_t *pgdir)
{
  uint i;

  if(pgdir == 0)
    panic("freevm: no pgdir");
  deallocuvm(pgdir, USERTOP, START_ADDRESS);
  for(i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTE_P)
      kfree((char*)PTE_ADDR(pgdir[i]));
  }
  kfree((char*)pgdir);
}

// Given a parent process's page table, create a copy
// of it for a child.
pde_t*
copyuvm(pde_t *pgdir, uint sz)
{
  pde_t *d;
  pte_t *pte;
  uint pa, i;
  char *mem;

  if((d = setupkvm()) == 0)
    return 0;
  for(i = START_ADDRESS; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, (void*)i, 0)) == 0)
      panic("copyuvm: pte should exist");
    if(!(*pte & PTE_P))
      panic("copyuvm: page not present");
    pa = PTE_ADDR(*pte);
    if((mem = kalloc()) == 0)
      goto bad;
    memmove(mem, (char*)pa, PGSIZE);
    if(mappages(d, (void*)i, PGSIZE, PADDR(mem), PTE_W|PTE_U) < 0)
      goto bad;
  }
  return d;

bad:
  freevm(d);
  return 0;
}

// Map user virtual address to kernel physical address.
char*
uva2ka(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if((*pte & PTE_P) == 0)
    return 0;
  if((*pte & PTE_U) == 0)
    return 0;
  return (char*)PTE_ADDR(*pte);
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
int
copyout(pde_t *pgdir, uint va, void *p, uint len)
{
  char *buf, *pa0;
  uint n, va0;
  
  buf = (char*)p;
  while(len > 0){
    va0 = (uint)PGROUNDDOWN(va);
    pa0 = uva2ka(pgdir, (char*)va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (va - va0);
    if(n > len)
      n = len;
    memmove(pa0 + (va - va0), buf, n);
    len -= n;
    buf += n;
    va = va0 + PGSIZE;
  }
  return 0;
}

int
forbid_zero_address(pde_t* pgdir) {
  if (mappages(pgdir,(char*)ZERO_ADDRESS, ZERO_LENGTH, ZERO_ADDRESS, 0) < 0) {
    return -1;
  }
  return 0;
}

void
shmem_init(void)
{
  initlock(&shmems.lock, "shmems");
  acquire(&shmems.lock);
  memset(shmems.shmem_address, NULL, SHMEM_SIZE * sizeof(*shmems.shmem_address));
  memset(shmems.shmem_count, 0, SHMEM_SIZE * sizeof(*shmems.shmem_count));
  release(&shmems.lock);
}

int
shmem_access(struct proc* p, int page_number) {
  int i;
  pte_t* pte = NULL;
  uint addr;

  //If page number is invalid, exit
  if (page_number < 0 || page_number >= SHMEM_SIZE) {
    return 0;
  }
  //If the process hold the corresponding virtual address,
  //return the virtual address
  if (p->shmem[page_number] != NULL) {
	return p->shmem[page_number];
  }

  acquire(&shmems.lock);
  //If the shared memory is not allocated, allocate memory for it
  if (shmems.shmem_address[page_number] == NULL) {
	if ((shmems.shmem_address[page_number] = (uint)kalloc()) == 0)
		goto bad;
  }

  //Shared memory will be mapped into the virtual address space
  //USERTOP - PGSIZE, USERTOP - 2 * PGSIZE, ... , USERTOP - SHMEM_SIZE * PGSIZE
  //Shared memory can be mapped to any one of them. If all of them are used as private memory, then the requirement fail
  //With a little modification, physical shared memory address can be mapped to any address in the virtual memory without losing performance.
  for (i = 1; i <= SHMEM_SIZE; i++) {
	addr = USERTOP - PGSIZE * i;
    pte = walkpgdir(p->pgdir, (void*)addr, 1);
	if (pte && !(*pte & PTE_P)) {
		if (mappages(p->pgdir, (void*)addr, PGSIZE, shmems.shmem_address[page_number], PTE_W | PTE_U) < 0)
			goto bad;
		p->shmem[page_number] = addr;
		break;
	}
  }
  if (i > SHMEM_SIZE)
	goto bad;
  release(&shmems.lock);
  shmem_new_process(page_number);
  return (int)addr;
bad:
  release(&shmems.lock);
  return 0;
}

int
shmem_count(int page_number) {
  int res = -1;

  if (page_number < 0 || page_number > SHMEM_SIZE)
	return -1;
  acquire(&shmems.lock);
  res = shmems.shmem_count[page_number];
  release(&shmems.lock);
  return res;
}

int
shmem_new_process(int page_number) {
  if (page_number < 0 || page_number > SHMEM_SIZE)
	return -1;
  acquire(&shmems.lock);
  shmems.shmem_count[page_number]++;
  release(&shmems.lock);
  return 0;
}

//One process exits, if no more processes hold this shared memory,
//it should be freed
//This should be automic operation
int
shmem_exit_process(int page_number) {
  if (page_number < 0 || page_number > SHMEM_SIZE)
	return -1;
  acquire(&shmems.lock);
  //If no process hold the shared memory,
  //memory should be freed
  if (--shmems.shmem_count[page_number] <= 0) {
	kfree((char*)shmems.shmem_address[page_number]);
	shmems.shmem_address[page_number] = NULL;
  }
  release(&shmems.lock);
  return 0;
}

//Free the shared memory virtual address in the process
//If no process hold that shared memory, free that shared memory as well
//Must be used before freevm
void
free_process_shmem(struct proc* p) {
  uint i;
  pte_t* pte = NULL;
  pde_t* pgdir = p->pgdir;

  for (i = 0; i < SHMEM_SIZE; i++) {
	if (p->shmem[i] != NULL) {
	  pte = walkpgdir(pgdir, (char*)p->shmem[i], 0);
	  //Set the pte to be zero so that the freevm function will not
	  //free shared memory automatically
	  *pte = 0;
	  p->shmem[i] = NULL;
	  shmem_exit_process(i);
	}
  }
}
//Copy the shared memory virtual address pages from parent process to new process
void
copy_shmem(struct proc* np, struct proc* p) {
  int i;

  for(i = 0; i < SHMEM_SIZE; i++) {
	np->shmem[i] = p->shmem[i];
	if (np->shmem[i] != NULL) {
		shmem_new_process(i);
		if (mappages(np->pgdir, (void*)np->shmem[i], PGSIZE,shmems.shmem_address[i],PTE_U | PTE_W) < 0)
		  panic("copy shmem");
	}
  }
}
