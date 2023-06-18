#! /bin/sh

set -e

cd "$(dirname "$0")/../"

echo "=> Installing shutdown listener...\n"
sudo cp listen-for-shutdown.py /usr/local/bin/
sudo chmod +x /usr/local/bin/listen-for-shutdown.py

echo "=> Starting shutdown listener...\n"
cd script.rc.d
sudo cp listen-for-shutdown.sh /etc/init.d/
sudo chmod +x /etc/init.d/listen-for-shutdown.sh

sudo update-rc.d listen-for-shutdown.sh defaults
sudo /etc/init.d/listen-for-shutdown.sh start

echo "Shutdown listener installed.\n"
echo "Check out howchoo.com for more awesome Pi projects!"
