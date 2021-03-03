mkdir -p ~/package/arnold
cp -rf debian ~/package/arnold
../../../../package_src.sh
cp ../../../../arnold*.tar.gz ~/package/arnold
cd ~/package/arnold
debuild -i -us -uc -b