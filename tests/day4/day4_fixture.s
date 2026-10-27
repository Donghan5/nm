.file "day4_fixture.s"
.text
.globl zebra_text
.type zebra_text,@function
zebra_text:
.byte 0xc3
.size zebra_text,.-zebra_text
.local local_text
.type local_text,@function
local_text:
.byte 0xc3
.weak weak_function
.type weak_function,@function
weak_function:
.byte 0xc3
.weak weak_notype
weak_notype:
.byte 0xc3
/* Relocations preserve undefined entries in the symbol table. */
.long external_symbol
.weak weak_missing
.long weak_missing
.weak weak_object_missing
.type weak_object_missing,@object
.long weak_object_missing
.data
.globl alpha_data
.type alpha_data,@object
alpha_data:
.long 42
.local local_data
local_data:
.long 11
.weak weak_object
.type weak_object,@object
weak_object:
.long 7
.bss
.globl beta_bss
beta_bss:
.zero 8
.local local_bss
local_bss:
.zero 4
.section .rodata,"a",@progbits
.globl gamma_readonly
gamma_readonly:
.long 17
.local local_readonly
local_readonly:
.long 23
.section .rodata.str1.1,"aMS",@progbits,1
.globl merged_string
merged_string:
.asciz "day4"
.section .tdata,"awT",@progbits
.globl tls_data
.type tls_data,@tls_object
tls_data:
.long 1
.section .tbss,"awT",@nobits
.globl tls_bss
.type tls_bss,@tls_object
tls_bss:
.zero 4
.globl absolute_global
.set absolute_global,0x1234
.local absolute_local
.set absolute_local,0x42
.comm common_global,16,8
.section .init_array,"aw",@init_array
.globl init_array_global
init_array_global:
.long 0
.local init_array_local
init_array_local:
.long 0
.section .fini_array,"aw",@fini_array
.globl fini_array_global
fini_array_global:
.long 0
.section .preinit_array,"aw",@preinit_array
.local preinit_array_local
preinit_array_local:
.long 0
.section .note.day4,"a",@note
.balign 4
.local note_local
note_local:
.long 4,4,1
.asciz "NM4"
.long 1
.section .note.GNU-stack,"",@progbits
