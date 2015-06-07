now=$(date +"%T")
echo "Current time : $now"
python test_captain_down.py 10 10 100
now=$(date +"%T")
echo "Current time : $now"
echo "test_captain_random_down 10 10 100 DONE"

now=$(date +"%T")
echo "Current time : $now"
python test_captain_downup.py 10 10 100
now=$(date +"%T")
echo "Current time : $now"
echo "test_captain_random_downup 10 10 100 DONE"
