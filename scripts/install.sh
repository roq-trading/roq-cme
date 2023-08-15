#!/usr/bin/env bash

if [[ ! -z $CONDA_PREFIX ]]; then
  (>&2 echo -e "\033[1;31mERROR: Please deactivate your conda environment before using this script. Use 'conda deactivate'.\033[0m") && exit 1
fi

OPT_DIR="opt"

CONDA_DIR="$OPT_DIR/conda"

if [[ -d "$CONDA_DIR" ]]; then
  (>&2 echo -e "\033[1;31mERROR: Refusing to overwrite an existing conda installation. Directory is '$CONDA_DIR'.\033[0m") && exit 1
fi

if [[ -z $1 ]]; then
  (>&2 echo -e "\033[1;31mERROR: Expected first argument to be a build. Use 'stable' or 'unstable'..\033[0m") && exit 1
fi

BUILD="$1"

echo "BUILD=$BUILD"

case "$BUILD" in
  stable)
    ;;
  unstable)
    ;;
  test)  # temporary (maybe)
    ;;
  *)
    (>&2 echo -e "\033[1;31mERROR: Unknown build. Should be 'stable' or 'unstable'.\033[0m") && exit 1
    ;;
esac

CONDA_INSTALLER="Miniforge3-Linux-x86_64.sh"

MINIFORGE_DOWNLOAD_URL="https://github.com/conda-forge/miniforge/releases/latest/download"

CONDA_DOWNLOAD_URL="$MINIFORGE_DOWNLOAD_URL/$CONDA_INSTALLER"

if [[ -d $OPT_DIR ]]; then
  if [[ ! -f $OPT_DIR/$CONDA_INSTALLER ]]; then
    echo -e "\033[1;34mDownload installer...\033[0m"
    curl --location --output "$OPT_DIR/$CONDA_INSTALLER" "$CONDA_DOWNLOAD_URL"
  fi
fi

echo -e "\033[1;34mInstall conda...\033[0m"

bash "$OPT_DIR/$CONDA_INSTALLER" -b -p "$CONDA_DIR"

echo -e "\033[1;34mInstall gateway and tools...\033[0m"

ROQ_CHANNEL="https://roq-trading.com/conda/$BUILD"

$CONDA_DIR/bin/conda install \
  --freeze-installed --yes \
  --channel $ROQ_CHANNEL \
  roq-cme roq-tools roq-test

echo -e "\033[1;34mReady!\033[0m"
echo -e "\033[1;34mYou can now activate your conda environment using 'source $CONDA_DIR/bin/activate'.\033[0m"
