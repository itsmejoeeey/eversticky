# <img src="https://i.imgur.com/A1SvZTi.png" width="48"> EverSticky

<img src="https://i.imgur.com/WzwJvRG.png" width="450">

A Linux open-source sticky note client that syncs to Evernote. Displays rich text formatting.

## Installation and running

Supports Ubuntu 20.04 and greater. Unlikely to work on older Ubuntu versions.

### Install on Ubuntu

```
wget https://github.com/itsmejoeeey/eversticky/releases/download/v0.95.0/eversticky_0.95.0-1_amd64.deb
sudo apt install ./eversticky_0.95.0-1_amd64.deb
```

### Install on Arch Linux

Available in the [Arch User Repository](https://aur.archlinux.org/packages/eversticky/).

### Compiling from source

> IMPORTANT NOTE: You will need a production Evernote API key to compile the application and be able to see and alter notes you currently store on Evernote. This secret (along with the desired domain and key) needs to be provided at the top of `./src/eversticky.pro`.

Install build dependencies:
```
sudo apt install build-essential qt5-default qtwebengine5-dev libqt5x11extras5-dev libxpm-dev
```

From the root of the repo run the following commands:
```
mkdir src/build
cd src/build
qmake ../eversticky.pro
make
```
This produces the executable binary `./eversticky` in the current directory.

## Bugs
This project is still in its early days and bugs are expected. Open an issue if you encounter some unexpected behaviour (after making sure one hasn't already been raised).

## License
`src/qevercloud` licensed under MIT terms. Consists of `v4.0.0` release from @d1vanov's fork (https://github.com/d1vanov/QEverCloud), plus two patches ([66671bf](https://github.com/itsmejoeeey/eversticky/commit/66671bf4ffc03c4d7ed64227249971be2b35a492), [74b9b98](https://github.com/itsmejoeeey/eversticky/commit/74b9b98d67a9370a34e9b0d0a073482c0657bb3f)).

For all other code see `LICENSE`
