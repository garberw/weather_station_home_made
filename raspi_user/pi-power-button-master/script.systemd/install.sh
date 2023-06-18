#! /bin/sh

set -e

cd "$(dirname "$0")/.."

echo "=> Installing shutdown listener...\n"
sudo cp listen-for-shutdown.py /usr/local/bin/
sudo chmod +x /usr/local/bin/listen-for-shutdown.py

echo "=> Starting shutdown listener...\n"
cd script.systemd
sudo cp listen-for-shutdown.service /etc/systemd/system/
sudo systemctl daemon-reload

sudo systemctl enable --now listen-for-shutdown.service

systemctl status listen-for-shutdown.service

echo "Shutdown listener installed.\n"
echo "Check out howchoo.com for more awesome Pi projects!"
