#! /bin/bash

[ -z "$1" ] && exit 1
[ ! -d "$1" ] && exit 1

mkdir -p $1/drivers/human 2>/dev/null
if [ ! -e  $1/drivers/human/car.xml ] || [ drivers/human/car.xml -nt $1/drivers/human/car.xml ]
then
    if [ -e $1/drivers/human/car.xml ]
    then
        echo "Saving $1/drivers/human/car.xml to $1/drivers/human/car.xml.old"
        cp -f $1/drivers/human/car.xml $1/drivers/human/car.xml.old
    fi
    cp -f drivers/human/car.xml $1/drivers/human/car.xml
    chmod 640 $1/drivers/human/car.xml
fi
if [ ! -e  $1/drivers/human/human.xml ] || [ drivers/human/human.xml -nt $1/drivers/human/human.xml ]
then
    if [ -e $1/drivers/human/human.xml ]
    then
        echo "Saving $1/drivers/human/human.xml to $1/drivers/human/human.xml.old"
        cp -f $1/drivers/human/human.xml $1/drivers/human/human.xml.old
    fi
    cp -f drivers/human/human.xml $1/drivers/human/human.xml
    chmod 640 $1/drivers/human/human.xml
fi
if [ ! -e  $1/drivers/human/preferences.xml ] || [ drivers/human/preferences.xml -nt $1/drivers/human/preferences.xml ]
then
    if [ -e $1/drivers/human/preferences.xml ]
    then
        echo "Saving $1/drivers/human/preferences.xml to $1/drivers/human/preferences.xml.old"
        cp -f $1/drivers/human/preferences.xml $1/drivers/human/preferences.xml.old
    fi
    cp -f drivers/human/preferences.xml $1/drivers/human/preferences.xml
    chmod 640 $1/drivers/human/preferences.xml
fi
mkdir -p $1/config 2>/dev/null
if [ ! -e  $1/config/raceengine.xml ] || [ config/raceengine.xml -nt $1/config/raceengine.xml ]
then
    if [ -e $1/config/raceengine.xml ]
    then
        echo "Saving $1/config/raceengine.xml to $1/config/raceengine.xml.old"
        cp -f $1/config/raceengine.xml $1/config/raceengine.xml.old
    fi
    cp -f config/raceengine.xml $1/config/raceengine.xml
    chmod 640 $1/config/raceengine.xml
fi
if [ ! -e  $1/config/style.xsl ] || [ config/style.xsl -nt $1/config/style.xsl ]
then
    if [ -e $1/config/style.xsl ]
    then
        echo "Saving $1/config/style.xsl to $1/config/style.xsl.old"
        cp -f $1/config/style.xsl $1/config/style.xsl.old
    fi
    cp -f config/style.xsl $1/config/style.xsl
    chmod 640 $1/config/style.xsl
fi
mkdir -p $1/config 2>/dev/null
if [ ! -e  $1/config/screen.xml ] || [ config/screen.xml -nt $1/config/screen.xml ]
then
    if [ -e $1/config/screen.xml ]
    then
        echo "Saving $1/config/screen.xml to $1/config/screen.xml.old"
        cp -f $1/config/screen.xml $1/config/screen.xml.old
    fi
    cp -f config/screen.xml $1/config/screen.xml
    chmod 640 $1/config/screen.xml
fi
mkdir -p $1/config 2>/dev/null
if [ ! -e  $1/config/graph.xml ] || [ config/graph.xml -nt $1/config/graph.xml ]
then
    if [ -e $1/config/graph.xml ]
    then
        echo "Saving $1/config/graph.xml to $1/config/graph.xml.old"
        cp -f $1/config/graph.xml $1/config/graph.xml.old
    fi
    cp -f config/graph.xml $1/config/graph.xml
    chmod 640 $1/config/graph.xml
fi
if [ ! -e  $1/config/sound.xml ] || [ config/sound.xml -nt $1/config/sound.xml ]
then
    if [ -e $1/config/sound.xml ]
    then
        echo "Saving $1/config/sound.xml to $1/config/sound.xml.old"
        cp -f $1/config/sound.xml $1/config/sound.xml.old
    fi
    cp -f config/sound.xml $1/config/sound.xml
    chmod 640 $1/config/sound.xml
