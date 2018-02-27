#!/usr/bin/env bash

set -evx

sudo wget -O boost_1_66_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.gz/download
sudo tar xzf boost_1_66_0.tar.gz

cd boost_1_66_0/
sudo ./bootstrap.sh --with-libraries=random,thread,system --prefix=/usr/local
sudo ./b2 -d0 variant=debug link=shared
sudo ./b2 -d0 install 

