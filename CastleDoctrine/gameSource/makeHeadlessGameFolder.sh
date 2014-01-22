rm -rf headlessFolder

mkdir headlessFolder

cd headlessFolder

mkdir -p graphics
mkdir -p gameElements
mkdir -p settings
mkdir -p languages




cp ../CastleDoctrineHeadless ./
cp ../graphics/* ./graphics
cp ../settings/* ./settings
cp ../languages/* ./languages
cp ../language.txt ./

cp -r ../gameElements/* ./gameElements