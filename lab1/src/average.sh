if [ $# -eq 0 ]; then
	exit 1
fi

sum=0
for n in "$@"; do
	sum=$((sum+n))
done

count=$#
echo "count=$#"
echo "sum=$sum"
echo "average= $(echo "scale=2; $sum / $#" | bc)"
