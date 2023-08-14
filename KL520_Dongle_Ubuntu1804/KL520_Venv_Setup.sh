#create virtualenv is venv
python3 -m virtualenv --python=/usr/local/python3/bin/python3.8 venv

#into venv environment
source ./venv/bin/activate

pip install keras==2.2.4
pip install tensorflow
pip install opencv-python
pip install host_lib/python/packages/kdp_host_api-0.9.1_linux_-py3-none-any.whl
