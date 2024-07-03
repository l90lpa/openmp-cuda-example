# Nvidia Drivers Notes (1-July-2024)

## Installing Nvidia Drivers and CUDA Toolkit on Ubuntu 22.04
I followed these instructions: https://learn.microsoft.com/en-us/azure/virtual-machines/linux/n-series-driver-setup#ubuntu. And these instructions have some useful additional details https://ubuntu.com/server/docs/nvidia-drivers-installation. But I have also documented the process below:

### Check Current System
- Check for the presence of an Nvidia GPU (should display name of GPU): `lspci | grep -i NVIDIA`
- Check the version of the current installed Nvidia driver (if there is one, it should display the version): `cat /proc/driver/nvidia/version`

### Installation
1. Install Ubuntu drivers utility: `sudo apt update && sudo apt install -y ubuntu-drivers-common`
1. Install latest Nvidia drivers: `sudo ubuntu-drivers install`
    - Or you can install a specific version using: `sudo ubuntu-drivers install nvidia:535`
1. **Reboot** machine.
1. Add Nvidia package repository:
    ```
    wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
    sudo apt install -y ./cuda-keyring_1.1-1_all.deb
    sudo apt update
    ```
1. Install CUDA Toolkit: `sudo apt -y install cuda-toolkit-12-2`
1. Set-up paths (you may want to add these to `.profile` or `.bashrc` etc):
    ```
    export PATH=/usr/local/cuda/bin:$PATH
    export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH
    ```

### Verification
- Check that the driver is working (should report info about GPU): `nvidia-smi`