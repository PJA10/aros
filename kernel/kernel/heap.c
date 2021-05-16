#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <kernel/heap.h>
#include <arch-i386/paging.h>
#include <kernel/pmm.h>

static void *(*curr_kmalloc_func)(uint32_t sz, int align, uint32_t *phys);
static void (*curr_kfree_func)(void *addr);

void *kmalloc_internal(uint32_t sz, int align, uint32_t *phys) {
	//printf("in kmalloc_internal\n");
        return (*curr_kmalloc_func)(sz, align, phys);
}

void *kmalloc_a(uint32_t sz) { // page aligned.
	return kmalloc_internal(sz, true, NULL);
}

void *kmalloc_p(uint32_t sz, uint32_t *phys) { // returns a physical address.
	return kmalloc_internal(sz, false, phys);
}

void *kmalloc_ap(uint32_t sz, uint32_t *phys) { // page aligned and returns a physical address.
	return kmalloc_internal(sz, true, phys);
}

void *kmalloc(uint32_t sz) { // vanilla (normal).
	//printf("in kmalloc\n");
	return kmalloc_internal(sz, false, NULL);
}

void kfree(void *addr) {
	curr_kfree_func(addr);
}

void set_kmalloc_function(void *(*func_p)(uint32_t , int , uint32_t *)) {
	curr_kmalloc_func = func_p;
}

void set_kfree( void (*func_p)(void *addr)) {
	curr_kfree_func = func_p;
}


#define NALLOC 1024 // minimum #units morecore can request

typedef union header { // block header
	struct {
		union header *ptr; // next block if on free list
		uint32_t size; // size of this block
	} s;
	max_align_t x; // force alignment of blocks
} Header;

static Header base; // empry list to get started
static Header *freep = NULL; // start of free list
static uint32_t end_of_kernel_heap; // address of the end of the heap

static Header *morecore(uint32_t nu);
static void *adv_kmalloc(uint32_t nbytes, int align, uint32_t *phys);
static void adv_kfree(void *ap);

/*
 * this function ask the system for more memoty.
 * TODO: add a max to the kernel heap, and change the heap location to 0xd0000000-0xe0000000
 */
static Header *morecore(uint32_t nu) {
	uint32_t cp;
	Header *up;
	if (nu < NALLOC)
		nu = NALLOC;
	cp = end_of_kernel_heap;
	end_of_kernel_heap += nu * sizeof(Header);
	if (end_of_kernel_heap/PAGE_SIZE > cp/PAGE_SIZE) {
		uint32_t curr_page = cp + PAGE_SIZE;
		// alocate all the not present pages in the new allocated memory
		for (; end_of_kernel_heap/PAGE_SIZE >= curr_page/PAGE_SIZE; curr_page += PAGE_SIZE) {
			if (vmm_allocate_page(curr_page, 0x3) == NULL) {
				return NULL; // no memory?
			}
		}
	}
	up = (Header *) cp;
	up->s.size = nu;
	adv_kfree((void *)(up+1));
	return freep;
}

/*
 * this function is the kernel general-purposr storage allocator
 */
static void *adv_kmalloc(uint32_t nbytes, int align, uint32_t *phys) {
	Header *p, *prevp;
	uint32_t nunits;

	nunits = (nbytes + sizeof(Header)-1) / sizeof(Header) + 1;
	if ((prevp = freep) == NULL) { // no free list yet
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
		end_of_kernel_heap = KERNEL_HEAP_END + KERNEL_VIRTUAL_BASE; // run time value (cant be defined in global)
		if (vmm_allocate_page(end_of_kernel_heap, 0x3) == NULL) { // allocate the first page
			return NULL;
		}
	}
	for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
		if (p->s.size >= nunits) { // big enough
			if (p->s.size == nunits) { // exacly
				prevp->s.ptr = p->s.ptr;
			} else { // alocate tail end
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			freep = prevp;
			return (void *)(p+1);
		}
		if (p == freep) { // wrapped around free list
			if ((p = morecore(nunits)) == NULL) {
				return NULL; // no memory?
			}
		}
	}
}

/*
 * this function puts a given block in the free list
 */
static void adv_kfree(void *ap) {
	if (ap == NULL) {
		printf("kfree was called with null\n");
		return;
	}
	Header *bp, *p;

	bp = (Header *) ap - 1; // point to block header
	for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
		if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
			break; // freed block at start or end of area
	}

	if ( bp + bp->s.size == p->s.ptr) { // join the upper nbr
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	} else {
		bp->s.ptr = p->s.ptr;
	}
	if ( p + p->s.size == bp) { // join the lower nbr
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	} else {
		p->s.ptr = bp;
	}
	freep = p;
}

void adv_kmalloc_init() {
	set_kmalloc_function(adv_kmalloc);
	set_kfree(adv_kfree);
}
