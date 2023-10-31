sudo mkdir -p /u/cs452/public
tar -xaf arm-gnu-toolchain-13.2.rel1-x86_64-aarch64-none-elf.tar.xz -C /u/cs452/public/
mv arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-elf xdev
echo "/u/cs452/public/xdev/bin" >> .bashrc