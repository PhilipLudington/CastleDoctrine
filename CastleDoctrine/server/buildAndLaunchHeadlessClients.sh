# replace "secret" with a password below
# run this to build two headless client folders and launch them both
# in nohup mode

cd CastleDoctrine/gameSource
./makeHeadlessGame.sh
./makeHeadlessGameFolder.sh


cd ~

rm -r headlessClientA/
rm -r headlessClientB/

cp -r cpp/CastleDoctrine/gameSource/headlessFolder ./headlessClientA

echo "secret" > ./headlessClientA/settings/simulatorServerPassword.ini

cp -r ./headlessClientA ./headlessClientB

echo "5078" > ./headlessClientB/settings/simulatorServerPort.ini

cd ./headlessClientA
nohup ./CastleDoctrineHeadless > headlessLog.txt 2>&1 &


cd ../headlessClientB
nohup ./CastleDoctrineHeadless > headlessLog.txt 2>&1 &

