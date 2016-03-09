# !/bin/sh
#
# Script to create a bash test
#

function GenerateConFiles ()
{
grep -onE  '\{([0-9]*\.?[0-9]+ ?)+\}' ../$1/$1.template > ranges
wc -l ranges | grep -oE '[0-9]+' > num_ranges
grep -oE '[0-9]+:' ranges | grep -oE '[0-9]+' > range_lines
read n < num_ranges
declare num_params_vector[$n]
declare range_lines_vector[$n]
num_tests=1
for ((i=1;i<=$n;i++))
do
sed -n "$i p" ranges | grep -oE '\{.*\}' | grep -oE '[0-9]*\.?[0-9]+' > range_$i
wc -l range_$i | grep -oE '[0-9]+ ' > num_params
read num_params_vector[$i] < num_params
sed -n "$i p" range_lines > range_line
read range_lines_vector[$i] < range_line
num_tests=`expr $num_tests \* ${num_params_vector[$i]}`
done

param_period=$num_tests
for ((i=1;i<=$n;i++))
do
param_period=`expr $param_period / ${num_params_vector[$i]}`
for ((j=0;j<$num_tests;j++))
do
param_index=`expr $j / $param_period`
param_index=`expr $param_index % ${num_params_vector[$i]}`

sed -n "`expr $param_index + 1` p" range_$i >> $1_test_`expr $j + 1`
done
done

mkdir ./cons
for arq in `ls $1_test_*`
do
cp ../$1/$1.template ./cons/$arq
for ((i=1;i<=$n;i++))
do
sed -i "${range_lines_vector[$i]} s/{.*}/`sed -n "$i p" $arq`/" ./cons/$arq
done
done
return
}

function ShowUsage ()
{	
	echo "Usage: ./maebash.sh -option <example>"
	echo "Where options include:"
	echo " -create      Create a bash test"
	echo " -compile     Compile a bash test"
	echo " -clean       Clean a bash test"
	echo " -submit      Submit a bash test"
	echo " -resubmit    Resubmit a bash test"
	echo " -summary     Summary a bash test results"
	echo " -destroy     Destroy a bash test"
	echo " -testfail    Test fail jobs"

	return
}

if [ $# -ne 2 ]
then
	ShowUsage
	exit 1
fi

case $1 in
	"-create")
		echo "Creating $2 bash"
		mkdir ./.temp
		cd ./.temp
		GenerateConFiles $2
		cd ..

		for arq in `ls .temp/cons`
		do
			echo "-> Creating directory $arq"
			cp -r $2 $arq
			echo "-> Removing CVS files"
			rm -rf $arq/CVS
			rm -rf $arq/$2_user_functions/CVS
			echo "-> Copying .con file"
			cp .temp/cons/$arq $arq/$2.con	
		done
		;;
	"-compile")
		echo "Compiling $2 bash"
		for i in `ls -d $2_test_*`
		do
			echo "-> Compiling application $i/$2" 
			make -C $i
		done
		;;
	"-clean")
		echo "Cleaning $2 bash"
		for i in `ls -d $2_test_*`
		do
			echo "-> Cleaning directory $i" 
			make -C $i clean
		done
		;;
	"-submit")
		echo "Submitting $2 bash"
		for i in `ls -d $2_test_*`
		do
			echo "-> Submitting job $i/$2.sh" 
			cd $i/
			qsub $2.sh >> $2.test
			cd ..
		done
		;;
	"-resubmit")
		echo "Resubmitting $2 bash"
		for i in `ls -d $2_test_*`
		do
			cd $i/
			read results < $2.test
			if [ -n "$results" ]
			then
				echo "-> Submitting job $i/$2.sh"
				qsub $2.sh >> $2.test
			fi
			cd ..
		done
		;;
	"-testfail")
		echo "Testing failing $2 bash"
		for i in `ls -d $2_test_*`
		do
			cd $i/
			read results < $2.test
			if [ -n "$results" ]
			then
				echo "-> Jog $i/$2.sh failed"
			fi
			cd ..
		done
		;;
	"-summary")
		echo "Summaring $2 bash"
		for FILE in `ls -d $2_test_*`
		do
			echo "-> Catching $FILE results"
			tail -n 1 $FILE/$2.test >> .temp/$FILE
			LINE=`cat .temp/$FILE`
			echo -e $LINE >> .temp/$2.test
		done
		sort .temp/$2.test > $2.test
		;;
	"-destroy")   
		echo "Destroing $2 bash"
		for i in `ls -d $2_test_*`
		do
			echo "-> Removing directory $i" 
			rm -rf $i
		done
		rm -rf .temp
		;;
	*)
		echo "Sorry, wrong option"
		ShowUsage
		;;
esac	
