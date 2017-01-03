for i in `seq 5 2 13`
do
	echo $i
	./bin/EdgePreservedBlur ./data/input/glassred.jpg $i
	matlab -nodisplay -nodesktop -r "LeastSquare"
done
