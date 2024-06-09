
#--------
#!/bin/bash

usermod -l ubuntuadm ubuntu
sudo groupmod -n ubuntuadm ubuntu
sudo usermod -d /home/ubuntuadm -m ubuntuadm
sudo passwd ubuntuadm

sed -i "s/PasswordAuthentication no/PasswordAuthentication yes/" /etc/ssh/sshd_config.d/60-cloudimg-settings.conf
systemctl restart sshd

echo 'export LD_LIBRARY_PATH=:/home/ubuntuadm/SPREAD/libSpread' >> /home/ubuntuadm/.bashrc
echo 'export LIBRARY_PATH=:/home/ubuntuadm/SPREAD/libSpread' >> /home/ubuntuadm/.bashrc
echo 'export CPATH=:/home/ubuntuadm/SPREAD/includeSpread' >> /home/ubuntuadm/.bashrc

mv /root/SPREAD/ /home/ubuntuadm/

ln -s /home/ubuntuadm/SPREAD/libSpread/libspread-core.so.3.0.0* /home/ubuntuadm/SPREAD/libSpread/libspread-core.so
ln -s /home/ubuntuadm/SPREAD/libSpread/libspread-core.so.3.0.0* /home/ubuntuadm/SPREAD/libSpread/libspread-core.so.3
ln -s /home/ubuntuadm/SPREAD/libSpread/libspread.so.3.0.0* /home/ubuntuadm/SPREAD/libSpread/libspread.so
ln -s /home/ubuntuadm/SPREAD/libSpread/libspread.so.3.0.0* /home/ubuntuadm/SPREAD/libSpread/libspread.so.3
ln -s /home/ubuntuadm/SPREAD/libSpread/libtspread-core.so.3.0.0* /home/ubuntuadm/SPREAD/libSpread/libtspread-core.so
ln -s /home/ubuntuadm/SPREAD/libSpread/libtspread-core.so.3.0.0* /home/ubuntuadm/SPREAD/libSpread/libtspread-core.so.3
#--------
