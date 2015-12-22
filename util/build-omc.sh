# This shell script will download and built the OpenModelica compiler
# and object files that are required to use Modelica models with
# adevs. If you run this script in the director D then this script
# will place the modelica compiler into D/trunk/build/bin and
# you should point MODELICA_HOME in the adevs Makefile to D/trunk. 
HOMEDIR=$PWD/openmodelica
mkdir openmodelica
cd $HOMEDIR
# Build smlnj and put it in our path
mkdir smlnj
cd smlnj
wget http://smlnj.cs.uchicago.edu/dist/working/110.76/config.tgz
tar xfvz config.tgz
config/install.sh
export SMLNJ_HOME=$PWD
export PATH=$PWD/bin:$PATH
cd $HOMEDIR
# Build the RML compiler and put it in our path
mkdir rml
svn co --username=anonymous --password=none https://openmodelica.org/svn/MetaModelica/trunk mmc
export RMLHOME=$PWD/rml
cd mmc
./configure --prefix=$RMLHOME
make
make install
cd $HOMEDIR
export PATH=$RMLHOME/bin:$PATH
# Build the openmodelica compiler
svn co https://openmodelica.org/svn/OpenModelica/trunk/
cd trunk
# Check for the configure file. If its not there, then create it.
if [ ! -x configure ];
then
	autoconf
fi
# Configure and build omc
./configure --without-paradiseo --enable-modelica3d=no --enable-omnotebook=no  
make omc
# Done