fi
mkdir -p $1/results/followrace 2>/dev/null
mkdir -p $1/results/lightimagerace 2>/dev/null
mkdir -p $1/results/limitimagerace 2>/dev/null
mkdir -p $1/results/limitsensorrace 2>/dev/null
mkdir -p $1/results/passbasicrace 2>/dev/null
mkdir -p $1/results/passhardrace 2>/dev/null
mkdir -p $1/results/quickrace 2>/dev/null
mkdir -p $1/config/raceman 2>/dev/null
if [ ! -e  $1/config/raceman/followrace.xml ] || [ config/raceman/followrace.xml -nt $1/config/raceman/followrace.xml ]
then
    if [ -e $1/config/raceman/followrace.xml ]
    then
        echo "Saving $1/config/raceman/followrace.xml to $1/config/raceman/followrace.xml.old"
        cp -f $1/config/raceman/followrace.xml $1/config/raceman/followrace.xml.old
    fi
    cp -f config/raceman/followrace.xml $1/config/raceman/followrace.xml
    chmod 640 $1/config/raceman/followrace.xml
fi
if [ ! -e  $1/config/raceman/lightimagerace.xml ] || [ config/raceman/lightimagerace.xml -nt $1/config/raceman/lightimagerace.xml ]
then
    if [ -e $1/config/raceman/lightimagerace.xml ]
    then
        echo "Saving $1/config/raceman/lightimagerace.xml to $1/config/raceman/lightimagerace.xml.old"
        cp -f $1/config/raceman/lightimagerace.xml $1/config/raceman/lightimagerace.xml.old
    fi
    cp -f config/raceman/lightimagerace.xml $1/config/raceman/lightimagerace.xml
    chmod 640 $1/config/raceman/lightimagerace.xml
fi
if [ ! -e  $1/config/raceman/limitimagerace.xml ] || [ config/raceman/limitimagerace.xml -nt $1/config/raceman/limitimagerace.xml ]
then
    if [ -e $1/config/raceman/limitimagerace.xml ]
    then
        echo "Saving $1/config/raceman/limitimagerace.xml to $1/config/raceman/limitimagerace.xml.old"
        cp -f $1/config/raceman/limitimagerace.xml $1/config/raceman/limitimagerace.xml.old
    fi
    cp -f config/raceman/limitimagerace.xml $1/config/raceman/limitimagerace.xml
    chmod 640 $1/config/raceman/limitimagerace.xml
fi
if [ ! -e  $1/config/raceman/limitsensorrace.xml ] || [ config/raceman/limitsensorrace.xml -nt $1/config/raceman/limitsensorrace.xml ]
then
    if [ -e $1/config/raceman/limitsensorrace.xml ]
    then
        echo "Saving $1/config/raceman/limitsensorrace.xml to $1/config/raceman/limitsensorrace.xml.old"
        cp -f $1/config/raceman/limitsensorrace.xml $1/config/raceman/limitsensorrace.xml.old
    fi
    cp -f config/raceman/limitsensorrace.xml $1/config/raceman/limitsensorrace.xml
    chmod 640 $1/config/raceman/limitsensorrace.xml
fi
if [ ! -e  $1/config/raceman/passbasicrace.xml ] || [ config/raceman/passbasicrace.xml -nt $1/config/raceman/passbasicrace.xml ]
then
    if [ -e $1/config/raceman/passbasicrace.xml ]
    then
        echo "Saving $1/config/raceman/passbasicrace.xml to $1/config/raceman/passbasicrace.xml.old"
        cp -f $1/config/raceman/passbasicrace.xml $1/config/raceman/passbasicrace.xml.old
    fi
    cp -f config/raceman/passbasicrace.xml $1/config/raceman/passbasicrace.xml
    chmod 640 $1/config/raceman/passbasicrace.xml
fi
if [ ! -e  $1/config/raceman/passhardrace.xml ] || [ config/raceman/passhardrace.xml -nt $1/config/raceman/passhardrace.xml ]
then
    if [ -e $1/config/raceman/passhardrace.xml ]
    then
        echo "Saving $1/config/raceman/passhardrace.xml to $1/config/raceman/passhardrace.xml.old"
        cp -f $1/config/raceman/passhardrace.xml $1/config/raceman/passhardrace.xml.old
    fi
    cp -f config/raceman/passhardrace.xml $1/config/raceman/passhardrace.xml
    chmod 640 $1/config/raceman/passhardrace.xml
fi
if [ ! -e  $1/config/raceman/quickrace.xml ] || [ config/raceman/quickrace.xml -nt $1/config/raceman/quickrace.xml ]
then
    if [ -e $1/config/raceman/quickrace.xml ]
    then
        echo "Saving $1/config/raceman/quickrace.xml to $1/config/raceman/quickrace.xml.old"
        cp -f $1/config/raceman/quickrace.xml $1/config/raceman/quickrace.xml.old
    fi
    cp -f config/raceman/quickrace.xml $1/config/raceman/quickrace.xml
    chmod 640 $1/config/raceman/quickrace.xml
fi
