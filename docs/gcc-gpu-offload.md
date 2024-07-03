# GCC GPU Offload Notes (1-July-2024)

## Installing GCC with NVPTX offload support on Ubuntu 22.04

### Option 1: Using APT
- Install system gcc: `sudo apt install gcc-11 g++-11`
- Install NVPTX offload plugin and necessary tools: `sudo apt install gcc-11-offload-nvptx`

### Option 2: Using Spack
Assuming that Spack is installed and is present on the PATH.
- Build and install gcc with offload support: `spack install gcc@11.4.0+nvptx`
- Install dependencies: `sudo apt install gcc-11-offload-nvptx`

## Using GCC with NVPTX offload support

### Usage
To enable compilation of OpenMP pragmas one must pass the compiler flag `-fopenmp`, and furthermore to enable OpenMP to be offloaded to Nvidia hardware, one must also pass the compiler flag `-foffload=nvptx-none`.

If you are using gcc-11/12, ensure that `ptxas` is **not** on the PATH (see issue 1) and use the compiler options `-no-pie -fcf-protection=none -fno-stack-protector` (see issues 2, 3, and 4).

#### Example gcc-11/12
Example of compiling some file, `main.cpp`, that makes use of OpenMP offload and the CUDA runtime.
```
g++-11 -I$(CUDA_PATH)/include -fopenmp -foffload=nvptx-none -no-pie -fcf-protection=none -fno-stack-protector -std=c++17 -o app.exe main.cpp  -L$(CUDA_PATH)/lib64 -lcudart
```

### ISSUES

- **Issue 1**: If `ptxas` the Nvidia PTX Assembler (from the CUDA Toolkit) is present on the PATH, then the gcc offload mechanism tries to use it, otherwise it tries to use `nvptx-none-as` which is installed by the APT package `nvptx-tools` (a dependency of `gcc-11-offload-nvptx`). Usage of `ptxas` by the gcc offload mechanism can lead to compilation errors of the following form:
    ```
    ptxas fatal   : Value 'sm_30' is not defined for option 'gpu-name'
    nvptx-as: ptxas returned 255 exit status
    mkoffload: fatal error: x86_64-linux-gnu-accel-nvptx-none-gcc-12 returned 1 exit status
    compilation terminated.
    lto-wrapper: fatal error: /usr/lib/gcc/x86_64-linux-gnu/12//accel/nvptx-none/mkoffload returned 1 exit status
    compilation terminated.
    /usr/bin/ld: error: lto-wrapper failed
    collect2: error: ld returned 1 exit status
    ```
    The reason for this error is that the specific target architecture "sm_30" is not supported by the installed version of `ptxas`. On gcc-11 and gcc-12 at least, I have tried to mitigate this issue by setting the target compute architecture in the compilation flags for example by passing `-foffload=nvptx-none="-misa=sm_53"`, however, this doesn't seem to have any effect. And so the only mitigation seems to be to ensure that `ptxas` is not on the PATH hence making gcc use `nvptx-none-as`.

    **Remedy**: ensure that `ptxas` is **not** on the PATH while using gcc for compilation involving offload.
- **Issue 2**: The following compilation warning by:
    ```
    /usr/bin/ld: /tmp/cctA1T8M.crtoffloadtable.o: warning: relocation against `__offload_funcs_end' in read-only section `.rodata'
    /usr/bin/ld: warning: creating DT_TEXTREL in a PIE
    ```
    **Remedy**: set the compiler option `-no-pie`.

- **Issue 3**: The following compilation error:
    ```
    lto1: error: '-fcf-protection=full' is not supported for this target
    mkoffload: fatal error: x86_64-linux-gnu-accel-nvptx-none-gcc-12 returned 1 exit status
    compilation terminated.
    ```
    **Remedy**: set the compiler option `-fcf-protection=none`.
- **Issue 4**: The following runtime error:
    ```
    libgomp: Link error log ptxas application ptx input, line 135; error   : Illegal operand type to instruction 'ld'
    ptxas application ptx input, line 239; error   : Illegal operand type to instruction 'ld'
    ptxas application ptx input, line 135; error   : Unknown symbol '__stack_chk_guard'
    ptxas application ptx input, line 239; error   : Unknown symbol '__stack_chk_guard'
    ptxas fatal   : Ptx assembly aborted due to errors
    ```
    **Remedy**: set the compiler option `-fno-stack-protector` (see https://stackoverflow.com/questions/58854858/undefined-symbol-stack-chk-guard-in-libopenh264-so-when-building-ffmpeg-wit).
- **Issue 5**: The following compilation error (is due to the program `mkoffload` not being found in the specified locations):
    ```
    lto-wrapper: fatal error: could not find accel/nvptx-none/mkoffload in /usr/lib/gcc/x86_64-linux-gnu/11/:/usr/lib/gcc/x86_64-linux-gnu/11/:/usr/lib/gcc/x86_64-linux-gnu/:/usr/lib/gcc/x86_64-linux-gnu/11/:/usr/lib/gcc/x86_64-linux-gnu/ (consider using ‘-B’)
    compilation terminated.
    /usr/bin/ld: error: lto-wrapper failed
    collect2: error: ld returned 1 exit status
    ```
    **Remedy**: Install `mkoffload` in an appropriate location. `mkoffload` is part of the APT package `gcc-11-offload-nvptx` and so can be installed using `sudo apt install gcc-11-offload-nvptx` which is a step highlighted in the gcc installation instructions above.
