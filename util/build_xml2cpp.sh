# Clean up stale files
rm -f a.out
rm -rf binaries
# Cleanup the junk produced by the omc compiler - FourZone_GymArea should be replaced with the fmu file name. 
rm -f FourZone_GymArea_*
rm -f FourZone_GymArea.c
# Unpack the fmu data 
unzip -o -qq FourZone_GymArea.fmu
python xml2cpp.py -r modelDescription.xml -type double -f $PWD/binaries/linux64/FourZone_GymArea.so -o FourZoneGymArea


