#load folder open docker toolchain
#example : $bash KL520_Toolchain_Run.sh

#sudo docker run --rm -it -v /home/$USER/host_lib/$1:/$1 kneron/toolchain:520
#sudo docker run --rm -it -v /home/$USER/host_lib/data1:/data1 kneron/toolchain:520
mkdir -p data1
sudo docker run --rm -it -v $(dirname $(readlink -f $0))/data1:/data1 kneron/toolchain:520
