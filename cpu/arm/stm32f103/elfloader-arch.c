/**
 * \addtogroup mbxxx-platform
 *
 * @{
 */

/*
 * Copyright (c) 2009, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "contiki.h"
#include "elfloader-arch.h"
#include "cfs-coffee-arch.h"
#include "drivers/stm32f1xx/flash.h"

#if 1
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define ELF32_R_TYPE(info)      ((unsigned char)(info))

/* Supported relocations */
#define R_ARM_ABS32	2
#define R_ARM_THM_CALL	10

/* Adapted from elfloader-arm.c */
/* word aligned */
static uint32_t datamemory_aligned[(ELFLOADER_DATAMEMORY_SIZE + 3) / 4];
static uint8_t *datamemory = (uint8_t *) datamemory_aligned;

/* halfword aligned */
VAR_AT_SEGMENT(static const uint16_t
               textmemory[ELFLOADER_TEXTMEMORY_SIZE / 2], ".elf_text") = {0};
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_ram(int size)
{
  if(size > sizeof(datamemory_aligned)) {
    PRINTF("RESERVED RAM TOO SMALL\n");
  }
  return datamemory;
}
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_rom(int size)
{
  if(size > sizeof(textmemory)) {
    PRINTF("RESERVED FLASH TOO SMALL\n");
  }
  return (void *)textmemory;
}
/*---------------------------------------------------------------------------*/
#define READSIZE sizeof(datamemory_aligned)

void
elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size,
                         char *mem)
{
  uint32_t ptr;
  int nbytes;
  cfs_seek(fd, textoff, CFS_SEEK_SET);
  cfs_seek(fd, textoff, CFS_SEEK_SET);
  for(ptr = 0; ptr < size; ptr += READSIZE) {
    /* Read data from file into RAM. */
    nbytes = cfs_read(fd, (unsigned char *)datamemory, READSIZE);
    /* Write data to flash. */
    flash_write_memory_half_word((uint32_t) mem, datamemory, nbytes);
  }
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_relocate(int fd,
                        unsigned int sectionoffset,
                        char *sectionaddr,
                        struct elf32_rela *rela, char *addr)
{
  unsigned int type;
  type = ELF32_R_TYPE(rela->r_info);
  cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
/*   PRINTF("elfloader_arch_relocate: type %d\n", type); */
/*   PRINTF("Addr: %p, Addend: %ld\n",   addr, rela->r_addend); */
  switch (type) {
  case R_ARM_ABS32:
    {
      int32_t addend;
      cfs_read(fd, (char *)&addend, 4);
      addr += addend;
      cfs_seek(fd, -4, CFS_SEEK_CUR);
      cfs_write(fd, &addr, 4);
      /* elfloader_output_write_segment(output,(char*) &addr, 4); */
      PRINTF("%p: addr: %p\n", sectionaddr + rela->r_offset, addr);
    }
    break;
  case R_ARM_THM_CALL:
    {
      uint16_t instr[2];
      int32_t offset;
      char *base;
      cfs_read(fd, (char *)instr, 4);
      cfs_seek(fd, -4, CFS_SEEK_CUR);

      /*
       * Ignore the addend since it will be zero for calls to symbols,
       * and I can't think of a case when doing a relative call to
       * a non-symbol position
       */
      base = sectionaddr + (rela->r_offset + 4);
      if(((instr[1]) & 0xe800) == 0xe800) {
        /* BL or BLX */
        if(((uint32_t) addr) & 0x1) {
          /* BL */
          instr[1] |= 0x1800;
        } else {
#if defined(__ARM_ARCH_4T__)
          return ELFLOADER_UNHANDLED_RELOC;
#else
          /* BLX */
          instr[1] &= ~0x1800;
          instr[1] |= 0x0800;
#endif
        }
      }
      
      /* Adjust address for BLX */
      if((instr[1] & 0x1800) == 0x0800) {
        addr = (char *)((((uint32_t) addr) & 0xfffffffd) |
                (((uint32_t) base) & 0x00000002));
      }
      offset = addr - (sectionaddr + (rela->r_offset + 4));
      PRINTF("elfloader-arm.c: offset %d\n", (int)offset);
      if(offset < -(1 << 22) || offset >= (1 << 22)) {
        PRINTF("elfloader-arm.c: offset %d too large for relative call\n",
               (int)offset);
      }
      /*   PRINTF("%p: %04x %04x  offset: %d addr: %p\n", sectionaddr +rela->r_offset, instr[0], instr[1], (int)offset, addr);  */
      instr[0] = (instr[0] & 0xf800) | ((offset >> 12) & 0x07ff);
      instr[1] = (instr[1] & 0xf800) | ((offset >> 1) & 0x07ff);
      cfs_write(fd, &instr, 4);
      /* elfloader_output_write_segment(output, (char*)instr, 4); */
      /*     PRINTF("cfs_write: %04x %04x\n",instr[0], instr[1]); */
    }
    break;

  default:
    PRINTF("elfloader-arm.c: unsupported relocation type %d\n", type);
  }
}
/** @} */
