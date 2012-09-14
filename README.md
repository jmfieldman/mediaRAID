mediaRAID
=========

./mediaRAID -o default_permissions -o allow_other -log output.log -port 14444 tmp
wget -qO- -t 1 "http://localhost:14444/volume/add?basepath=/tmp/raid/t1"