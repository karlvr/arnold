chmod +x arnold.desktop
mkdir -p ~/.local/share/applications
mkdir -p ~/.local/share/mime/packages


xdg-icon-resource install --context mimetypes --size 32 --mode system ../icons/32x32/disk.png application-x-dsk
xdg-icon-resource install --context mimetypes --size 32 --mode system ../icons/32x32/cass.png application-x-cdt
xdg-icon-resource install --context mimetypes --size 32 --mode system ../icons/32x32/cass.png application-x-tzx
xdg-icon-resource install --context mimetypes --size 32 --mode system ../icons/32x32/cart.png application-x-cpr
cp arnold.desktop ~/.local/share/applications

#xdg-mime install mytype-mime.xml
cp x-dsk.xml ~/.local/share/mime/packages/x-dsk.xml
cp x-cpr.xml ~/.local/share/mime/packages/x-cpr.xml
cp x-cdt.xml ~/.local/share/mime/packages/x-cdt.xml
cp x-tzx.xml ~/.local/share/mime/packages/x-tzx.xml
cp x-sna.xml ~/.local/share/mime/packages/x-sna.xml
update-desktop-database ~/.local/share/applications
update-mime-database ~/.local/share/mime
