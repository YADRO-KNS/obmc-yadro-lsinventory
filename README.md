# Utility for printing OpenBMC inventory
The project provides console utility that can be used for printing inventory
list stored by OpenBMC.

## Build with OpenBMC SDK
OpenBMC SDK contains toolchain and all dependencies needed for building the
project. See [official documentation](https://github.com/openbmc/docs/blob/master/development/dev-environment.md#download-and-install-sdk) for details.

Build steps:
```sh
$ source /path/to/sdk/environment-setup-arm1176jzs-openbmc-linux-gnueabi
$ mkdir build_dir
$ meson build_dir
$ ninja -C build_dir
```
If build process succeeded, the directory `build_dir` contains executable
file `lsinventory`.

## Testing
Unit tests can be built and run with OpenBMC SDK.

Run tests:
```sh
$ source /path/to/sdk/environment-setup-arm1176jzs-openbmc-linux-gnueabi
$ # build the project (see above)
$ qemu-arm -L ${SDKTARGETSYSROOT} build_dir/test/lsinventory_test
```
