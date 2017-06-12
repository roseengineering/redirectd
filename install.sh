
set -v

make clean
make

sudo systemctl stop redirectd
sudo systemctl disable redirectd

sudo cp redirectd /usr/bin
sudo cp redirectd.service /lib/systemd/system

sudo systemctl enable redirectd
sudo systemctl start redirectd
sudo systemctl status redirectd


