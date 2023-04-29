.section .data
.global ramdisk_start, ramdisk_end
ramdisk_start:
.incbin "build/ramdisk.img"
ramdisk_end:

.section .rodata
.globl logo
logo:
.incbin "resources/logo.txt"
.byte 0

// ramdisk_start: 创建标识符
// .global ramdisk 设置该标识符可被其它编译单元访问
// .inbin ".img" 可引入镜像文件
// .section [.text/.data/.rodata/.bss] 标明后面的代码属于哪个段 