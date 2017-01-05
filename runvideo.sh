max=721
for i in `seq 1 $max`
do
	echo $i
	var=$(printf './data/input/social/social%04d.jpg 5' "$i")
	./bin/EdgePreservedBlur $var
	matlab -nodisplay -nodesktop -r "xSolver"
done
