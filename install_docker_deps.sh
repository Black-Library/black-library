#!/bin/bash

# info taken from https://docs.docker.com/engine/install/ubuntu/

# get deps for docker
sudo apt-get update

sudo apt-get install \
    apt-transport-https \
    ca-certificates \
    curl \
    gnupg-agent \
    software-properties-common

# get Docker's offical GPG key
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -

# add apt repo from docker
sudo add-apt-repository \
   "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
   $(lsb_release -cs) \
   stable"

# install docker
sudo apt-get update

sudo apt-get install docker-ce docker-ce-cli containerd.io

echo "installing docker in safe mode and will require sudo priviledges to run containers"

# endinfo

# pull apline for minimal builds and ubuntu for full builds
docker pull alpine
docker pull ubuntu
