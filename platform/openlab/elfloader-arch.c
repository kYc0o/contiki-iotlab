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
#define ELFLOADER_CONF_TEXT_IN_ROM 1

#if ELFLOADER_CONF_TEXT_IN_ROM
/* Adapted from elfloader-arm.c */
/* word aligned */
static uint32_t datamemory_aligned[(ELFLOADER_DATAMEMORY_SIZE + 3) / 4];
static uint8_t *datamemory = (uint8_t *) datamemory_aligned;
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
static uint16_t datamemory_aligned[ELFLOADER_DATAMEMORY_SIZE/2+1];
static uint8_t* datamemory = (uint8_t *)datamemory_aligned;
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */

/* halfword aligned
VAR_AT_SEGMENT(static const uint16_t
               textmemory[ELFLOADER_TEXTMEMORY_SIZE / 2], ".elf_text") = {0};
static const uint16_t textmemory[ELFLOADER_TEXTMEMORY_SIZE] = {0};*/
#if ELFLOADER_CONF_TEXT_IN_ROM
static const uint16_t textmemory[ELFLOADER_TEXTMEMORY_SIZE / 2] = {0};
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
static uint16_t textmemory[ELFLOADER_TEXTMEMORY_SIZE];
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_ram(int size)
{
  if(size > sizeof(datamemory_aligned)) {
    PRINTF("RESERVED RAM TOO SMALL\n");
  }
  PRINTF("Allocated RAM: %p\n", datamemory);
  return datamemory;
}
/*---------------------------------------------------------------------------*/
void *
elfloader_arch_allocate_rom(int size)
{
  if(size > sizeof(textmemory)) {
    PRINTF("RESERVED FLASH TOO SMALL\n");
  }
  PRINTF("Allocated ROM: %p\n", textmemory);
  return (void *)textmemory;
}
/*---------------------------------------------------------------------------*/
#define READSIZE sizeof(datamemory_aligned)

void
elfloader_arch_write_rom(int fd, unsigned short textoff, unsigned int size,
                         char *mem)
{
#if ELFLOADER_CONF_TEXT_IN_ROM
  uint32_t ptr;
  int nbytes;
  cfs_seek(fd, textoff, CFS_SEEK_SET);
  cfs_seek(fd, textoff, CFS_SEEK_SET);
    /* Read data from file into RAM. */
  nbytes = cfs_read(fd, (unsigned char *)datamemory, READSIZE);
  PRINTF("Bytes read: %d\n", nbytes);
  flash_erase_memory_page((uint32_t)mem);
  
  for(ptr = 0; ptr < size; ptr += 2) {
    uint16_t data;
    data = datamemory[ptr + 1];
    data = data << 8;
    data |= datamemory[ptr];
    /* Write data to flash. */
    PRINTF("Writting %04X to %08X\n", data, (uint32_t)mem + ptr);
    flash_write_memory_half_word((uint32_t) mem + ptr, data);
  }
#else /* ELFLOADER_CONF_TEXT_IN_ROM */
  PRINTF("Using serial flash\n");
  cfs_seek(fd, textoff, CFS_SEEK_SET);
  cfs_read(fd, (unsigned char *)mem, size);
#endif /* ELFLOADER_CONF_TEXT_IN_ROM */
}
/*---------------------------------------------------------------------------*/
void
elfloader_arch_relocate(int fd,
                        unsigned int sectionoffset,
                        char *sectionaddr,
                        struct elf32_rela *rela, char *addr)
{
  PRINTF("File descriptor: %d\n", fd);
  PRINTF("Section offset: %x\n", sectionoffset);
  PRINTF("Section address: %p\n", sectionaddr);
  PRINTF("rela->r_info: %08X\n", rela->r_info);
  PRINTF("rela->r_offset: %08X\n", rela->r_offset);
  PRINTF("Address: %p\n", addr);
  unsigned int type;
  type = ELF32_R_TYPE(rela->r_info);
  cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
/*   PRINTF("elfloader_arch_relocate: type %d\n", type); */
/*   PRINTF("Addr: %p, Addend: %ld\n",   addr, rela->r_addend); */
  switch (type) {
  case R_ARM_ABS32:
    {
      /*int32_t addend;
      cfs_read(fd, (char *)&addend, sizeof(char*));
      PRINTF("addend: %d, addr: %p\n", addend, addr);*/
      PRINTF("addend: %x, addr: %p\n", rela->r_addend, addr);
      /*addr += addend;*/
      addr += rela->r_addend;
      /*cfs_seek(fd, -sizeof(char*), CFS_SEEK_CUR);*/
      cfs_write(fd, &addr, sizeof(char*));
      /* elfloader_output_write_segment(output,(char*) &addr, 4); */
      PRINTF("sectionadd + rela->r_offset: %p, addr: %p\n", sectionaddr + rela->r_offset, addr);
    }
    break;
  case R_ARM_THM_CALL:
    {
      uint16_t instr[2];
      cfs_read(fd, (char *)instr, 4);
      uint32_t i, j;
      int32_t addend;
      int32_t final_offset;
      i = instr[1];
      j = instr[0];
      j = j << 16;
      i = i | j;
      PRINTF("R_ARM_THM_CALL instruction: %x\n", i);
      i = (i << 2) & 0x00FFFFFF;
      if(i & 0x00800000)
	i = i | 0xFF000000;
      addend = (int32_t)i;
      PRINTF("Initial addend is: %d\n", addend);   
      
      final_offset = (addr + addend) - (sectionaddr + rela->r_offset);
      
      PRINTF("Pre-Final offset: %d\n", final_offset);
      
      final_offset = final_offset >> 2;
      
      if(final_offset & 0x20000000)
	final_offset &= 0x00FFFFFF;
      
      PRINTF("Final offset: %d\n", final_offset);
      
      i = (uint32_t)(final_offset & 0x0000FFFF);
      j = (uint32_t)((final_offset & 0x00FF0000) >> 16);
      
      instr[1] = i;
      instr[0] = instr[0] | (uint16_t)j;
      
      PRINTF("instr[0]: %x, instr[1]: %x\n", instr[0], instr[1]);
      
      cfs_seek(fd, -4, CFS_SEEK_CUR);
      cfs_write(fd, &instr, 4);
      
      
    }
    break;

  default:
    PRINTF("elfloader-arm.c: unsupported relocation type %d\n", type);
  }
}

void elfloader_arch_read_rom(int fd, unsigned short textoff, unsigned int size, char *mem)
{
  uint32_t ptr;
  PRINTF("Size: %d, READSIZE: %d\n", size, READSIZE);
  for(ptr = 0; ptr < size; ptr++) {
    /* Read data from flash. */
    printf("%02X", *mem + ptr);
  }
  printf("\n");
}
/*void
elfloader_arch_relocate(int fd, unsigned int sectionoffset,
			char *sectionaddr,
			struct elf32_rela *rela, char *addr)
{
  addr += rela->r_addend;

  cfs_seek(fd, sectionoffset + rela->r_offset, CFS_SEEK_SET);
  cfs_write(fd, (char *)&addr, 2);
}*/
/** @} */
