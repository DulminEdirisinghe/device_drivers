# Linux Driver Development

This README contains the commands required to compile the Linux kernel driver, build the initramfs image, run the x86 kernel using QEMU, and inspect the compiled kernel module.

## 1. Compile the Driver

The following command should be executed **inside the driver directory**.

The `-C` option specifies the path to the Linux kernel build directory.

```bash
make -C /home/ddul6978/Documents/ldd/kernel/linux-build M=$PWD modules
```

### Notes

* `-C` specifies the Linux kernel build directory.
* `M=$PWD` specifies the current directory as the external kernel module directory.
* `modules` builds the kernel modules in the current directory.

---

## 2. Build the Initramfs Image

Navigate to the `initramfs` directory:

```bash
cd initramfs/
```

Create the compressed initramfs image:

```bash
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.img
```

This creates `initramfs.img` in the parent directory.

---

## 3. Run the x86 Kernel Using QEMU

Run the compiled x86 kernel image with QEMU:

```bash
qemu-system-x86_64 \
    -kernel linux-build/arch/x86/boot/bzImage \
    -initrd initramfs.img \
    -append "console=ttyS0" \
    -nographic
```

### Options

* `-kernel` specifies the Linux kernel image.
* `-initrd` specifies the initramfs image.
* `-append "console=ttyS0"` configures the kernel to use the serial console.
* `-nographic` runs QEMU without a graphical display and uses the terminal as the console.

---

## 4. Inspect the Compiled Kernel Module

After compiling the driver, the kernel module will be generated as a `.ko` file.

To inspect the sections of the compiled `main.ko` module:

```bash
objdump -h main.ko
```

This displays information about the sections contained in the kernel module, including section names, sizes, addresses, and offsets.

---

## 5. Development Workflow

A typical workflow is:

```text
1. Compile the driver
        ↓
2. Build the initramfs image
        ↓
3. Boot the kernel using QEMU
        ↓
4. Inspect the compiled kernel module when required
```
