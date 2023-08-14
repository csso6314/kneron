sudo rm /var/lib/dpkg/lock-frontend
sudo rm /var/lib/dpkg/lock
sudo rm /var/cache/apt/archives/lock

#####################install python3####################
sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install cmake
sudo apt-get -y install libusb-1.0-0-dev
sudo apt-get -y install python3
sudo apt-get -y install python3-dev python3-pip
sudo python3 -m pip install --upgrade pip
python3-venv pip3 install virtualenv --user

#####################install python3.8.6 for venv####################
sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install gcc g++ make
sudo apt-get -y dist-upgrade
sudo apt-get -y install build-essential python-dev python-setuptools python-pip python-smbus libncursesw5-dev libgdbm-dev libc6-dev zlib1g-dev libsqlite3-dev tk-dev libssl-dev openssl libffi-dev
sudo apt-get -y install build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev wget
wget https://www.python.org/ftp/python/3.8.6/Python-3.8.6.tgz
tar -zxvf Python-3.8.6.tgz
cd Python-3.8.6
./configure --with-ssl --prefix=/usr/local/python3
make
sudo make install
cd ..
sudo rm -rf ./Python-3.8.6
rm -rf ./Python-3.8.6.tgz

####################set dongle permission##############################################
sudo touch /etc/udev/rules.d/rplidar.rules
sudo sh -c 'echo SUBSYSTEMS==\"usb\", ATTRS{idVendor}==\"3231\", ATTRS{idProduct}==\"0100\", MODE=\"0777\", GROUP=\"users\", SYMLINK+=\"rplidar\" > /etc/udev/rules.d/rplidar.rules'
sudo service udev reload
sudo service udev restart

####################DOCKER_INSTALL######################################################
sudo apt-get update
sudo apt-get -y install \
    apt-transport-https \
    ca-certificates \
    curl \
    gnupg-agent \
    software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo apt-key fingerprint 0EBFCD88
sudo add-apt-repository \
   "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
   $(lsb_release -cs) \
   stable"
sudo apt-get update
sudo apt-get -y install docker-ce docker-ce-cli containerd.io
sudo docker run hello-world
sudo adduser $USERNAME docker
sudo apt-get -y install docker-compose
#pull kl520 toolchain
sudo docker pull kneron/toolchain:520

#build
cd host_lib
mkdir build
cd build
cmake ../
make -j
