DIR="$(dirname "$(readlink -f "$0")")"
QEMU="$(which qemu-system-x86_64)"
QEMU=/home/kruztw/Downloads/qemu/build/x86_64-softmmu/qemu-system-x86_64

$QEMU  \
    -cpu max,+smap,+smep,check \
    -m 128 -nographic \
    -kernel "$DIR/bzImage" \
    -initrd "$DIR/local.cpio.gz" \
    -append "console=ttyS0 quiet init='/init' nokaslr" -gdb tcp::1234 \
    -device pci-hellodev
    -monitor tcp:127.0.0.1:1235,server,nowait \
