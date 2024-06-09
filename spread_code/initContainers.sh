#!/bin/bash

sloc="/home/ubuntuadm/SPREAD"

sudo lxc launch ubuntu:jammy $1
#sudo lxc launch ubuntu:jammy ubuntuC-node2
#sudo lxc launch ubuntu:jammy ubuntuC-node3

sleep 2

cat > cconf.sh <<EOF

#--------
#!/bin/bash

usermod -l ubuntuadm ubuntu
sudo groupmod -n ubuntuadm ubuntu
sudo usermod -d /home/ubuntuadm -m ubuntuadm
sudo passwd ubuntuadm

sed -i "s/PasswordAuthentication no/PasswordAuthentication yes/" /etc/ssh/sshd_config.d/60-cloudimg-settings.conf
systemctl restart sshd

echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ubuntuadm/SPREAD/libSpread' >> /home/ubuntuadm/.bashrc
echo 'export LIBRARY_PATH=$LIBRARY_PATH:/home/ubuntuadm/SPREAD/libSpread' >> /home/ubuntuadm/.bashrc
echo 'export CPATH=$CPATH:/home/ubuntuadm/SPREAD/includeSpread' >> /home/ubuntuadm/.bashrc

mv /root/SPREAD/ /home/ubuntuadm/

ln -s ${sloc}/libSpread/libspread-core.so.3.0.0* ${sloc}/libSpread/libspread-core.so
ln -s ${sloc}/libSpread/libspread-core.so.3.0.0* ${sloc}/libSpread/libspread-core.so.3
ln -s ${sloc}/libSpread/libspread.so.3.0.0* ${sloc}/libSpread/libspread.so
ln -s ${sloc}/libSpread/libspread.so.3.0.0* ${sloc}/libSpread/libspread.so.3
ln -s ${sloc}/libSpread/libtspread-core.so.3.0.0* ${sloc}/libSpread/libtspread-core.so
ln -s ${sloc}/libSpread/libtspread-core.so.3.0.0* ${sloc}/libSpread/libtspread-core.so.3
#--------
EOF

chmod +x cconf.sh

lxc file push ./cconf.sh $1/root/
#lxc file push ./cconf.sh ubuntuC-node2/home/ubuntuadm/
#lxc file push ./cconf.sh ubuntuC-node3/home/ubuntuadm/

lxc file push --recursive SPREAD $1/root/
#lxc file push --recursive SPREAD ubuntuC-node2/home/ubuntuadm/
#lxc file push --recursive SPREAD ubuntuC-node3/home/ubuntuadm/

#lxc exec $1 sh cconf.sh
#lxc exec ubuntuC-node2 sh ubuntuC-node2/home/ubuntuadm/cconf.sh
#lxc exec ubuntuC-node3 sh ubuntuC-node3/home/ubuntuadm/cconf.sh
