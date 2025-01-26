#!/bin/bash

# Enable 32 bit architecture
sudo dpkg --add-architecture i386

# Add Wine repository key
sudo mkdir -pm755 /etc/apt/keyrings
sudo wget -O /etc/apt/keyrings/winehq-archive.key https://dl.winehq.org/wine-builds/winehq.archive.key

# Add Wine repository
sudo wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/debian/dists/bullseye/winehq-bullseye.sources

# Update package list
sudo apt update

# Install Wine
sudo apt install -y --install-recommends winehq-stable