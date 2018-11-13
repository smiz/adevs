# Setup steps to make this work
# (1) Add kernel parameters
# To the kernel parameters used at boot time add
# intel_iommu=on rd.driver.blacklist=nouveau nouveau.modeset=0
# (2) Add module loading and configuration files
# sudo echo "vfio-pci" > /etc/modules-load.d/vfio-pci.conf
# Use lspci -nnk to find the device ID of your NVIDIA GPU then
# create a file /etc/modprobe.d/vfio.conf and add the line
# options vfio-pci ids=<your card ID here> disable_vga=1
# (3) If all goes well, you can pass through the GPU to qemu now
# by running the command below.

# To install an OS
#/home/1qn/Code/qemu-build/bin/qemu-system-x86_64 -m 8028 -hda ubuntu.qcow2 -device vfio-pci,host=0000:17:00.0 -vga std -enable-kvm -cpu host,kvm=off -cdrom /home/1qn/Downloads/ubuntu-18.04.1-desktop-amd64.iso -boot d
# To boot an already installed OS
/home/1qn/Code/qemu-build/bin/qemu-system-x86_64 -m 8028 -hda ubuntu.qcow2 -device vfio-pci,host=0000:17:00.0 -vga std -enable-kvm -cpu host,kvm=off 
