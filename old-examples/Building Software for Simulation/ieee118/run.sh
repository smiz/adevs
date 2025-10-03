for network_rate in 100 1000 
do
	for freq_levels in 2 20 
	do
		for gain in 2 12
		do
			./a.out $network_rate $gain $freq_levels 21 
		done
	done
done

