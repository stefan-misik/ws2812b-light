#!/usr/bin/sh



github_get() {
  git clone "https://github.com/$1/$2.git"
  git -C "$2" checkout -d "$3"
  git -C "$2" submodule update --init
}

github_get STMicroelectronics STM32CubeG0 v1.6.2

