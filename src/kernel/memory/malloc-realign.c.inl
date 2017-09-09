/* Copyright (c) 2017 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "malloc.c"
#endif

DECL_BEGIN
#ifdef CONFIG_TRACE_LEAKS
#ifdef HEAP_REALIGN
PRIVATE SAFE struct mptr *KCALL
mheap_realign(struct dsetup *setup, struct mheap *__restrict self,
              struct mptr *ptr, size_t alignment, size_t new_size, gfp_t flags)
#else
PRIVATE SAFE struct mptr *KCALL
mheap_realloc(struct dsetup *setup, struct mheap *__restrict self,
              struct mptr *ptr, size_t new_size, gfp_t flags)
#endif
#else
#ifdef HEAP_REALIGN
PRIVATE SAFE struct mptr *KCALL
mheap_realign(struct mheap *__restrict self, struct mptr *ptr,
              size_t alignment, size_t new_size, gfp_t flags)
#else
PRIVATE SAFE struct mptr *KCALL
mheap_realloc(struct mheap *__restrict self, struct mptr *ptr,
              size_t new_size, gfp_t flags)
#endif
#endif
{
 struct mptr *new_ptr;
#ifdef MPTR_HAVE_TAIL
 size_t old_usersize,new_usersize;
#endif
 MALIGNED size_t old_size = MPTR_SIZE(ptr);
 assert(old_size >= MPTR_SIZEOF(0));
 assert(IS_ALIGNED(old_size,HEAP_ALIGNMENT));
 assert((flags&GFP_MASK_MPTR) == MPTR_FLAGS(ptr));
#ifdef MPTR_HAVE_TAIL
 old_usersize = MPTR_SIZEOF(MPTR_USERSIZE(ptr));
 new_usersize = MPTR_SIZEOF(new_size);
 new_size = CEIL_ALIGN(new_usersize,HEAP_ALIGNMENT);
#else
 new_size = CEIL_ALIGN(MPTR_SIZEOF(new_size),HEAP_ALIGNMENT);
#endif
 if (new_size > old_size) {
  MALIGNED size_t missing_size = new_size-old_size;
  MALIGNED size_t alloc_size;
  /* Allocate more memory above. */
  mheap_write(self);
  if likely(mheap_acquire_at(self,(void *)((uintptr_t)ptr+old_size),
                             missing_size,&alloc_size,flags) != PAGE_ERROR) {
   mheap_endwrite(self);
#ifdef MALLOC_DEBUG_API
   mptr_unlink(setup,ptr);
#endif /* MALLOC_DEBUG_API */
   /* Update the size pointer within the allocated header. */
   ptr->m_size += alloc_size;
#ifdef MALLOC_DEBUG_API
   assert(MPTR_SIZE(ptr) == old_size+alloc_size);
   mptr_mvtail(ptr,old_size,old_usersize,
               old_size+alloc_size,
               new_usersize,flags);
   mptr_relink(setup,ptr);
#endif /* MALLOC_DEBUG_API */
  } else {
   if (flags&GFP_NOMOVE) {
    mheap_endwrite(self);
    return MPTR_OF(NULL);
   }
#ifndef HEAP_REALIGN /* TODO: Ensure proper alignment below. */
   /* Try to allocate memory directly below. */
   new_ptr = (struct mptr *)mheap_acquire_at(self,(void *)((uintptr_t)ptr-missing_size),
                                             missing_size,&alloc_size,flags&~GFP_CALLOC);
   if likely(new_ptr != PAGE_ERROR) {
    mheap_endwrite(self);
#ifdef MALLOC_DEBUG_API
    mptr_unlink(setup,ptr);
#endif /* MALLOC_DEBUG_API */
    assert(missing_size == alloc_size);
    memmove(new_ptr,ptr,old_size);
    if (flags&GFP_CALLOC) memset((void *)((uintptr_t)new_ptr+old_size),0,alloc_size);
    assert(MPTR_SIZE(new_ptr) == old_size);
#ifndef MPTR_HAVE_TYPE
    assert(!(alloc_size&GFP_MASK_MPTR));
#endif
    new_ptr->m_size += alloc_size;
#ifdef MALLOC_DEBUG_API
#ifdef MPTR_HAVE_TAIL
    *(uintptr_t *)&new_ptr->m_tail -= (uintptr_t)ptr-(uintptr_t)new_ptr;
#endif
    assert(MPTR_SIZE(new_ptr) == old_size+alloc_size);
    mptr_mvtail(new_ptr,old_size,old_usersize,
                old_size+alloc_size,new_usersize,flags);
    mptr_relink(setup,new_ptr);
#endif /* MALLOC_DEBUG_API */
    return new_ptr;
   }
#endif
   /* Allocate totally new block. */
#ifdef HEAP_REALIGN
   new_ptr = (struct mptr *)mheap_acquire_al(self,alignment,sizeof(struct mptr),
                                             new_size,&alloc_size,flags&~GFP_CALLOC);
#else
   new_ptr = (struct mptr *)mheap_acquire(self,new_size,&alloc_size,
                                          flags&~GFP_CALLOC);
#endif
   mheap_endwrite(self);
   if (new_ptr == PAGE_ERROR) return MPTR_OF(NULL);
   assert(alloc_size >= new_size);
   /* Setup the new pointer, and reset the old. */
   memcpy(new_ptr,ptr,old_size);
#ifdef MALLOC_DEBUG_API
#ifdef MPTR_HAVE_TAIL
   *(uintptr_t *)&new_ptr->m_tail += (uintptr_t)new_ptr-(uintptr_t)ptr;
#endif
#ifndef MPTR_HAVE_TYPE
   MPTR_SETUP(new_ptr,flags&GFP_MASK_MPTR,alloc_size);
#else
   new_ptr->m_size = alloc_size;
#endif
   if (flags&GFP_CALLOC) {
    memset((void *)((uintptr_t)new_ptr+old_size),0,
           alloc_size-old_size);
   }
   mptr_unlink(setup,ptr);
   assert(MPTR_SIZE(new_ptr) == alloc_size);
   mptr_mvtail(new_ptr,old_size,old_usersize,
               alloc_size,new_usersize,flags);
   mptr_relink(setup,new_ptr);
#endif
   mheap_resetdebug(ptr,old_size,flags&~GFP_CALLOC);
   /* Free the old pointer. */
   mheap_write(self);
   asserte(mheap_release(self,ptr,old_size,flags&~GFP_CALLOC));
   mheap_endwrite(self);
#ifndef MALLOC_DEBUG_API
#ifndef MPTR_HAVE_TYPE
   MPTR_SETUP(new_ptr,flags&GFP_MASK_MPTR,alloc_size);
#else
   new_ptr->m_size = alloc_size;
#endif
   if (flags&GFP_CALLOC) {
    memset((void *)((uintptr_t)new_ptr+old_size),0,
           alloc_size-old_size);
   }
#endif /* !MALLOC_DEBUG_API */
   return new_ptr;
  }
 } else if (new_size < old_size) {
  MALIGNED uintptr_t free_begin = (uintptr_t)ptr+new_size;
  MALIGNED size_t free_size = old_size-new_size;
  assert(IS_ALIGNED(new_size,HEAP_ALIGNMENT));
  assert(IS_ALIGNED(free_size,HEAP_ALIGNMENT));
  /* Actually free the old data. */
#ifdef MALLOC_DEBUG_API
  { bool free_ok;
    mptr_unlink(setup,ptr);
#ifndef MPTR_HAVE_TYPE
    assert(!(free_size&GFP_MASK_MPTR));
#endif
    ptr->m_size -= free_size;
    assert(MPTR_SIZE(ptr) == new_size);
    mptr_mvtail(ptr,old_size,old_usersize,
                    new_size,new_usersize,
                flags);
    mheap_write(self);
    assert(IS_ALIGNED(free_begin,HEAP_ALIGNMENT));
    assert(IS_ALIGNED(free_size,HEAP_ALIGNMENT));
    free_ok = mheap_release(self,(void *)free_begin,
                            free_size,flags&~GFP_CALLOC);
    mheap_endwrite(self);

    if (!free_ok) {
     ptr->m_size += free_size;
     assert(MPTR_SIZE(ptr) == old_size);
     mptr_mvtail(ptr,new_size,new_usersize,
                     old_size,new_usersize,
                 flags);
    }
    mptr_relink(setup,ptr);
  }
#else
  atomic_rwlock_write(&self->mh_lock);
  assert(IS_ALIGNED(free_begin,HEAP_ALIGNMENT));
  assert(IS_ALIGNED(free_size,HEAP_ALIGNMENT));
  if (!mheap_release(self,(void *)free_begin,free_size,flags&~GFP_CALLOC))
       free_size = 0;
  atomic_rwlock_endwrite(&self->mh_lock);
#ifndef MPTR_HAVE_TYPE
  assert(!(free_size&GFP_MASK_MPTR));
#endif
  ptr->m_size -= free_size;
#endif
 }
#ifdef MPTR_HAVE_TAIL
 else if (old_usersize != new_usersize) {
  assert(old_size == new_size);
  mptr_mvtail(ptr,old_size,old_usersize,
                  old_size,new_usersize,
              flags);
 }
#endif
 return ptr;
}

#ifdef HEAP_REALIGN
#undef HEAP_REALIGN
#endif

DECL_END
